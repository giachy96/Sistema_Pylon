#include "Arduino.h"
#include <SoftwareSerial.h>
#include "EBYTE.h"
#include <AltSoftSerial.h>

#define PIN_RX 2
#define PIN_TX 3
#define PIN_M0 4
#define PIN_M1 5
#define PIN_AX 6

AltSoftSerial  AltSerial;
SoftwareSerial ESerial(PIN_RX, PIN_TX);
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);


struct DATA {
  char string[32];
  int Bits;
  float Volts;
  float Amps;

};

DATA MyData;
unsigned long Last;


//Inizio configurazione
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Startup = "1000";
String State = Show; //Startup state
String OldState;

String end10lapB = "5534";
//Fine Configurazione

String TxData = "0";
String TxDatastr;
const byte numChars = 32;
char RecCh[numChars];
boolean newData = false;



void setup() {


  Serial.begin(9600);
  delay(500);
  ESerial.begin(9600);
  Transceiver.init();
  delay(500);
  AltSerial.begin(9600);



}
void loop() {

  recvWithStartEndMarkers( RecCh );
  State = RecCh;

  if (newData == true) {
    TxDatastr = "";
    memcpy(MyData.string, RecCh, 32);
    Transceiver.SendStruct(&MyData, sizeof(MyData));
    newData = false;

    Serial.println("Ricevo dalla seriale e mando immediataente");
  }
  if (ESerial.available()) { // If there is something arrived from lora (Transmitter)
    Transceiver.GetStruct(&MyData, sizeof(MyData));
    TxData = MyData.string; //Assign incoming data on TxData variable
    Serial.println("Ricevo dal lora");
    Serial.println(TxData);
    String pack  = "<";
    pack.concat(TxData);
    pack.concat(">");
    AltSerial.print(pack); //Send Txdata from rx to Mega
    Serial.println("Mando via il dato");
    Serial.println(pack);

    if (TxData != "0" && (State == Show  || State == StartRace  )) { //Condition for sent to mega press
      if (AltSerial.available() > 0) { // If mega will sent state to sem_rx
        recvWithStartEndMarkers( RecCh );
        State = RecCh;

      }
      if (true) { //(State == Show  || State == StartRace ) { //if nothing has change from previous if-statements
        String pack  = "<";
        pack.concat(TxData);
        pack.concat(">");
        AltSerial.print(pack); //Send Txdata from rx to Mega
        Serial.println("Mando via il dato");
        Serial.println(pack);
      }
      TxDatastr = TxData;
      TxData = "0"; //Reset condition for set data
    }
  }

}





int decodecomma (String str , String tempi[]) {
  int lungh_str = str.length();
  char buff[lungh_str + 1];
  str.toCharArray(buff, lungh_str + 1);
  //Serial.println(parseData(buff));
  int i = 0;
  char* p;
  //Serial.println("Example of splitting a string into tokens: ");
  // Serial.print("The input string is: '");
  // Serial.print(buff);
  // Serial.println("'");

  p = strtok(buff, "{,}"); //2nd argument is a char[] of delimiters
  while (p != '\0') { //not equal to NULL
    //Serial.println(p);
    tempi[i] = p;

    p = strtok('\0', "{,}");  //expects NULL for string on subsequent calls
    i++;
  }

  return tempi;
}



void  recvWithStartEndMarkers( char receivedChars[numChars] ) {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (AltSerial.available() > 0 && newData == false) {
    rc = AltSerial.read();

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
  AltSerial.flush();

}

void showNewData(char receivedChars[numChars]) {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}
