//throttle output program. 
//by default throttle is modulated to 1100ms.
//use updateD to control throttle.

#include <wiringPi.h>
#include <timing.h>

volatile double throttle;

int updateD(double d){
	if (d < 0) d = 0;
	else if (d > 1) d = 1;
	throttle = d;
	return 0;
}

int main(){

/*	if (p < 0 ) p = 0;
	else if (p > 1) p =1;*/

	throttle = 0.1979;

	wiringPiSetup();
	pinMode(0, OUTPUT);

	for(;;){
		digitalWrite(0, HIGH);
		usleep(910+throttle*970);
		digitalWrite(0, LOW);
		usleep(21090-throttle*970);
	}
	return 0;
}