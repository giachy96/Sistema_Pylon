#include <SoftwareSerial.h>
SoftwareSerial ArduinoSlave(2, 3);

void setup() {
  Serial.begin(9600);
  ArduinoSlave.begin(9600);
}
void loop() {
  msg = "STOP";
  ArduinoSlave.print(msg);
  delay(5000);
  msg = "GO";
  ArduinoSlave.print(msg);
  delay(5000);
}
