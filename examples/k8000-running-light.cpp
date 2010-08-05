/*
By hihihi@wanadoo.nl
Feb 7 2002
This is freeware.

Example on how to use the LIBK8000 version 2.0.0.

K8000-RUNNING-LIGHT:


  The program sets the IO channels and LED's one by one ON.
  And then the program set the IO channels and LED's one by one OFF.

  Disconnect every thing from the IO channel before running this !



COMPILE WITH :

     g++ -O3 k8000-running-light.cpp -o k8000-running-light -lk8000

*/

#include <k8000.h>  // Makes the LIBK8000 library for the Velleman K8000 available.
#include <iostream> // Makes it possible to print to screen.
#include <unistd.h> // To use the usleep(.....) delay
#include <errno.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

int main()
{

  cout<<endl;
  cout<<endl;
  cout<<"    K8000-running-light : "<<endl;
  cout<<endl;
  cout<<endl;
  cout<<"           Set the IO channels and LED's one by one ON"<<endl;
  cout<<endl;
  cout<<endl;
  cout<<"           And the set the IO channels and LED's one by one OFF"<<endl;
  cout<<endl;
  cout<<endl;

  // Enable output of libk8000 errors
  SetPrintError(1);

  // Enable debug output
  SetDebug(1);

  // The Velleman K8000 is connected to parallel port 1
  // If you use the /dev/velleman interface, specify -1 or I2C_DEV
  if(SelectI2CprinterPort(1))
  {
    cout << "Error selecting the correct interface (remember to run as root) : " << strerror(errno)<<endl;
    exit(1);
  }

  ConfigAllIOasOutput();  // Configure all IO channels to be used as OUTPUT channels.
                          // Try to replace this line with 'ConfigAllIOasInput.
                          // Then nothing will happen.
                          // No leds lightup.
                          // LIBK8000 knows a input cannot beused as output :-)
 
  for(int led=1;led<=16;led++){  

    SetIOchannel(led);   // Set IO number 'led' ON.

    usleep(500*1000);    // A delay of 0.5 seconds.
  }

  for(int led=1;led<=16;led++){

    ClearIOchannel(led);   // Set IO number 'led' OFF.

    usleep(500*1000);    // A delay of 0.5 seconds.
  }

  cout<<endl;
  cout<<"                        Running Light Program ended..."<<endl;
  cout<<endl;
  cout<<endl;

  // That was easy wasn't it :-)
  
}

