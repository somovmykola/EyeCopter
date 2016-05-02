#include <Wire.h>

#define SLAVE_ADDRESS 0x69

void setup() {
  Wire.begin();
  
  Serial.begin(9600);
  while (!Serial) {
  }
}

void loop() {
  Wire.requestFrom(SLAVE_ADDRESS, 1);
  while(Wire.available()) {
    //Serial.print("Received 0x");
    uint8_t r = Wire.read();
    //Serial.print(r, HEX);
    //Serial.print(" = ");
    Serial.println(r, DEC);
    delay(50);
  }

  delay(50);
}
