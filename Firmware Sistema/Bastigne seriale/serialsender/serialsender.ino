//Sender
#include "SoftwareSerial.h"
SoftwareSerial Myserial (3,2);
char mystr[5] = "Hello"; //String data

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);
  Myserial.begin(9600);
}

void loop() {
  Myserial.write(mystr,5); //Write the serial data
  delay(1000);
}
