CFLAGS = -std=gnu99
LFLAGS = 

ODIR = build

_OBJ = MakeI2CMessage.o i2c.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	gcc -c -o $@ $< $(CFLAGS) $(LFLAGS)

all: i2c-test i2c-sweep

i2c-test: $(ODIR)/i2c-test.o $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)
    
i2c-sweep: $(ODIR)/i2c-sweep.o $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o i2c-test
