//pinOff and pinOn are purely test programs that turn a pin on/off.


#include <stdio.h>
#include <wiringPi.h>

int main() {

	wiringPiSetup();
	pinMode(8, OUTPUT);
	digitalWrite(8, HIGH);


	return 0;
}
