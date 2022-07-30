//reeive
#include "SoftwareSerial.h"
SoftwareSerial Myserial (2,3);
char mystr[10]; //Initialized variable to store recieved data

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);
  Myserial.begin(9600);
}

void loop() {
 Myserial.readBytes(mystr,5); //Read the serial data and store in var
  Serial.println(mystr); //Print data on Serial Monitor
  delay(1000);
}
