//test function that determines the length of a pwm
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>

//struct timeval start, end;
struct timespec start, stop;
double t;


void swap(){
	if(digitalRead(5)==1){
		//togl=!togl;
		clock_gettime(CLOCK_REALTIME, &start);
		//printf("posedge");

	}
	else{
		//togl=!togl;
		//printf("negedge");
		clock_gettime(CLOCK_REALTIME, &stop);
		printf("t=");
		t = (stop.tv_sec-start.tv_sec)*1000000000L + stop.tv_nsec-start.tv_nsec;
		printf("%ls \n", t);
	}

}

int main(){
	
	if(wiringPiSetup()==-1){
		printf("something went horribly wrong");
		return 0;
	}
	else{
		printf("youre dead");}
	pinMode(5,INPUT);	
	//printf("made it");

	wiringPiISR(5,INT_EDGE_BOTH, &swap);
  	for(;;) {
  		//printf("lol");
  		usleep(10000);}
	return 0;

}