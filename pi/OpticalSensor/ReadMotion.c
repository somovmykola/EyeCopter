#include <stdio.h>
#include "ADNS_Sensor.h"
#include <wiringPi.h>

ADNS_Sensor sensor;

int main(int argc, char **argv) {
	// SETUP:
	if (ADNS_init(&sensor) < 0)
		fprintf(stderr, "Failed to initialize ADNS3080\n");

	delay(200);

	// MAIN LOOP:
	while (1) {
		ADNS_update(&sensor);
		//ADNS_update_position(&sensor, 0, 0, 0, 100);

		// check for errors
		if (sensor.has_overflow)
			printf("overflow!!\n");

		// TODO: make these numbers fixed length so it looks pretty
		// when outputting. Maybe remove x and only keep DX
		printf("\r");
		printf("dx: %3d\t", sensor.dx);
		printf("dy: %3d\t", sensor.dy);
		printf("squal: %03d", sensor.surface_quality);
		fflush(stdout);

		delay(500);	// ms
	}

	return 0;
}
