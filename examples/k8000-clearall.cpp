/*
By hihihi@wanadoo.nl
Feb 7 2002
This is freeware.

Example on how to use the LIBK8000 version 2.0.0.

It sets all output channels to zero.

This is a usefull prog to have around when you are experimenting with the K8000.
Because when you exit a prog the IO, DA and DAC channels are left in the state the
prog has set them.
This could mean the output channels are still set to a value higher then zero.

With this prog you can quickly set everything to zero.
Why don't you place it in /usr/local/bin so that you can get to it quickly.


COMPILE WITH :

     g++ -O3 k8000-clearall.cpp -o k8000-clearall -lk8000++

*/

#include <k8000.h>  // Makes the LIBK8000 library for the Velleman K8000 available.
#include <iostream> // Makes it possible to print to screen.
#include <errno.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int main()
{

  cout<<endl;
  cout<<endl;
  cout<<"    K8000-clearall : "<<endl;

  SetDebug(1);
  SetPrintError(1);
  // The Velleman K8000 is connected to parallel port 1
  // If you use the /dev/velleman interface, specify -1 or I2C_DEV
  if(SelectI2CprinterPort(1))
  {
    cout << "Error selecting the correct interface (remember to run as root) : " << strerror(errno)<<endl;
    exit(1);
  }

  ClearAllIO();  // Sets all IO output channels to zero.
  ClearAllDA();  // Sets all DA output channels to zero.        
  ClearAllDAC(); // Sets all DAC output channels to zero.

  cout<<endl;
  cout<<"           All IO output channels are set to zero"<<endl;
  cout<<"           All DA output channels are set to zero"<<endl;
  cout<<"           All DAC output channels are set to zero"<<endl;
  cout<<endl;
  cout<<endl;

  // That was easy wasn't it :-)
  
}

