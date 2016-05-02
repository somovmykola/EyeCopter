#include <Wire.h>

#define SLAVE_ADDRESS 0x69

uint8_t outCmd[4] = {
  0x01, // Read Memory Pitch
  0x10, // Read Memory Yaw
  0x20, // Read Memory Roll
  0x31, // Read Memory Throttle
};

uint8_t wCmd[8] = {
  0x88, 0x5B, // Write Pitch 1069
  0x98, 0x5A, // Write Yaw 1069
  0xA8, 0x5A, // Write Roll 1069
  0xB8, 0x5B, // Write Throttle 1069
};

void setup() {
  Wire.begin();
  
  Serial.begin(9600);
  while (!Serial) {
  }
}

void loop() {
  while(Serial.available() > 0) {
    uint8_t c = Serial.read();

    if (c < '1' || c > '4')
      continue;

    uint8_t n = c - '1';


    // Read
    Serial.print("Sending command 0x");
    Serial.println(outCmd[n], HEX);

    delay(100);

    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(outCmd[n]);
    Wire.write(0x00);
    Wire.endTransmission();

    delay(100);

    Wire.requestFrom(SLAVE_ADDRESS, 1);
    while(Wire.available()) {
      Serial.print("Received 0x");
      uint8_t r = Wire.read();
      Serial.print(r, HEX);
      Serial.print(" = ");
      Serial.println(((uint16_t)r << 2) + 888, DEC);
      delay(50);
    }

    Serial.println();



    // Write
    Serial.print("Sending command 0x");
    Serial.print(wCmd[2*n + 0], HEX);
    Serial.print(", 0x");
    Serial.println(wCmd[2*n + 1], HEX);

    delay(100);

    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(wCmd[2*n + 0]);
    Wire.write(wCmd[2*n + 1]);
    Wire.endTransmission();

    delay(100);

    Serial.println();




    // Read
    Serial.print("Sending command 0x");
    Serial.println(outCmd[n], HEX);

    delay(100);

    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(outCmd[n]);
    Wire.endTransmission();

    delay(100);

    Wire.requestFrom(SLAVE_ADDRESS, 1);
    while(Wire.available()) {
      Serial.print("Received 0x");
      uint8_t r = Wire.read();
      Serial.print(r, HEX);
      Serial.print(" = ");
      Serial.println(((uint16_t)r << 2) + 888, DEC);
      delay(50);
    }

    
    Serial.println();
    /**/
  }
}
