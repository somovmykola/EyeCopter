#define TRIG 9
#define ECHO 10

void setup() {
  Serial.begin(9600);
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
}

long duration;

void loop() {
  if (Serial.available() > 0) {
    Serial.read();

    for (int i = 0; i < 30; i++ ) {
      digitalWrite(TRIG, HIGH);
      delayMicroseconds(20);  // only 10 uS needed
      digitalWrite(TRIG, LOW);
    
      duration = pulseIn(ECHO, HIGH);
      
      Serial.print(duration);
      Serial.print("uS\r\n");

      delay(100);
    }
  }
}
