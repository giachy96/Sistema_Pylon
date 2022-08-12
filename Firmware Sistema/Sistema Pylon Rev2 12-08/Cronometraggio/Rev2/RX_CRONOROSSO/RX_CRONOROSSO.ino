#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial AltSerial;

LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
//#define SOFTRX 11 // MT - define rx pin 10
//#define SOFTTX 10 // MT - define tx pin 11
//SoftwareSerial AltSerial(SOFTRX,SOFTTX);
String Race = "<300>";
String Show = "<200>";
String Startup = "<100>";
String State = Show; //Startup state
String TxData = "0";
const byte numChars = 32;
char RecCh[numChars];
boolean newData = false;


void setup() {
  //pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
  //pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500);
  AltSerial.begin(9600);
  delay(500);
  e22ttl.begin();

}
void loop() {
  
  recvWithStartEndMarkers( RecCh );
  State = RecCh;

  if(newData == true){
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 1, 6, RecCh);
    newData = false;
    }
  if (e22ttl.available() > 1) { // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    TxData = rc.data; //Assign incoming data on TxData variable
    Serial.println("Ricevo dal lora");
    Serial.println(TxData);

    if (TxData != "0" && (State == "200"|| State == "300"|| State == "329" )) { //Condition for sent to mega press
      if (AltSerial.available() > 0) { // If mega will sent state to sem_rx
        recvWithStartEndMarkers( RecCh );
        State = RecCh;
      }
      if (State == "200" || State == "300" || State == "329") { //if nothing has change from previous if-statements
        
        String pack  = "<";
        pack.concat(TxData);
        pack.concat(">");
        AltSerial.print(pack); //Send Txdata from rx to Mega
        Serial.println("Mando via il dato");
        Serial.print(pack);
      }
      TxData = "0"; //Reset condition for set data
    }
  }


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
