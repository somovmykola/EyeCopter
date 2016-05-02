#include <stdio.h>
//#include <wiringPiI2C.h>
#include "i2c.h"
#include "MakeI2CMessage.h"

#define MIN_LEN 888
#define MAX_LEN 1908

int fd;

void printHelp() {
    printf("r - read\n");
    printf("w - write\n");
    printf("h - help\n");
    printf("q - quit\n");
}

char* pNames[4] = {"PITCH", "YAW", "ROLL", "THROTTLE"};

int main(int argc, char* argv[]) {
    fd = i2c_init();

    
    // Read from all four channels
    uint8_t rMsg, reply;
    uint16_t len;
    
    for (int i = 0; i < 4; i++) {
        printf("%-8s: ", pNames[i]);
        
        // Read channel i from memory
        rMsg = makeReadMsg(0, i);
        
        // Send I2C message
        printf("TX [0x%02X]  ", rMsg);

        i2c_requestBytes(fd, &rMsg, 1, &reply, 1);

        printf("RX [0x%02X]    ", reply);

        len = ((uint16_t)reply << 2) + MIN_LEN;

        printf("len = %d\n", len);
    }

    i2c_close(fd);

    return 0;
}
