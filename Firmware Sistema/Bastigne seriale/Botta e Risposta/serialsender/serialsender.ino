#include <SoftwareSerial.h>
SoftwareSerial ArduinoSlave(2, 3);
String msg;
void setup() {
  Serial.begin(9600);
  Serial.println("ENTER Commands:");
  ArduinoSlave.begin(9600);
  }
void loop() {
  //Read command from monitor
  readSerialPort();
  //Send data to slave
  if (msg != "") {
    Serial.print("Master sent : ");
    Serial.println(msg);
    ArduinoSlave.print(msg);
    msg = "";
    }
}
void readSerialPort() {
  while (Serial.available()) {
    delay(10);
    if (Serial.available() > 0) {
      char c = Serial.read();//gets one byte from serial buffer
      msg += c; //add to String
    }
  }
  Serial.flush(); //clean buffer
}
