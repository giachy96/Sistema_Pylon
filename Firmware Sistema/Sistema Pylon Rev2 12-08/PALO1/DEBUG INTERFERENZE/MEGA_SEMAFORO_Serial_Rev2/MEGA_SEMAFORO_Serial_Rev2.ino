#include "AltSoftSerial.h"
AltSoftSerial AltSerial ;

unsigned long Timeserial1 = 0;
unsigned long Timeserial2 = 0;
unsigned long Timeserial3 = 0;
unsigned long Timesend = 0;
unsigned long Currentmilliss = 0;
unsigned long Currentmillisx = 0;
unsigned long DelayLight = 2000;
unsigned long Delayserialx = 0;

String Dateserial1 = "0";
String Dateserial2 = "0";
String Dateserial3 = "0";
String Dateserial1p = "0";
String Dateserial2p = "0";
String Dateserial3p = "0";
String Dateserial;
const byte numChars = 32;
char RecCh1[numChars];
char RecCh2[numChars];
char RecCh3[numChars];
char  RecChAltSerial[numChars];
int FlagState = 200;

//Inizio Configurazione Mega
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Startup = "1000";
String end10lapB = "5534";
String end10lapV = "5524";
String end10lapR = "5514";
String State = Startup;
String OldState = Startup;
unsigned long Delaysend = 0;

//Fine Configurazione Mega

boolean newData1 = false;
boolean newData2 = false;
boolean newData3 = false;
boolean newDataAltSerial = false;

void setup() {
  Serial1.begin(9600);  // Serial Rosso
  delay(300);
  Serial2.begin(9600);  // Serial Verde
  delay(300);
  Serial3.begin(9600);  // Serial Blu
  delay(300);
  Serial.begin(9600);  // Serial Pc interface
  delay(300);
  AltSerial.begin(9600);
  delay(300);
  // Serial.println("prova");
}
void loop() {
  Currentmillisx = millis();                  // Assign millis value to Currentmillisx for if statements
  if (State == Show || State == StartRace) {  //Delay time for receive serial data, if necessary...
    RecStr1();
    if (newData1 == true) {  //(Serial1.available()>0){
      //Serial.println("C'è qualcosa in seriale");
      Dateserial1 = RecCh1;  //Assign incoming data from Serial1 to Dateserial1
      Serial.println(Dateserial1);
      Timeserial1 = millis();  // Assign millis value for turning off relay
      newData1 = false;
    }
  }


  Currentmillisx = millis();
  if (State == Show || State == StartRace) {
    RecStr2();
    if (newData2 == true) {  //(Serial2.available()>0){
      //Serial.println("C'è qualcosa in seriale");
      Dateserial2 = RecCh2;  //Assign incoming data from Serial1 to Dateserial2
      Timeserial2 = millis();
      newData2 = false;
    }
  }


  Currentmillisx = millis();
  if (State == Show || State == StartRace) {
    RecStr3();
    if (newData3 == true) {  //(Serial3.available()>0){
      //Serial.println("C'è qualcosa in seriale");
      Dateserial3 = RecCh3;  //Assign incoming data from Serial3 to Dateserial3
      Timeserial3 = millis();
      newData3 = false;
    }
  }




  Currentmilliss = millis();                                                                // millis assignments for send delay control
  if ((Currentmilliss - Timesend) >= Delaysend && (State == Show || State == StartRace)) {  //If the last receive is older than Delaysend, and state is race or show
    if (Dateserial1 != "0" || Dateserial2 != "0" || Dateserial3 != "0") {                   //If some button was pressed
      Dateserial = "<";
      Dateserial.concat(State);
      Dateserial.concat(",");
      Dateserial.concat(Dateserial2);  // Compose message for send with Lora
      Dateserial.concat(",");
      Dateserial.concat(Dateserial1);
      Dateserial.concat(",");
      Dateserial.concat(Dateserial3);
      Dateserial.concat(">");
      AltSerial.print(Dateserial);  // Send fixedmessage Dateseria that contain all update of Dateserial1/2/3
      Serial.println(Dateserial);                                               // DEBUG
      Dateserial1 = "0";                                                        //Reset condition for received press button
      Dateserial2 = "0";
      Dateserial3 = "0";
    }
  }
  if (AltSerial.available() > 0) {                      //If there's something incoming from lora
    RecStrAlt();
    if (newDataAltSerial == true) {  //(Serial1.available()>0){
      State = RecChAltSerial;  //Assign incoming data from Serial3 to Dateserial3

      Serial.println("Ricevo dalla seriale AltSerial ");
      Serial.println(State);  //debug
      FlagState = 1;
      Timesend = millis();  // millis assigment for last data receive
      newDataAltSerial = false;
    }

  }

  if (FlagState == 1) {  //If state was change on previous code
    String str = "<";
    str.concat(State);
    str.concat(">");
    Serial.println("Cambio stato");
    // RecStr1();
    // if (newData1 == true) {   //(Serial1.available()>0){ //check if some press was incoming from receiver 1
    //   Dateserial1p = RecCh1;  //Assign incoming data from Serial1 to Dateserial1//Assign useless press on Dateserial1
    //   //Serial.println("Assegno provvisorio");
    //   Serial.println(RecCh1);
    //   newData1 = false;
    // }
    //    if (str.indexOf(end10lapB) == -1 || str.indexOf(end10lapV) == -1) {
    //      Serial1.print(str);  //Send to receiver 1 State
    //      //Serial.println(State);
    //    }
    // RecStr2();
    // if (newData2 == true) {  //(Serial2.available()>0){
    //   Dateserial2p = RecCh2;  //Assign incoming data from Serial1 to Dateserial1
    //   Serial.println(RecCh2);
    //   newData2 = false;
    // }
    //    if (str.indexOf(end10lapB) == -1 || str.indexOf(end10lapR) == -1) {
    //      Serial2.print(str);
    //    }
    // RecStr3();
    // if (newData3 == true) {   //(Serial3.available()>0){
    //   Dateserial3p = RecCh3;  //Assign incoming data from Serial1 to Dateserial1
    //   Serial.println(RecCh3);
    //   newData3 == false;
    // }
    //    if (str.indexOf(end10lapR) == -1 || str.indexOf(end10lapV) == -1) {
    //      Serial3.print(str);
    //    }
    Serial1.print(str);
    Serial2.print(str);
    Serial3.print(str);
    Serial.println("Mando i pachetti");
    FlagState = 0;  //Reset condition for send State to the rx
  }
}

void RecStr1() {
  static boolean recvInProgress1 = false;
  static byte ndx1 = 0;
  char startMarker1 = '<';
  char endMarker1 = '>';
  char rc1;

  while (Serial1.available() > 0 && newData1 == false) {
    //Serial.println("Lettura Serial1");
    rc1 = Serial1.read();

    if (recvInProgress1 == true) {
      if (rc1 != endMarker1) {
        RecCh1[ndx1] = rc1;
        ndx1++;
        if (ndx1 >= numChars) {
          ndx1 = numChars - 1;
        }
      } else {
        RecCh1[ndx1] = '\0';  // terminate the string
        recvInProgress1 = false;
        ndx1 = 0;
        newData1 = true;
        //Serial.println("metodo");
        //Serial.println(RecCh1);
        //Serial.println(newData1);
      }
    }

    else if (rc1 == startMarker1) {
      recvInProgress1 = true;
    }
  }
  Serial1.flush();
}

void RecStr2() {
  static boolean recvInProgress2 = false;
  static byte ndx2 = 0;
  char startMarker2 = '<';
  char endMarker2 = '>';
  char rc2;
  while (Serial2.available() > 0 && newData2 == false) {
    rc2 = Serial2.read();

    if (recvInProgress2 == true) {
      if (rc2 != endMarker2) {
        RecCh2[ndx2] = rc2;
        ndx2++;
        if (ndx2 >= numChars) {
          ndx2 = numChars - 1;
        }
      } else {
        RecCh2[ndx2] = '\0';  // terminate the string
        recvInProgress2 = false;
        ndx2 = 0;
        newData2 = true;
      }
    }

    else if (rc2 == startMarker2) {
      recvInProgress2 = true;
    }
  }
  Serial2.flush();
}

void RecStr3() {
  static boolean recvInProgress3 = false;
  static byte ndx3 = 0;
  char startMarker3 = '<';
  char endMarker3 = '>';
  char rc3;

  while (Serial3.available() > 0 && newData3 == false) {
    rc3 = Serial3.read();

    if (recvInProgress3 == true) {
      if (rc3 != endMarker3) {
        RecCh3[ndx3] = rc3;
        ndx3++;
        if (ndx3 >= numChars) {
          ndx3 = numChars - 1;
        }
      } else {
        RecCh3[ndx3] = '\0';  // terminate the string
        recvInProgress3 = false;
        ndx3 = 0;
        newData3 = true;
      }
    }

    else if (rc3 == startMarker3) {
      recvInProgress3 = true;
    }
  }
  Serial3.flush();
}

void RecStrAlt() {
  static boolean recvInProgressAltSerial = false;
  static byte ndxAltSerial = 0;
  char startMarkerAltSerial = '<';
  char endMarkerAltSerial = '>';
  char rcAltSerial;

  while (AltSerial.available() > 0 && newDataAltSerial == false) {
    rcAltSerial = AltSerial.read();

    if (recvInProgressAltSerial == true) {
      if (rcAltSerial != endMarkerAltSerial) {
        RecChAltSerial[ndxAltSerial] = rcAltSerial;
        ndxAltSerial++;
        if (ndxAltSerial >= numChars) {
          ndxAltSerial = numChars - 1;
        }
      } else {
        RecChAltSerial[ndxAltSerial] = '\0';  // terminate the string
        recvInProgressAltSerial = false;
        ndxAltSerial = 0;
        newDataAltSerial = true;
      }
    }

    else if (rcAltSerial == startMarkerAltSerial) {
      recvInProgressAltSerial = true;
    }
  }
  AltSerial.flush();
}
