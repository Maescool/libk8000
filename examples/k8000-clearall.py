#!/bin/python
import sys
import time
import k8000

def main():
    print " "
    print "    K8000-clearall : "
    
    k8000.SetDebug(1)
    k8000.SetPrintError(1)
    # The Velleman K8000 is connected to parallel port 1
    # If you use the /dev/velleman interface, specify -1 or I2C_DEV
    if k8000.SelectI2CprinterPort(1):
            print "Error selecting the correct interface (remember to run as root)"
            sys.exit(1)
    
    # configure IO ports
    k8000.ConfigAllIOasOutput()
    
    k8000.ClearAllIO();  # Sets all IO output channels to zero.
    k8000.ClearAllDA();  # Sets all DA output channels to zero.
    k8000.ClearAllDAC(); # Sets all DAC output channels to zero.
    
    print " "
    print "           All IO output channels are set to zero"
    print "           All DA output channels are set to zero"
    print "           All DAC output channels are set to zero"
    print " "

if __name__ == '__main__':
    main()