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

int main(int argc, char* argv[]) {
    fd = i2c_init();

    printf("fd = %d\n", fd);
    printf("type h for help\n");

    char c = 0;
    uint8_t port, src;
    uint16_t len;

    uint8_t rMsg;
    uint16_t wMsg;
    while(c != 'q') {
        printf(":");
        c = getchar();
        getchar();  // consume newline

        if (c == 'r') {
            printf("Read from (m)emory or (r)adio?\n");

            c = getchar();
            getchar();  // consume newline

            if (c == 'm') {
                src = 0;
            } else if (c == 'r') {
                src = 1;
            } else {
                printf("unknown command \'%c\'\n", c);
                continue;
            }

            printf("read (p)itch, (y)aw, (r)oll, or (t)hrottle?\n");

            c = getchar();
            getchar();  // consume newline

            if (c == 'p')       port = 0;
            else if (c == 'y')  port = 1;
            else if (c == 'r')  port = 2;
            else if (c == 't')  port = 3;
            else {
                printf("unknown command \'%c\'\n", c);
                continue;
            }

            // Generate read message
            rMsg = makeReadMsg(src, port);

            // Send I2C message
            printf("Sent [0x%02X]\n", rMsg);

            uint8_t reply;
            i2c_requestBytes(fd, &rMsg, 1, &reply, 1);

            printf("Received 0x%02X\n", reply);

            uint16_t len = ((uint16_t)reply << 2) + MIN_LEN;

            printf("len = %d\n", len);
        } else if (c == 'w') {
            printf("Write to (p)itch, (y)aw, (r)oll, or (t)hrottle?\n");

            c = getchar();
            getchar();  // consume newline

            if (c == 'p')       port = 0;
            else if (c == 'y')  port = 1;
            else if (c == 'r')  port = 2;
            else if (c == 't')  port = 3;
            else {
                printf("unknown command \'%c\'\n", c);
                continue;
            }

            printf("Pulse length to write [%d - %d]\n", MIN_LEN, MAX_LEN);

            scanf("%d", &len);
            getchar();  // consume newline

            if (len < MIN_LEN) {
                printf("%d is too small\n", len);
                continue;
            } else if (len > MAX_LEN) {
                printf("%d is too large\n", len);
                continue;
            }

            // Generate write message
            wMsg = makeWriteMsg(port, len);

            uint8_t wMsgBytes[2];
            wMsgBytes[0] = (uint8_t)(wMsg >> 8);
            wMsgBytes[1] = (uint8_t)(wMsg & 0xFF);

            // Send I2C message
            printf("Sent [0x%02X 0x%02X]\n", wMsgBytes[0], wMsgBytes[1]);
            i2c_writeBytes(fd, wMsgBytes, 2);
        } else if (c == 'h') {
            printHelp();
        } else if (c == 'q') {
            break;
        }
    }

    i2c_close(fd);

    return 0;
}
