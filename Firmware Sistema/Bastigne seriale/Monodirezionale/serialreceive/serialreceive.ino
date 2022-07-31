#include <SoftwareSerial.h>

SoftwareSerial ArduinoMaster(4, 5);

String msg;
void setup() {
  Serial.begin(9600);
  ArduinoMaster.begin(9600);
}
void loop() {
  readMasterPort();
  // Send answer to master
  if (msg != "") {
    Serial.print("Master sent : " );
    Serial.println(msg);
    msg = "";
  }
}
void readMasterPort() {
  while (ArduinoMaster.available()) {
    delay(10);
    if (ArduinoMaster.available() > 0) {
      char c = ArduinoMaster.read();  //gets one byte from serial buffer
      msg += c; //makes the string readString
    }
  }
  ArduinoMaster.flush();
}
