#!/bin/python
import sys
import time
import k8000

def main():
	
	print ""
	print "    K8000-running-light : "
	print ""
	print ""
	print "           Set the IO channels and LED's one by one ON"
	print ""
	print ""
	print "           And the set the IO channels and LED's one by one OFF"
	print ""
	print ""
	
	# Enable output of libk8000 errors
	k8000.SetPrintError(1);

  	# Enable debug output
  	k8000.SetDebug(1);
  
	# The Velleman K8000 is connected to parallel port 1
	# If you use the /dev/velleman interface, specify -1 or I2C_DEV
	if k8000.SelectI2CprinterPort(1):
		print "Error selecting the correct interface (remember to run as root)"
		sys.exit(1)
	
	# configure IO ports
	k8000.ConfigAllIOasOutput()	# Configure all IO channels to be used as OUTPUT channels.
								# Try to replace this line with 'ConfigAllIOasInput.
								# Then nothing will happen.
								# No leds lightup.
								# LIBK8000 knows a input cannot beused as output :-)
	
	for led in range(1,17):
		k8000.SetIOchannel(led) # Set IO number 'led' ON.
		time.sleep(0.5) # A delay of 0.5 seconds.
		
	for led in range(1,17):
		k8000.ClearIOchannel(led) # Set IO number 'led' OFF.
		time.sleep(0.5) # A delay of 0.5 seconds.
	
	print ""
	print "						Running Light Program ended..."
	print ""

if __name__ == '__main__':
    main()