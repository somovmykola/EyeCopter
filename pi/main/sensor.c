//reads inputs from the sensors and returns a distance long when getDist is called.

#include <wiringPi.h>
#include <timing.h>

struct timeval start;
struct timeval end;
bool togl = true;
long time;

long getDist(){
	long dist;
	dist = (time + 191.3)/57.3;
	return dist;
}

void edge(){
	if(togl){
		togl=!togl;
		gettimeofday(&start,0);

	}
	else{
		togl=!togl;
		gettimeofday(&end,0);
		time = (end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec;
	}

}

int main (void)

{
  wiringPiSetup () ;
  pinMode (0, INPUT) ;
  
  wiringPiISR(0,INT_EDGE_BOTH, &edge);
  for(;;){
  	sleep(10);
  }
  return 0 ;
}