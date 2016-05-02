//test program that causes a pin of choice to blink.

#include <wiringPi.h>
int main (void)
{
  wiringPiSetup () ;
  piHiPri(99);  
  pinMode (0, OUTPUT) ;
  for (;;)
  {
    digitalWrite (1, HIGH) ; delayMicroseconds (2000) ;
    digitalWrite (1,  LOW) ; delayMicroseconds (2000) ;
  }
  return 0 ;
}
