/***************************************************************************
                          k8000-chains.c  -  description
                             -------------------
    begin                : Mon Apr 1 2002
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
#include <k8000.h>  // Makes the LIBK8000 library for the Velleman K8000 available.
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  int i;
  unsigned char str[4];
  byte val; /* byte is defined in k8000.h as unsigned char */

  //SetDebug(1);
  // The Velleman K8000 is connected to parallel port 1
  // If you use the /dev/velleman interface, specify -1 or I2C_DEV
  if(SelectI2CprinterPort(1))
  {
    printf("Error selecting the correct interface (remember to run as root) : %s\n",strerror(errno));
    exit(1);
  }

  /* configure all the IO's as outputs (this clears the outputs) */
  ConfigAllIOasOutput();

  printf("==== Testing chain 0 (first default chain) ====\n");
  /* select the first chain (default) */
  SelectChain(0);

  /* light IO ports and read their status*/
  for (i=1;i<=16;i++)
  {
    printf("IO[%d] on chain %d = %d,",i,activechainnumber,IO_matrix[activechainnumber][i]);
    SetIOchannel(i);
    printf("  now IO[%d] = %d\n",i,IO_matrix[activechainnumber][i]);
    usleep(50000);
  }

  printf("===============================================\n\n");
  printf("==== Testing chain 1 ==========================\n");
  /* select the second chain */
  SelectChain(1);
  ConfigAllIOasOutput();
  /* light IO ports and read their status*/
  for (i=1;i<=16;i++)
  {
    printf("IO[%d] on chain %d = %d,",i,activechainnumber,IO_matrix[activechainnumber][i]);
    SetIOchannel(i);
    printf("  now IO[%d] = %d\n",i,IO_matrix[activechainnumber][i]);
    usleep(50000);
  }

  /* send a string to an I²C device at address 0x50 */
  /* Sending the string "text" to an LCD */
  /* panel at address 0x50 on chain 0    */
  printf("===============================================\n\n");
  printf("== Sending 'text' to device with address 0x50==\n");

  SelectChain(0);
  I2CBusNotBusy();
  str[0]='t';
  str[1]='e';
  str[2]='x';
  str[3]='t';
  I2cSendData(0x50,str,4);

  printf("===============================================\n\n");
  printf("== Direct K8000 control =======================\n");
  /* control IO's 1-8 on chain 0 directly  */
  /* By controlling the K8000 directly     */
  /* using I2cSendData and I2cReadData,    */
  /* the IO(_matrix) array will not be     */
  /* updated!                              */
  SelectChain(0);  /* make chain 0 active */

  /* set the IO's low */
  val = 0xFF;
  I2cSendData(0x38,&val,1); /* write the value byte to the chip */
  I2cReadData(0x38,&val,1); /* read status into the value byte from the chip */
  printf("Value read from device 0x38 = %x\n",val);
  usleep(500000);

  /* set the IO's back high */
  val = 0x00;
  I2cSendData(0x38,&val,1); /* write the value byte to the chip */
  I2cReadData(0x38,&val,1); /* read status into the value byte from the chip */
  printf("Value read from device 0x38 = %x\n",val);

  printf("===============================================\n\n");
  return 0;
}
