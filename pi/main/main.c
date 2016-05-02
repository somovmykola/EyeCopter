//main function to be run. when complete will create threads encapsulating the other
//pi programs to run concurrently, then enters a wait loop.

#include <wiringPi.h>
#include <timing.h>
#include <pthread.h>

int main(){


	pthread_create(1, NULL, &throttleOut, NULL);
	pthread_create(2, NULL, &yawOut, NULL);
	pthread_create(3, NULL, &pitchOut, NULL);
	pthread_create(4, NULL, &rollOut, NULL);
	pthread_create(5, NULL, &USSensor, 0);
	pthread_create(6, NULL, &USSensor, 1);
	pthread_create(7, NULL, &optical, NULL);
	pthread_create(8, NULL, &trigger, NULL);
	pthread_create(9, NULL, &serial, NULL);
	pthread_create(10, NULL, &watcher, NULL);
	for(;;){
		wait(1);
	}
	return 0;
}