#include "LoRa_E22.h"
LoRa_E22 e22ttl(10, 11);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX


const byte numChars = 32;
char  receivedChars1[numChars];
char  receivedChars2[numChars];
char  receivedChars3[numChars];
int FlagState = 0;

//Inizio Configurazione Mega
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Stop = "6000";
String Startup = "1000";
String end10lapB = "5534";
String end10lapV = "5524";
String end10lapR = "5514";
String DoubleCutR = "4015";
String DoubleCutV = "4025";
String DoubleCutB = "4035";
String StopTimeR = "6514";
String StopTimeV = "6524";
String StopTimeB = "6534";
String State;

int Key = 0;
int Add = 0;
int Chan = 10;
//Fine Configurazione Mega

boolean newData1 = false;
boolean newData2 = false;
boolean newData3 = false;
boolean  newDatalora = false;

void setup() {
  Serial1.begin(9600);  // Serial Verde
  delay(300);
  Serial2.begin(9600);  // Serial Rosso
  delay(300);
  Serial3.begin(9600);  // Serial Blu
  delay(300);
  Serial.begin(9600);  // Serial Pc interface
  delay(300);
  e22ttl.begin();  //Start e22ttl
  // Serial.println("prova");
}
void loop() {
  //  Currentmillis = millis();
  recvSerial1();
  recvSerial2();
  recvSerial3();
  if (newData1 == true) {  // ricevo qualcosa dalla seriale VERDE
    String Rxs1;
    Rxs1 = receivedChars1;
    Serial.println( Rxs1);
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs1);
    newData1 = false;

  }

  if (newData2 == true) { // ricevo qualcosa dalla seriale ROSSA
    String Rxs2;
    Rxs2 = receivedChars2;
    Serial.println( Rxs2);
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs2);
    newData2 = false;

  }
  if (newData3 == true) {   // ricevo qualcosa dalla seriale BLU
    String Rxs3;
    Rxs3 = receivedChars3;
    Serial.println( Rxs3);
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs3);
    newData3 = false;

  }

  if (e22ttl.available() > 1) {                      //If there's something incoming from lora
    ResponseContainer rc = e22ttl.receiveMessage();  //Receive message
    State = rc.data;
    newDatalora = true;

  }

  if (newDatalora == true) {
    Serial.println( State);
    if (State != end10lapB && State != end10lapR && State != end10lapV && State != DoubleCutR  && State != DoubleCutB && State != DoubleCutV && State != StopTimeR && State != StopTimeV && State != StopTimeB  ) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println( str);// verde
      Serial2.println( str);// rosso
      Serial3.println( str);// blu
    }
    if (State == end10lapR || State == DoubleCutR ||  State == StopTimeR  ) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial2.println( str);// rosso
    }
    if (State == end10lapV || State == DoubleCutV ||  State == StopTimeV  ) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println( str);// verde
    }
    if (State == end10lapB || State == DoubleCutB ||  State == StopTimeB  ) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial3.println( str);// blu
    }
    newDatalora = false;
  }

}

void  recvSerial1() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial1.available() > 0 && newData1 == false) {
    //while (port.available() > 0 ) {
    rc = Serial1.read();


    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars1[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars1[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData1 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}


void  recvSerial2() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial2.available() > 0 && newData2 == false) {
    //while (port.available() > 0 ) {
    rc = Serial2.read();


    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars2[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars2[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData2 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void  recvSerial3() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial3.available() > 0 && newData3 == false) {
    //while (port.available() > 0 ) {
    rc = Serial3.read();


    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars3[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars3[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData3 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}
