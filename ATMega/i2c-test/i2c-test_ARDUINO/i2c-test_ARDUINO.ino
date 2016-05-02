// Quick testbed program to test I2C communication on the ATMega328P
// Open up the serial monitor, send a '1' to turn the microcontroller's
// LED on, and send a a '0' to turn it off. The LED on the arduino will
// mirror the expected output from the ATMega's LED.

// Connection:
// ARDUINO <->   ATMEGA
//   A5    <->  SCL (28)
//   A4    <->  SDA (27)
//  +5V    <->    +5V
//  GND    <->    GND

#include <Wire.h>

#define SLAVE_ADDRESS 0x69

#define ON_CODE   0x12
#define OFF_CODE  0x34

#define LED_PIN       13

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

byte x = 0;

void turnON() {
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ON_CODE);
  Wire.endTransmission();

  
  digitalWrite(LED_PIN, HIGH);
}

void turnOFF() {
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(OFF_CODE);
  Wire.endTransmission();
  
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    Serial.print("Sent 0x");
    switch(Serial.read()) {
    case '0':
      Serial.println(OFF_CODE, HEX);
      turnOFF();
      break;
    case '1':
      Serial.println(ON_CODE, HEX);
      turnON();
      break;
    default:
      break;
    }
    delay(50);
    Wire.requestFrom(SLAVE_ADDRESS, 1);
    while(Wire.available()) {
      Serial.print("Received 0x");
      Serial.println(Wire.read(), HEX);
    }
  }
}
