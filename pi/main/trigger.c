//trigger function. sends a very soft real time pwm to the sensors every so often to trigger a response

#include <wiringPi.h>


int main(){
	wiringPiSetup();
	pinMode(0, INPUT);
	pinMode(1, INPUT);

	for(;;){

		digitalWrite(0,HIGH);
		digitalWrite(1,HIGH);
		usleep(20);
		digitalWrite(0,LOW);
		digitalWrite(1,LOW);		

		wait(1);
	}

	return 0;
}