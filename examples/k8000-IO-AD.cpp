/*
By hihihi@wanadoo.nl
Feb 7 2002
This is freeware.

Example on how to use the LIBK8000 version 2.0.0 .

K8000-IO-AD:

  Read the IO and AD from the first card.
  And display the values on screen.


COMPILE WITH :

     g++ -O3 k8000-IO-AD.cpp -o k8000-IO-AD -lk8000++

 Press CTRL-C to end this prog.

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
  cout<<"    K8000-IO-AD : "<<endl;
  cout<<endl;
  cout<<endl;
  cout<<"           "<<endl;
  cout<<endl;
  cout<<endl;
  cout<<"           "<<endl;
  cout<<endl;
  cout<<endl;

  // The Velleman K8000 is connected to parallel port 1
  // If you use the /dev/velleman interface, specify -1 or I2C_DEV
  if(SelectI2CprinterPort(1))
  {
    cout << "Error selecting the correct interface (remember to run as root) : " << strerror(errno)<<endl;
    exit(1);
  }

  for(;;){

    ReadCard(0);           // read all channels of k8000 card number zero.

    cout<<"       ";
    for(int IOchannel=1;IOchannel<=16;IOchannel++){
      cout<<IO[IOchannel];      // All values of IO channels are stored in IO[]
      if((IOchannel%4)==0){
        cout<<" ";
      }
    } 
    cout<<" ";
   
    for(int ADchannel=1;ADchannel<=4;ADchannel++){
      cout<<AD[ADchannel];      // All values of AD channels are stored in AD[]
      cout<<" ";
    } 
    cout<<endl;
  } 
  

  // That was easy wasn't it :-)
  
}

