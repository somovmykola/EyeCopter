//old deprecated pin I/O program.
//only used for testing at this point.
//use allFeeds in main from now on.

#include <wiringPi.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

//#define PERIOD 300
//#define ON_TIME 100

//bool cycle = true;

void setup(void){

    wiringPiSetup () ;
  //piHiPri(99);  
  pinMode (1, INPUT) ;
  pinMode(0, OUTPUT);
//  pullUpDnControl (0, PUD_UP);
 // pullUpDnControl (1, PUD_UP);


}

int main (void)
{

  setup();

  //int i;
  //clock_t high, low, t;

  //high = clock();
  //low = high + ON_TIME; 
  for (;;)
  {
   //t = clock();
        
        digitalWrite(0, digitalRead(1));
//        printf("%d\n", digitalRead(1));
//	digitalWrite(0, digitalRead(1));
//	digitalWrite(0, digitalRead(1));
//	digitalWrite(0, digitalRead(1));	
//        delay(500);

   }


   //if (t >=high){ digitalWrite (0, HIGH) ; high = t+400; low = t+200;}
   //if (t >= low){ digitalWrite (0,  LOW) ; low = t+400; high = t+200;}
  
  return 0 ;
}
