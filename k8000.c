/***************************************************************************
                        mod_k8000.c  -  description
                             -------------------
    begin                : Mon Jan 07 2002
    email                : joris@struyve.be
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* CVS information :
 *
 * $Id: k8000.c,v 1.5 2005/07/30 10:53:23 js Exp $
 *
 */

#ifdef __KERNEL__
  #include <linux/kernel.h>
  #include <linux/module.h>
  MODULE_AUTHOR("Joris Struyve <joris@struyve.be>");
  MODULE_DESCRIPTION("libk8000 kernel module");
  MODULE_LICENSE("GPL");

  #ifdef __MPC860__
    #include <asm/commproc.h>
    #define SCL 0x00000020
    #define SDA 0x00000010
  #else
    #include <asm/io.h>
  #endif
#else
  #include <time.h>
  #include <stdarg.h>
  #include <stdio.h>
  #include <string.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <sys/ipc.h>
  #include <sys/sem.h>
  #include <sys/io.h>
  #include <errno.h>

  /* Includes for I2C Kernel Device */
  #include <unistd.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include <linux/i2c.h>

  #define MUTEX_LPT0 4567
  #define MUTEX_LPT1 4568
  #define MUTEX_LPT2 4569
  #define MUTEX_LPT3 4570
#endif

#define HIGH 1
#define LOW 0
#include "k8000.h"

#ifndef __KERNEL__
  int deviceDescriptor;
  char DeviceName[100]="/dev/velleman";

  /* mutex stuff */
  int mut_id[4];
  int *current_mut_id;
  int use_mutexlock=0;
#endif



/*********************************************\
*  Pin assignment for  5 chains               *
***********************************************
* Chain 0 (default chain)                     *
* pin 17 1-Select in SCL D3 (controlport)     *
* pin 13 Select SDA in D4 (statusport)        *
* pin 14 1-Auto Feed SDA out D1 (controlport) *
***********************************************
* Chain 1                                     *
* pin 2 Data0 SCL D0 (dataport)               *
* pin 10 1-ACK SDA in D6 (statusport)         *
* pin 1 1-Strobe SDA out D0 (controlport)     *
***********************************************
* Chain 2                                     *
* pin 4 Data2 SCL D2 (dataport)               *
* pin 11 Busy SDA in D7 (statusport)          *
* pin 3 Data1 SDA out D1 (dataport)           *
***********************************************
* Chain 3                                     *
* pin 6 Data4 SCL D4 (dataport)               *
* pin 12 Paper Empty SDA in D5 (statusport)   *
* pin 5 Data3 SDA out D3 (dataport)           *
***********************************************
* Chain 4                                     *
* pin 8 Data6 SCL D6 (dataport)               *
* pin 15 1-Error SDA in D3 (statusport)       *
* pin 7 Data5 SDA out D5 (dataport)           *
\*********************************************/
typedef struct
{
	short *scl_port_addr;
	short *sda_in_port_addr;
	short *sda_out_port_addr;

	byte scl_mask;
	byte sda_in_mask;
	byte sda_out_mask;

	byte scl_reverse;
	byte sda_in_reverse;
	byte sda_out_reverse;
} chain_config;

short ControlPort;
short StatusPort;
short DataPort;

/* declare and init the chains array */
chain_config chains[5] = { {&ControlPort        ,&StatusPort         ,&ControlPort,
                            0x08 /* 0000.1000 */,0x10 /* 0001.0000 */,0x02 /* 0000.0010 */,1,0,1},
                            
                           {&DataPort           ,&StatusPort         ,&ControlPort,
                            0x01 /* 0000.0001 */,0x40 /* 0100.0000 */,0x01 /* 0000.0001 */,0,1,1},
                            
                           {&DataPort           ,&StatusPort         , &DataPort,
                            0x04 /* 0000.0100 */,0x80 /* 1000.0000 */,0x02 /* 0000.0010 */,0,0,0},
                            
                           {&DataPort           ,&StatusPort         ,&DataPort,
                            0x10 /* 0001.0000 */,0x20 /* 0010.0000 */,0x08 /* 0000.1000 */,0,0,0},
                            
                           {&DataPort           ,&StatusPort         ,&DataPort,
                            0x40 /* 0100.0000 */,0x08 /* 0000.1000 */,0x20 /* 0010.0000 */,0,1,0}};

/* active_chain points to a chain_config struct in the chains array */
chain_config *active_chain = chains;

/* IC addresses ( 7 LSB's are meaningfull ) */
byte IOchipcode[]   = {0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
byte DACchipcode[]  = {0x20, 0x21, 0x22, 0x23};
byte ADDAchipcode[] = {0x48, 0x49, 0x4A, 0x4B};

/* configuration and data matrix*/
short IOconfig_matrix[MaxChains][MaxIOchip+1];
short IOdata_matrix  [MaxChains][MaxIOchip+1];
short IO_matrix      [MaxChains][MaxIOchannel+1];
short DAC_matrix     [MaxChains][MaxDACchannel+1];
short AD_matrix      [MaxChains][MaxADchannel+1];
short DA_matrix      [MaxChains][MaxDAchannel+1];

/* old configuration and data arrays  */
/* they just point to the first chain */
short *IOconfig = IOconfig_matrix[0];
short *IOdata   = IOdata_matrix[0];
short *IO       = IO_matrix[0];
short *DAC      = DAC_matrix[0];
short *AD       = AD_matrix[0];
short *DA       = DA_matrix[0];

/* Global vars */
int debug=0;
int printerror=0;
int I2CbusDelay = 70;
int pre_I2CbusDelay = 35;
int post_I2CbusDelay = 35;
int autoioperm=1;
int activechainnumber=0;

#ifdef __KERNEL__
	MODULE_PARM(debug,"i");
#endif

/* Function pointer declarations */
void (*I2cSendData)(byte addr, byte *data, int len)=NULL;
void (*I2cReadData)(byte addr,byte *data,int len)=NULL;

/**************************************************************\
* Prints message  if debug is on                               *
\**************************************************************/
void Debug(char *fmt,...)
{
	va_list ap;
	char buf[200];

	if(debug)
	{
		va_start(ap,fmt);
		vsprintf(buf,fmt,ap);
		va_end(ap);
		buf[200]=0;
#ifdef __KERNEL__
		printk(buf);
#else
		printf(buf);fflush(stdout);
#endif
	}
}
/**************************************************************\
* Prints error message to stderr or kernel ring buffer         *
\**************************************************************/
void PrintError(char *fmt,...)
{
	va_list ap;
	char buf[200];

	if(printerror)
	{
		va_start(ap,fmt);
		vsprintf(buf,fmt,ap);
		va_end(ap);
		buf[200]=0;
#ifdef __KERNEL__
		printk(buf);
#else
		fprintf(stderr,"LibK8000 : ");
		fprintf(stderr,buf);fflush(stderr);
#endif
	}
}

/**************************************************************\
*  Delay functions.                                            *
\**************************************************************/
inline void I2cDelay(void)
{
	int i,j;
	for (i=0;i<I2CbusDelay;i++) {j++;};
}

inline void I2cPreDelay(void)
{
	int i,j;
	for (i=0;i<pre_I2CbusDelay;i++) {j++;};
}

inline void I2cPostDelay(void)
{
	int i,j;
	for (i=0;i<post_I2CbusDelay;i++) {j++;};
}

/**************************************************************\
*  IC basic functions (platform/interface specific)           *
*                                                              *
*  This is the platform/interface specific part.               *
*  This is for user space progs over the parallel port.        *
*                                                              *
\**************************************************************/
#ifdef __MPC860__
inline byte GetScl(void)
{
	byte tmp;
	volatile cpm8xx_t *cp = cpmp;

	tmp = (byte) cp->cp_pbdat & SCL;
	return tmp;
}

inline void SetScl(short state)
{
	volatile cpm8xx_t *cp = cpmp;
	int i=0;

	I2cPreDelay();
	if(state==HIGH)
	{
		cp->cp_pbdat |= SCL;
		cp->cp_pbdir &= 0xFFFFFFDF;
		// wait some time for scl to be released by slave
		for(i=0;i<I2CbusDelay && !GetScl();i++);
		cp->cp_pbdir |= 0x00000020;
	}
	else
		cp->cp_pbdat &= ~SCL;
	I2cPostDelay();
}

inline void SetSda(short state)
{
	volatile cpm8xx_t *cp = cpmp;

	if(state==HIGH)
		cp->cp_pbdat |= SDA;
	else
		cp->cp_pbdat &= ~SDA;
}

inline byte GetSda(void)
{
	byte tmp;
	volatile cpm8xx_t *cp = cpmp;

	tmp = (byte) cp->cp_pbdat & SDA;

	return tmp;
}

inline void SdaOut(void)
{
	volatile cpm8xx_t *cp = cpmp;

	cp->cp_pbdir |= 0x00000010;
}

inline void SdaIn(void)
{
        volatile cpm8xx_t *cp = cpmp;

        cp->cp_pbdir &= 0xFFFFFFEF;
}
#else // __MPC860__

inline byte GetScl(void)
{
	// SCL is uni-directional for a k8000 connected on the i386 // port
	return 1;
}

inline void SetScl(short state)
{
	I2cPreDelay();
	if ( (active_chain->scl_reverse && state) || (!active_chain->scl_reverse && !state) )
		outb(inb(*(active_chain->scl_port_addr)) & (~active_chain->scl_mask),*(active_chain->scl_port_addr) );
	else
		outb(inb(*(active_chain->scl_port_addr)) | active_chain->scl_mask,*(active_chain->scl_port_addr) );
	I2cPostDelay();
}

inline void SetSda(short state)
{
	if ( (active_chain->sda_out_reverse && state) || (!active_chain->sda_out_reverse && !state) )
		outb(inb(*(active_chain->sda_out_port_addr)) & (~active_chain->sda_out_mask) ,*(active_chain->sda_out_port_addr));
	else
		outb(inb(*(active_chain->sda_out_port_addr)) | active_chain->sda_out_mask,*(active_chain->sda_out_port_addr));
}

inline byte GetSda(void)
{
	if(active_chain->sda_in_reverse)
		return (byte) !((inb(*(active_chain->sda_in_port_addr))) & active_chain->sda_in_mask);
	else
		return (byte) ((inb(*(active_chain->sda_in_port_addr))) & active_chain->sda_in_mask);
}

// No need to set the sda direction on a i386 // port
inline void SdaOut(void) {}
inline void SdaIn(void) {} 
#endif // __MPC860__


/**************************************************************\
*  IC standard protocol conditions                             *
\**************************************************************/
inline void StartCond(void)
{
	/* Assert : SCL and SDA are high */
	SetSda(LOW);
	SetScl(LOW);
}

inline void I2CBusNotBusy(void)
{
	SetScl(HIGH);
	SetSda(HIGH);
}

inline void StopCond(void)
{
	SetSda(LOW);
	SetScl(HIGH);
	SetSda(HIGH);
}

inline byte GetAck(void)
{
	int i;
	byte val;

	SetSda(HIGH);
	SdaIn();
	SetScl(HIGH);
	// wait some time for sda to be pulled low by slave
	val = GetSda();
	for(i=0;i<I2CbusDelay && val;i++)
		val = GetSda();
	SetScl(LOW);
	SdaOut();
	return val;
}

inline void MasterAck(void)
{
	SetSda(LOW);
	SetScl(HIGH);
	SetScl(LOW);
	SetSda(HIGH);
}

inline void MasterNack(void)
{
	SetSda(HIGH);
	SetScl(HIGH);
	SetScl(LOW);
}

/**************************************************************\
*  Generic IC input/output functions                          *
\**************************************************************/
byte I2cSendAddressByte(byte addr)
{
	int i;
	int sb;

	/* clock out the slave the address byte */
	Debug("I2cSendAddressByte ADDR : ");
	for (i=7;i>=0;i--)
	{
		sb = addr & ( 1 << i );
		Debug("%d",sb>0?1:0);
		if(sb)
			SetSda(HIGH);
		else
			SetSda(LOW);
		SetScl(HIGH);
		SetScl(LOW);
	}

	// return the slave ack state
	return GetAck();
}

void I2cSendData_direct(byte addr,byte *data,int len)
{
	int i,j;
	byte sb;

	/* set the write lsb (0) */
	addr = (addr<<1);
	StartCond();
	if( I2cSendAddressByte(addr) )
		PrintError("(W) NACK addr 0x%X\n",addr>>1);

	/* send the data */
	Debug(" (DATA ");
	for (j=0;j<len;j++) /* for all data bytes */
	{
		Debug(":");
		for (i=7;i>=0;i--)
		{
			sb = (*(data +j)) & ( 1 << i );
			Debug("%d",sb>0?1:0);
			if(sb)
				SetSda(HIGH);
			else
				SetSda(LOW);
			SetScl(HIGH);
			SetScl(LOW);
		}
		// read byte ack from slave
		if ( GetAck() )
			PrintError("(W) NACK byte %d for 0x%X\n",j+1,addr>>1);
	}
	StopCond();
	Debug(")\n");
}

void I2cReadData_direct(byte addr,byte *data,int len)
{
	int i,j;
	byte sb;

	/* set the read lsb (1) */
	addr = (addr<<1)|1;
	StartCond();
	if( I2cSendAddressByte(addr) )
		PrintError("(R) NACK addr 0x%X\n",addr>>1);

	/* read the data */
	Debug(" (DATA ");
	for (j=0;j<len;j++) /* for all data bytes */
	{
		Debug(":");
		*(data+j) = 0;
		SdaIn();
		for (i=7;i>=0;i--)
		{
			SetScl(HIGH);
			sb = GetSda();
			SetScl(LOW);

			Debug("%d",sb>0?1:0);
			*(data+j) |= ((sb>0?1:0)<<i);
		}
		SdaOut();
		// ack byte, except last byte
		if(j!=len-1)
			MasterAck();
		else
			MasterNack();
	}
	Debug(")\n");
	StopCond();
}

#ifndef __KERNEL__
void I2cSendData_device(byte addr,byte *data,int len)
{
	int result=0;
	if(ioctl(deviceDescriptor,I2C_SLAVE, addr))
		PrintError("LIBK8000 I2cSendData_device : IOCTL Problem",strerror(errno));
	
	result=write(deviceDescriptor,data,len);
}


void I2cReadData_device(byte addr,byte *data,int len)
{
	int result=0;
	if(ioctl(deviceDescriptor,I2C_SLAVE, addr))
		PrintError("LIBK8000 I2cReadData_device : IOCTL Problem %s",strerror(errno));

	result=read(deviceDescriptor,data,len);
}
#endif

/**************************************************************\
*  IO input procedures                                         *
\**************************************************************/
void ReadIOchip(int chip_no)
{
	short start_channel, channel;
	byte data;

	I2cReadData(IOchipcode[chip_no],&data,1);
	IOdata_matrix[activechainnumber][chip_no] = ~data;

	start_channel = chip_no * 8 + 1;
	for (channel = 0; channel <= 7; channel++)
		IO_matrix[activechainnumber][start_channel+channel] = ((IOdata_matrix[activechainnumber][chip_no] & (0x01 << channel)) != 0);
}

void ReadAllIO(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOchip; chip_no++)
		ReadIOchip(chip_no);
}

void ReadIOchannel(int channel_no)
{
	int chip_no;

	chip_no = (channel_no - 1) / 8;
	ReadIOchip(chip_no);
}

/**************************************************************\
*  IO output procedures                                        *
\**************************************************************/
void IOoutput(int chip_no, short data)
{
	byte start_channel, channel;
	byte conv;

	conv = (byte) ((~data) | IOconfig_matrix[activechainnumber][chip_no]);
	I2cSendData((byte)IOchipcode[chip_no],&conv,1);
	IOdata_matrix[activechainnumber][chip_no] = (IOdata_matrix[activechainnumber][chip_no] & IOconfig_matrix[activechainnumber][chip_no]) | (~conv);

	start_channel = chip_no * 8 + 1;
	for (channel = 0; channel <= 7; channel++)
		IO_matrix[activechainnumber][start_channel+channel] = ((IOdata_matrix[activechainnumber][chip_no] & (0x01 << channel)) != 0);
}

void ClearIOchip(int chip_no)
{
	IOoutput(chip_no, 0);
}

void ClearIOchannel(int channel_no)
{
	int chip_no, channel;
	byte data;

	chip_no = (channel_no - 1) / 8;
	channel = (channel_no - 1) % 8;
	data = IOdata_matrix[activechainnumber][chip_no] & (~(0x01 << channel));
	IOoutput(chip_no, data);
}

void SetIOchip(int chip_no)
{
	IOoutput(chip_no, 0xFF);
}

void SetIOchannel(int channel_no)
{
	int chip_no, channel;
	byte data;

	chip_no = (channel_no - 1) / 8;
	channel = (channel_no - 1) % 8;
	data = IOdata_matrix[activechainnumber][chip_no] | (0x01 << channel);
	IOoutput(chip_no, data);
}

void SetAllIO(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOchip; chip_no++)
		IOoutput(chip_no, 0xFF);
}

void ClearAllIO(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOchip; chip_no++)
		IOoutput(chip_no, 0);
}

/**************************************************************\
*  IO data update procedures                                   *
\**************************************************************/
void UpdateIOdataArray(int chip_no, short data)
{
	int start_channel, channel;

	IOdata_matrix[activechainnumber][chip_no] = (IOdata_matrix[activechainnumber][chip_no] & IOconfig_matrix[activechainnumber][chip_no]);
	IOdata_matrix[activechainnumber][chip_no] = IOdata_matrix[activechainnumber][chip_no] | (data & (~IOconfig_matrix[activechainnumber][chip_no]));

	start_channel = chip_no * 8 + 1;
	for (channel = 0; channel <= 7; channel++)
		IO_matrix[activechainnumber][start_channel+channel] = ((IOdata_matrix[activechainnumber][chip_no] & (0x01 << channel)) != 0);
}

void ClearIOdataArray(int chip_no)
{
	int start_channel, channel;

	IOdata_matrix[activechainnumber][chip_no] = IOdata_matrix[activechainnumber][chip_no] & IOconfig_matrix[activechainnumber][chip_no];
	start_channel = chip_no * 8 + 1;
	for (channel = 0; channel <= 7; channel++)
		IO_matrix[activechainnumber][start_channel+channel] = ((IOdata_matrix[activechainnumber][chip_no] & (0x01 << channel)) != 0);
}

void SetIOdataArray(int chip_no)
{
	int start_channel, channel;

	IOdata_matrix[activechainnumber][chip_no] = IOdata_matrix[activechainnumber][chip_no] | ( ~IOconfig_matrix[activechainnumber][chip_no]);
	start_channel = chip_no * 8 + 1;
	for (channel = 0; channel <= 7; channel++)
		IO_matrix[activechainnumber][start_channel+channel] = ((IOdata_matrix[activechainnumber][chip_no] & (0x01 << channel)) != 0);
}

void SetIOchArray(int channel_no)
{
	int chip_no, channel;
	byte data;

	chip_no = (channel_no - 1) / 8;
	channel = (channel_no - 1) % 8;
	data = IOdata_matrix[activechainnumber][chip_no] | (0x01 << channel);
	UpdateIOdataArray(chip_no, data);
}

void ClearIOchArray(int channel_no)
{
	int chip_no, channel;
	byte data;

	chip_no = (channel_no - 1) / 8;
	channel = (channel_no - 1) % 8;
	data = IOdata_matrix[activechainnumber][chip_no] & (~(0x01 << channel));
	UpdateIOdataArray(chip_no, data);
}

void UpdateAllIO(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOchip; chip_no++)
	IOoutput(chip_no, IOdata_matrix[activechainnumber][chip_no]);
}

void UpdateIOchip(int chip_no)
{
	IOoutput(chip_no, IOdata_matrix[activechainnumber][chip_no]);
}


/**************************************************************\
*  IO config procedures                                        *
\**************************************************************/
void ConfigAllIOasInput(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOchip; chip_no++)
	{
		IOconfig_matrix[activechainnumber][chip_no] = 0;
		ClearIOchip(chip_no);
		IOconfig_matrix[activechainnumber][chip_no] = 0xFF;
		ReadIOchip(chip_no);
	}
}

void ConfigAllIOasOutput(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOchip; chip_no++)
	IOconfig_matrix[activechainnumber][chip_no] = 0x00;
	ClearAllIO();
}

void ConfigIOchipAsInput(int chip_no)
{
	IOconfig_matrix[activechainnumber][chip_no] = 0;
	ClearIOchip(chip_no);
	IOconfig_matrix[activechainnumber][chip_no] = 0xFF;
	ReadIOchip(chip_no);
}

void ConfigIOchipAsOutput(int chip_no)
{
	IOconfig_matrix[activechainnumber][chip_no] = 0x00;
	ClearIOchip(chip_no);
}

void ConfigIOchannelAsInput(int channel_no)
{
	int chip_no, channel;

	chip_no = (channel_no - 1) / 8;
	channel = (channel_no - 1) % 8;
	IOconfig_matrix[activechainnumber][chip_no] = IOconfig_matrix[activechainnumber][chip_no] & (~(0x01 << channel));
	ClearIOchannel(channel_no);
	IOconfig_matrix[activechainnumber][chip_no] = IOconfig_matrix[activechainnumber][chip_no] | (0x01 << channel);
	ReadIOchannel(channel_no);
}

void ConfigIOchannelAsOutput(int channel_no)
{
	int chip_no, channel;

	chip_no = (channel_no - 1) / 8;
	channel = (channel_no - 1) % 8;
	IOconfig_matrix[activechainnumber][chip_no] = IOconfig_matrix[activechainnumber][chip_no] & (~(0x01 << channel));
	ClearIOchannel(channel_no);
}

/**************************************************************\
*  DAC output procedures                                       *
\**************************************************************/
void OutputDACchannel(int channel_no, int data)
{
	byte total_data[2];

	total_data[0] = (byte) (0xF0 | ((channel_no - 1) % 8));
	total_data[1] = (byte) (data > 63) ? 63: data;
	DAC_matrix[activechainnumber][channel_no] = total_data[1];

	I2cSendData(DACchipcode[(channel_no-1)/8],total_data,2);
}

void UpdateDACchip(int chip_no)
{
	byte total_data[9];
	int i,channel;

	total_data[0] = 0x00;
	channel = chip_no * 8;
	for(i=1;i<=8;i++)
		total_data[i] = DAC_matrix[activechainnumber][channel + i] = (DAC_matrix[activechainnumber][channel+i] > 63) ? 63 : DAC_matrix[activechainnumber][channel+i];

	I2cSendData(DACchipcode[chip_no],total_data,9);
}

void UpdateDACchannel(int channel_no)
{
	OutputDACchannel(channel_no, DAC_matrix[activechainnumber][channel_no]);
}

void ClearDACchannel(int channel_no)
{
	OutputDACchannel(channel_no, 0);
}

void SetDACchannel(int channel_no)
{
	OutputDACchannel(channel_no,63);
}

void ClearDACchip(int chip_no)
{
	int i, channel;

	channel = chip_no * 8;
	for (i = 1; i <= 8; i++)
		DAC_matrix[activechainnumber][channel + i] = 0;
	UpdateDACchip(chip_no);
}

void SetDACchip(int chip_no)
{
	int i, channel;

	channel = chip_no * 8;
	for (i = 1; i <= 8; i++)
		DAC_matrix[activechainnumber][channel + i] = 63;
	UpdateDACchip(chip_no);
}

void UpdateAllDAC(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOcard; chip_no++)
		UpdateDACchip(chip_no);
}

void ClearAllDAC(void)
{
	int channel_no;

	for (channel_no = 1; channel_no <= MaxDACchannel; channel_no++)
		DAC_matrix[activechainnumber][channel_no] = 0x00;
	UpdateAllDAC();
}

void SetAllDAC(void)
{
	int channel_no;

	for (channel_no = 1; channel_no <= MaxDACchannel; channel_no++)
		DAC_matrix[activechainnumber][channel_no] = 63;
	UpdateAllDAC();
}

/**************************************************************\
*  DA output procedures                                        *
\**************************************************************/
void OutputDAchannel(int channel_no, int data)
{
	byte total_data[2];

	total_data[0] = (0x40);
	total_data[1] = (byte) data;
	DA_matrix[activechainnumber][channel_no] = (byte) data;

	I2cSendData(ADDAchipcode[channel_no-1],total_data,2);
}

void UpdateDAchannel(int channel_no)
{
	OutputDAchannel(channel_no, DA_matrix[activechainnumber][channel_no]);
}

void ClearDAchannel(int channel_no)
{
	OutputDAchannel(channel_no, 0);
}

void SetDAchannel(int channel_no)
{
	OutputDAchannel(channel_no, 255);
}

void UpdateAllDA(void)
{
	int channel_no;

	for (channel_no = 1; channel_no <= MaxDAchannel; channel_no++)
		OutputDAchannel(channel_no, DA_matrix[activechainnumber][channel_no]);
}

void ClearAllDA(void)
{
	int channel_no;

	for (channel_no = 1; channel_no <= MaxDAchannel; channel_no++)
		OutputDAchannel(channel_no, 0);
}

void SetAllDA(void)
{
	int channel_no;

	for (channel_no = 1; channel_no <= MaxDAchannel; channel_no++ )
		OutputDAchannel(channel_no, 255);
}

/**************************************************************\
*  AD input procedures                                         *
\**************************************************************/
void ReadADchannel(int channel_no)
{
	byte total_data[2];

	total_data[0] = (byte) (0x40 | ((channel_no - 1) % 4));
	I2cSendData(ADDAchipcode[(channel_no-1)/4],total_data,1);

	I2cReadData(ADDAchipcode[(channel_no-1)/4],total_data,2);
	AD_matrix[activechainnumber][channel_no] = (short) total_data[1];
}

void ReadADchip(int chip_no)
{
	byte total_data[5];
	int channel;

	total_data[0] = (0x44);
	I2cSendData(ADDAchipcode[chip_no],total_data,1);
	I2cReadData(ADDAchipcode[chip_no],total_data,5);

	channel = chip_no * 4 + 1;
	AD_matrix[activechainnumber][channel]     = (short) total_data[1];
	AD_matrix[activechainnumber][channel + 1] = (short) total_data[2];
	AD_matrix[activechainnumber][channel + 2] = (short) total_data[3];
	AD_matrix[activechainnumber][channel + 3] = (short) total_data[4];
}

void ReadAllAD(void)
{
	int chip_no;

	for (chip_no = 0; chip_no <= MaxIOcard; chip_no++)
		ReadADchip(chip_no);
}

/**************************************************************\
*  Complete card and general procedures                        *
\**************************************************************/
void ReadAll(void)
{
	ReadAllIO();
	ReadAllAD();
}

void ReadCard(int card_no)
{
	int chip_no;

	chip_no = card_no * 2;
	ReadIOchip(chip_no);
	ReadIOchip(chip_no + 1);
	ReadADchip(card_no);
}

void UpdateAll(void)
{
	UpdateAllIO();
	UpdateAllDAC();
	UpdateAllDA();
}

void UpdateCard(int card_no)
{
	int chip_no;

	chip_no = card_no * 2;
	UpdateIOchip(chip_no);
	UpdateIOchip(chip_no + 1);
	UpdateDACchip(card_no);
	UpdateDAchannel(card_no + 1);
}

int SetPortPerm(unsigned long from, unsigned long num, int turn_on)
{
#ifdef __KERNEL__
	return 0;
#else
	if (ioperm(from,num,turn_on))
	{
		PrintError("Error setting port permissions '%s'\n",strerror(errno));
		return 1;
	}
	else
		return 0;
#endif
}

int SelectI2CprinterPort(int Printer_no)
{
	switch (Printer_no)
	{
#ifndef __KERNEL__
		case I2C_DEV: /*  Use I2C Kernel Device ( -1 = I2C_DEV) */
			I2cSendData=I2cSendData_device;
			I2cReadData=I2cReadData_device;
			deviceDescriptor=open(DeviceName, O_RDWR);
			if (deviceDescriptor == -1)
			{
				PrintError("Error opening device '%s'\n",strerror(errno));
				return -1;
			}
			else
			return 0;
			break;
#endif
		case 0: /* lpt on monochroom display adapter */
			I2cSendData=I2cSendData_direct;
			I2cReadData=I2cReadData_direct;
#ifndef __KERNEL__
			current_mut_id = &mut_id[0];
#endif
			DataPort    = 0x03BC;
			StatusPort  = 0x03BD;
			ControlPort = 0x03BE;
			if (autoioperm)
				return SetPortPerm(0x3BC, 3, 1);
			break;

		case 1:  /* lpt1 on mainboard */
			I2cSendData=I2cSendData_direct;
			I2cReadData=I2cReadData_direct;
#ifndef __KERNEL__
			current_mut_id = &mut_id[1];
#endif
			DataPort    = 0x0378;
			StatusPort  = 0x0379;
			ControlPort = 0x037A;
			if (autoioperm)
				return SetPortPerm(0x378, 3, 1);
			break;

		case 2:  /* lpt2 on mainboard */
			I2cSendData=I2cSendData_direct;
			I2cReadData=I2cReadData_direct;
#ifndef __KERNEL__
			current_mut_id = &mut_id[2];
#endif
			DataPort    = 0x0278;
			StatusPort  = 0x0279;
			ControlPort = 0x027A;
			if (autoioperm)
				return SetPortPerm(0x278, 3, 1);
			break;
		case 3:  /* lpt on pci board, provided by */
		         /* Philippe Martinole <philippe.martinole@teleauto.org> */
			I2cSendData=I2cSendData_direct;
			I2cReadData=I2cReadData_direct;
#ifndef __KERNEL__
			current_mut_id = &mut_id[3];
#endif
			DataPort    = 0xD000;
			StatusPort  = 0xD001;
			ControlPort = 0xD002;
			if (autoioperm)
				return ioperm(0xD000, 3, 1);
			break;
	}
	return 0;
}

void SelectChain(int chain_no)
{
#ifndef __KERNEL__
	if( I2cSendData==I2cSendData_device && chain_no!=0)
	{
			printf("LIBK8000: SelectChain: ERROR: Chains 1, 2, 3 and 4 can not (yet ?) be used with i2c device\n");
			printf("                              Only default chain 0 can be used with i2c device. \n");
			printf("                              Use direct parallel port instead.\n\n");
			exit(0);
	}
#endif
	if (chain_no>4)
		chain_no = 4;
		
	active_chain = chains+(chain_no);
	activechainnumber = chain_no;
}

void SetI2cBusdelay(int d)
{
	I2CbusDelay = d;
	post_I2CbusDelay = I2CbusDelay /2;
	pre_I2CbusDelay  = I2CbusDelay /2;
}

void initialize(void)
{
	SelectI2CprinterPort(1);
	SetI2cBusdelay(1000);
	I2CBusNotBusy();
}

void SetDebug(int d)
{
	debug = d;
}

void SetPrintError(int d)
{
        printerror = d;
}


void SetAutoPerm(int set)
{
	autoioperm = set;
}

/**************************************************************\
*  Mutex functions                                             *
\**************************************************************/
#ifndef __KERNEL__
void SetMutexlock(int set)
{
	use_mutexlock = set;
	if(set)
	{
		mut_id[0] = CreateMutex(MUTEX_LPT0);
		mut_id[1] = CreateMutex(MUTEX_LPT1);
		mut_id[2] = CreateMutex(MUTEX_LPT2);
		mut_id[3] = CreateMutex(MUTEX_LPT3);
	}
}

int CreateMutex(key_t key)
{
	int sem;
	int retval = 0;

	union semun
	{
		int val;
		struct semid_ds *buf;
		ushort *array;
	} argument;

	argument.val = 1;
	if ((sem = semget(key,1,0666|IPC_CREAT|IPC_EXCL)) == -1)
	{
		if (errno==EEXIST)
		sem = semget(key, 1, 0);
	}
	else
		retval = semctl(sem,0,SETVAL,argument);

	if (sem == -1 || retval == -1)
		return -1;
	else
		return sem;
}

int LockMutex(int sem)
{
	struct sembuf buf;

	buf.sem_num = 0;
	buf.sem_op  = -1;
	buf.sem_flg = SEM_UNDO;

	if (semop(sem, &buf, 1) == -1)
		return 1;
	else
		return 0;
}

int UnlockMutex(int sem)
{
	struct sembuf buf;

	buf.sem_num = 0;
	buf.sem_op  = 1;
	buf.sem_flg = SEM_UNDO;

	if (semop(sem, &buf, 1) == -1)
		return 1;
	else
		return 0;
}

void DisposeMutex(int sem)
{
	semctl(sem,0,IPC_RMID,0);
}
#endif

/**************************************************************\
*  Standard kernel module functions                            *
\**************************************************************/
#ifdef __KERNEL__
int  init_module(void)
{
#ifdef __MPC860__
	volatile cpm8xx_t *cp = cpmp;

	/* enable general purpose IO on PB26&27 */
	cp->cp_pbpar &= 0xFFFFFFCF;

	// Both pins outputs
	cp->cp_pbdir |= 0x00000030;

#endif
	printk("mod_k8000: Libk8000 kernel functions library loaded\n");
	printk("mod_k8000: by Joris Struyve <joris@struyve.be>\n");
	
	I2cSendData=I2cSendData_direct;
	I2cReadData=I2cReadData_direct;
	if(debug)
	{
		Debug("Reading card 1:\n");
		ReadCard(0);
		Debug("\nReading card 2:\n");
		ReadCard(1);
		Debug("\nReading card 3:\n");
		ReadCard(2);
		Debug("\nReading card 4:\n");
		ReadCard(3);
	}
	return 0;
}

void  cleanup_module(void)
{
	printk("mod_k8000: Libk8000 kernel functions library unloaded\n");
}

EXPORT_SYMBOL(SelectI2CprinterPort);
EXPORT_SYMBOL(DA_matrix);
EXPORT_SYMBOL(AD_matrix);
EXPORT_SYMBOL(IO_matrix);
EXPORT_SYMBOL(DAC_matrix);
EXPORT_SYMBOL(I2cSendData);
EXPORT_SYMBOL(I2cReadData);
EXPORT_SYMBOL(ReadIOchip);
EXPORT_SYMBOL(ReadAllIO);
EXPORT_SYMBOL(ReadIOchannel);
EXPORT_SYMBOL(IOoutput);
EXPORT_SYMBOL(ClearIOchip);
EXPORT_SYMBOL(ClearIOchannel);
EXPORT_SYMBOL(SetIOchip);
EXPORT_SYMBOL(SetIOchannel);
EXPORT_SYMBOL(SetAllIO);
EXPORT_SYMBOL(ClearAllIO);
EXPORT_SYMBOL(UpdateIOdataArray);
EXPORT_SYMBOL(ClearIOdataArray);
EXPORT_SYMBOL(SetIOdataArray);
EXPORT_SYMBOL(SetIOchArray);
EXPORT_SYMBOL(ClearIOchArray);
EXPORT_SYMBOL(UpdateAllIO);
EXPORT_SYMBOL(UpdateIOchip);
EXPORT_SYMBOL(ConfigAllIOasInput);
EXPORT_SYMBOL(ConfigAllIOasOutput);
EXPORT_SYMBOL(ConfigIOchipAsInput);
EXPORT_SYMBOL(ConfigIOchipAsOutput);
EXPORT_SYMBOL(ConfigIOchannelAsInput);
EXPORT_SYMBOL(ConfigIOchannelAsOutput);
EXPORT_SYMBOL(OutputDACchannel);
EXPORT_SYMBOL(UpdateDACchip);
EXPORT_SYMBOL(UpdateDACchannel);
EXPORT_SYMBOL(ClearDACchannel);
EXPORT_SYMBOL(SetDACchannel);
EXPORT_SYMBOL(ClearDACchip);
EXPORT_SYMBOL(SetDACchip);
EXPORT_SYMBOL(UpdateAllDAC);
EXPORT_SYMBOL(ClearAllDAC);
EXPORT_SYMBOL(SetAllDAC);
EXPORT_SYMBOL(OutputDAchannel);
EXPORT_SYMBOL(UpdateDAchannel);
EXPORT_SYMBOL(ClearDAchannel);
EXPORT_SYMBOL(SetDAchannel);
EXPORT_SYMBOL(UpdateAllDA);
EXPORT_SYMBOL(ClearAllDA);
EXPORT_SYMBOL(SetAllDA);
EXPORT_SYMBOL(ReadADchannel);
EXPORT_SYMBOL(ReadADchip);
EXPORT_SYMBOL(ReadAllAD);
EXPORT_SYMBOL(ReadAll);
EXPORT_SYMBOL(ReadCard);
EXPORT_SYMBOL(UpdateAll);
EXPORT_SYMBOL(UpdateCard);
EXPORT_SYMBOL(SelectChain);
EXPORT_SYMBOL(SetI2cBusdelay);
EXPORT_SYMBOL(initialize);
EXPORT_SYMBOL(SetDebug);
EXPORT_SYMBOL(SetPrintError);

#endif
