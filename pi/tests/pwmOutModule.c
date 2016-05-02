
//pwm output test file. give this file an integer argument at 
//CMD ant it sets up a pwm of that width in us

#include <linux/module.h>
#include <linux/kernel.h>
//#include <sched.h>
#include <stdio.h>
#include <wiringPi.h>

int main( int argc, char **argv){
	
//	struct sched_param sp;

//	sched_setscheduler(0, SCHED_FIFO, &sp);

	if (argc!=2){
		printf("lol");
		return 0;
	}

	wiringPiSetup();
	//piHiPri(99);
	pinMode(0, OUTPUT);

	int pwm = atoi(argv[1]);
	pwm -= 85;

	for(;;){
		digitalWrite(0, HIGH);
		usleep(pwm);
		digitalWrite(0, LOW);
		usleep(21090-pwm);
	}

	return 0;
}