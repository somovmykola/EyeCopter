CFLAGS = -std=gnu99
LFLAGS = -lwiringPi -lpigpio -lncurses -lpthread -lm -lrt


all:
	gcc -c -o ADNS_Sensor.o ADNS_Sensor.c $(CFLAGS)
	gcc -c -o sensorUI.o sensorUI.c $(CFLAGS)
	gcc -o sensorUI sensorUI.o ADNS_Sensor.o $(CFLAGS) $(LFLAGS)


.PHONY: clean

clean:
	rm -f sensorUI sensorUI.o ADNS_Sensor.o
