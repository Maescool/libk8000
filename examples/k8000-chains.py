#!/bin/python
import sys
import time
import k8000

def main():
    # The Velleman K8000 is connected to parallel port 1
    # If you use the /dev/velleman interface, specify -1 or I2C_DEV
    if k8000.SelectI2CprinterPort(1):
            print "Error selecting the correct interface (remember to run as root)"
            sys.exit(1)
    
    print "==== Testing chain 0 (first default chain) ====";
    # select the first chain (default)
    k8000.SelectChain(0)
    # configure IO ports
    k8000.ConfigAllIOasOutput()
    
    # light IO ports and read their status
    for IOchannel in range(1,17):
    	print "IO[%d] on chain %d = %d" % (IOchannel, k8000.cvar.activechainnumber, k8000.IO_matrix_getitem(k8000.cvar.IO_matrix, k8000.cvar.activechainnumber, IOchannel)),
    	k8000.SetIOchannel(IOchannel)
    	print "  now IO[%d] = %d" % (IOchannel, k8000.IO_matrix_getitem(k8000.cvar.IO_matrix, k8000.cvar.activechainnumber, IOchannel))
    	time.sleep(0.05)
    
    print "===============================================\n"
    print "==== Testing chain 1 =========================="
    # select the second chain
    k8000.SelectChain(1)
    # configure IO ports
    k8000.ConfigAllIOasOutput()
    # light IO ports and read their status
    for IOchannel in range(1,17):
    	print "IO[%d] on chain %d = %d" % (IOchannel, k8000.cvar.activechainnumber, k8000.IO_matrix_getitem(k8000.cvar.IO_matrix, k8000.cvar.activechainnumber, IOchannel)),
    	k8000.SetIOchannel(IOchannel)
    	print "  now IO[%d] = %d" % (IOchannel, k8000.IO_matrix_getitem(k8000.cvar.IO_matrix, k8000.cvar.activechainnumber, IOchannel))
    	time.sleep(0.05)
    
    # send a string to an I<B2>C device at address 0x50
    # Sending the string "text" to an LCD
    # panel at address 0x50 on chain 0
    print "===============================================\n"
    print "== Sending 'text' to device with address 0x50=="
    
    k8000.SelectChain(0)
    k8000.I2CBusNotBusy()
    ##
    ## NOTE: this part needs to be fixed, i'm not skilled enough in python/swig to fix this yet..
    ##
    str = "text"
    k8000.I2cSendData(b"0x50",str,len(str))
    
    print "===============================================\n"
    print "== Direct K8000 control ======================="
    # control IO's 1-8 on chain 0 directly
    # By controlling the K8000 directly
    # using I2cSendData and I2cReadData,
    # the IO(_matrix) array will not be
    # updated!
    k8000.SelectChain(0);  # make chain 0 active
    
    # set the IO's low
    val = "0xFF"
    k8000.I2cSendData(chr("0x38"),chr(val),1) # write the value byte to the chip
    k8000.I2cReadData(chr("0x38"),val,1) # read status into the value byte from the chip
    print "Value read from device 0x38 = %x" % val
    time.sleep(0.5)
    
    # set the IO's back high
    val = "0x00"
    k8000.I2cSendData("0x38",chr(val),1) # write the value byte to the chip
    k8000.I2cReadData("0x38",val,1) # read status into the value byte from the chip
    print "Value read from device 0x38 = %x" % val
    
    print "===============================================\n"
    return 0

if __name__ == '__main__':
    main()