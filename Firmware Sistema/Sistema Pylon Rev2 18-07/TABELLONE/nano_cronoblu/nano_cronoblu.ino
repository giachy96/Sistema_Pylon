
#include <AltSoftSerial.h>

AltSoftSerial altSerial;



String msg;




void setup() {

  Serial.begin(9600);
  delay(200);
  altSerial.begin(9600);

}


void loop() {


  //if (altSerial.available()) {
  //    msg = altSerial.read();
  //    Serial.println(msg);
  //  }

  altSerial.println("<SONO IL CRONOBLU>");
  delay(5000);






}



void readMasterPort() {

  while (altSerial.available()) {

    delay(10);
    if (altSerial.available() > 0) {

      char c = altSerial.read();  //gets one byte from serial buffer
      msg += c; //makes the string readString
    }
  }
  altSerial.flush();

}
