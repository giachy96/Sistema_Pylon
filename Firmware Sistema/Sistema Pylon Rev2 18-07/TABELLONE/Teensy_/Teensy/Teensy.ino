

const byte numChars = 64;
char receivedChars1[numChars];
char receivedChars2[numChars];
char receivedChars3[numChars];
char receivedChars4[numChars];
char receivedChars5[numChars];
char receivedChars6[numChars];
char receivedChars7[numChars];
char receivedChars8[numChars];
boolean newData = false;

String msg1 ;
String msg2 ;
String msg3 ;
String msg4 ;
String msg5 ;
String msg6 ;
String msg7 ;
String msg8 ;



void readMasterPort2() {

  while (Serial2.available()) {

    delay(10);
    if (Serial2.available() > 0) {

      char c = Serial2.read();  //gets one byte from serial buffer
      msg2 += c; //makes the string readString
    }
  }
  Serial2.flush();

}



void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); // cronoblu
  Serial2.begin(9600); // palo 2/3 blu
  Serial3.begin(9600); // palo1
  Serial4.begin(9600); // palo 2/3 verde
  Serial5.begin(9600); // palo 2/3 rosso
  Serial6.begin(9600); // centrale
  Serial7.begin(9600); // cronoverde
  Serial8.begin(9600); // cronorosso

}


// the loop routine runs over and over again forever:
void loop() {
  // msg = "STOP";
  //  Serial3.print(msg);
  //  Serial.print(msg);
  // delay(5000);
  //  msg = "GO";
  // Serial3.print(msg);
  // Serial.print(msg);
  //  delay(5000);
  //
  //    if (Serial2.available()>0){
  //      char c = Serial2.read();  //gets one byte from serial buffer
  //      msg2 += c; //makes the string readString
  //      }
  //        Serial.println(msg2);

  recvWithStartEndMarkers(Serial1 , receivedChars1 );
  showNewData(receivedChars1);
  delay(200);
  recvWithStartEndMarkers(Serial2 , receivedChars2 );
  showNewData(receivedChars2);
  delay(200);
  recvWithStartEndMarkers(Serial4 , receivedChars4 );
  showNewData(receivedChars4);
  delay(200);
  recvWithStartEndMarkers(Serial5 , receivedChars5 );
  showNewData(receivedChars5);
  delay(200);
  recvWithStartEndMarkers(Serial6 , receivedChars6 );
  showNewData(receivedChars6);
  delay(200);
  recvWithStartEndMarkers(Serial7 , receivedChars7 );
  showNewData(receivedChars7);
  delay(200);
  recvWithStartEndMarkers(Serial8 , receivedChars8 );
  showNewData(receivedChars8);
  delay(250);
  Serial6.print("<STOP>");
  delay(2000);
  Serial6.print("<GOOO>");
  delay(2000);


  //
  //   if (Serial3.available()) {
  //   // readMasterPort(Serial3 ,msg3);
  //    Serial.println(msg3);
  //  }
  //   if (Serial4.available()) {
  //  //  readMasterPort(Serial4 ,msg4);
  //    Serial.println(msg4);
  //  }
  //   if (Serial5.available()) {
  //   // readMasterPort(Serial5 ,msg5);
  //    Serial.println(msg5);
  //  }
  //   if (Serial8.available()) {
  //    //readMasterPort(Serial8 ,msg8);
  //    Serial.println(msg8);
  //  }


}



void  recvWithStartEndMarkers( HardwareSerial &port , char receivedChars[numChars] ) {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (port.available() > 0 && newData == false) {
    rc = port.read();

    if (recvInProgress == true) {

     
        if (rc != endMarker) {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= numChars) {
            ndx = numChars - 1;
          }
        }
        else {
          receivedChars[ndx] = '\0'; // terminate the string
          recvInProgress = false;
          ndx = 0;
          newData = true;
        }
      
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showNewData(char receivedChars[numChars]) {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}
