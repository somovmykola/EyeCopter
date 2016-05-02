CFLAGS = -std=gnu99
LFLAGS = -lm -lwiringPi -lpigpio -lrt -pthread

ODIR = build

_OBJ = i2c.o MakeI2CMessage.o ADNS_Sensor.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	gcc -c -o $@ $< $(CFLAGS) $(LFLAGS)

all: auto

auto: $(ODIR)/AutonomousFlightFinal.o $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o auto
