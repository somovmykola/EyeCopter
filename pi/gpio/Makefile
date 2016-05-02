CFLAGS = -std=gnu99
LFLAGS = -lm -lwiringPi

ODIR = build

_OBJ = 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	gcc -c -o $@ $< $(CFLAGS) $(LFLAGS)

all: sensor

sensor: $(ODIR)/sensor.o $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o motion test
