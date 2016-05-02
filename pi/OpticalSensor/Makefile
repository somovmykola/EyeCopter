CFLAGS = -std=gnu99
LFLAGS = -lm -lwiringPi

ODIR = build

_OBJ = ADNS_Sensor.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	gcc -c -o $@ $< $(CFLAGS) $(LFLAGS)

all: motion test

motion: $(ODIR)/ReadMotion.o $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

test: $(ODIR)/test.o $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o motion test
