/***************************************************************************
                        mod_k8000.h  -  description
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
 * $Id: k8000.h,v 1.4 2005/07/30 10:53:23 js Exp $
 *
 */

#ifndef __K8000_H__     /* To prevent re-inclusion loops */
#define __K8000_H__

#ifndef __KERNEL__
  #include <sys/types.h>
#endif

#define MaxIOcard 3
#define MaxIOchip 7
#define MaxIOchannel 64
#define MaxDACchannel 32
#define MaxADchannel 16
#define MaxDAchannel 4
#define MaxChains 5
#define I2C_DEV -1

typedef unsigned char byte;

#ifdef __cplusplus 
extern "C" {
#endif

/* configuration and data matrix*/
extern short IOconfig_matrix[MaxChains][MaxIOchip+1];
extern short IOdata_matrix[MaxChains][MaxIOchip+1];
extern short IO_matrix[MaxChains][MaxIOchannel+1];
extern short DAC_matrix[MaxChains][MaxDACchannel+1];
extern short AD_matrix[MaxChains][MaxADchannel+1];
extern short DA_matrix[MaxChains][MaxDAchannel+1];

/* old configuration and data arrays  */
/* they just point to the first chain */
/* like this: IO[1] = IO_matrix[0][1] */
extern short *IOconfig;
extern short *IOdata;
extern short *IO;
extern short *DAC;
extern short *AD;
extern short *DA;

extern short ControlPort;
extern short StatusPort;
extern short DataPort;
extern int I2CbusDelay;
extern int activechainnumber;

/**************************************************************\
*  Generic IC input/output functions                          *
\**************************************************************/
extern void (*I2cSendData)(byte addr,byte *data,int len);
extern void (*I2cReadData)(byte addr,byte *data,int len);

/**************************************************************\
*  IO input procedures                                         *
\**************************************************************/
void ReadIOchip(int chip_no);
void ReadAllIO(void);
void ReadIOchannel(int channel_no);

/**************************************************************\
*  IO output procedures                                        *
\**************************************************************/
void IOoutput(int chip_no, short data);
void ClearIOchip(int chip_no);
void SetIOchip(int chip_no);
void SetIOchannel(int channel_no);
void ClearIOchannel(int channel_no);
void ClearAllIO(void);
void SetAllIO(void);

/**************************************************************\
*  IO data update procedures                                   *
\**************************************************************/
void UpdateIOchip(int chip_no);
void UpdateIOdataArray(int chip_no, short data);
void ClearIOdataArray(int chip_no);
void SetIOdataArray(int chip_no);
void SetIOchArray(int channel_no);
void UpdateAllIO(void);
void ClearIOchArray(int channel_no);

/**************************************************************\
*  IO config procedures                                        *
\**************************************************************/
void ConfigAllIOasInput(void);
void ConfigAllIOasOutput(void);
void ConfigIOchipAsInput(int chip_no);
void ConfigIOchipAsOutput(int chip_no);
void ConfigIOchannelAsInput(int channel_no);
void ConfigIOchannelAsOutput(int channel_no);

/**************************************************************\
*  DAC output procedures                                       *
\**************************************************************/
void OutputDACchannel(int channel_no, int data);
void UpdateDACchannel(int channel_no);
void ClearDACchannel(int channel_no);
void SetDACchannel(int channel_no);
void UpdateDACchip(int chip_no);
void ClearDACchip(int chip_no);
void SetDACchip(int chip_no);
void UpdateAllDAC(void);
void ClearAllDAC(void);
void SetAllDAC(void);

/**************************************************************\
*  DA output procedures                                        *
\**************************************************************/
void OutputDAchannel(int channel_no, int data);
void UpdateDAchannel(int channel_no);
void ClearDAchannel(int channel_no);
void SetDAchannel(int channel_no);
void UpdateAllDA(void);
void ClearAllDA(void);
void SetAllDA(void);

/**************************************************************\
*  AD input procedures                                         *
\**************************************************************/
void ReadADchannel(int channel_no);
void ReadADchip(int chip_no);
void ReadAllAD(void);

/**************************************************************\
*  Complete card and general procedures                        *
\**************************************************************/
void ReadAll(void);
void ReadCard(int card_no);
void UpdateAll(void);
void UpdateCard(int card_no);
void I2CBusNotBusy(void);
void SetI2cBusdelay(int d);
int  SelectI2CprinterPort(int Printer_no);
void SelectChain(int chain_no);
void SetDebug(int d);
void SetPrintError(int d);
void SetAutoPerm(int set);

#ifndef __KERNEL__
  int  CreateMutex(key_t key);
  int  LockMutex(int sem);
  int  UnlockMutex(int sem);
  void DisposeMutex(int sem);
  void SetMutexlock(int set);
#endif

/* For c++ linkage with c functions/variables defined in k8000.c */
#ifdef __cplusplus
}
#endif

#endif /* __K8000_H__ */
