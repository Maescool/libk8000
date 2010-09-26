#!/bin/python
import sys
import time
import k8000

def main():
    print " "
    print "     K8000-IO-AD : "
    
    # The Velleman K8000 is connected to parallel port 1
    # If you use the /dev/velleman interface, specify -1 or I2C_DEV
    if k8000.SelectI2CprinterPort(1):
            print "Error selecting the correct interface (remember to run as root)"
            sys.exit(1)
            
    while True:
        k8000.ReadCard(0)
        print "       ",
        for IOchannel in range(1,17):
            print k8000.IO_matrix_getitem(k8000.cvar.IO_matrix, k8000.cvar.activechainnumber, IOchannel),
            if (IOchannel % 4) == 0:
                print " ",
        print " ",
        for ADchannel in range(1,5):
            print "%d" % k8000.AD_matrix_getitem(k8000.cvar.AD_matrix, k8000.cvar.activechainnumber, ADchannel),
        print " "
        
        
    #That was easy wasn't it :-)
        
if __name__ == '__main__':
    main()