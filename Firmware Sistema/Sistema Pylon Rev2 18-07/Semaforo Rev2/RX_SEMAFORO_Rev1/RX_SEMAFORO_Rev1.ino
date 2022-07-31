#include "Arduino.h"
#include "LoRa_E22.h"
#include <SoftwareSerial.h> //MT - include softwareserial library
 
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
#define SOFTRX 11 // MT - define rx pin 10
#define SOFTTX 10 // MT - define tx pin 11
SoftwareSerial SwSerial(SOFTRX,SOFTTX);
String State="200";
String OldState="200";
String TxData="0";
unsigned long Timesend=0;
unsigned long Currentmillis=0;
unsigned long Delaysend=200;

void setup() {
   pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
   pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500); 
  SwSerial.begin(9600);
  delay(500);
  
  e22ttl.begin(); 
   Serial.println("prova");
}
void loop() { 
   if (SwSerial.available()){  // If mega will sent state to sem_rx
      Serial.println("C'è qualcosa in seriale");
      State=SwSerial.readString(); //Assign system status to state
      Serial.println(State);
      }
  if (e22ttl.available()>1) {  // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    if (rc.status.code!=1){ // If there is some problem
      rc.status.getResponseDescription(); //Get report
      Timesend=millis();
    }
    else{ //If there isn't any problem we're going to receive press 
      TxData=rc.data; //Assign incoming data on TxData variable
      Serial.println("Ricevo dal lora");
      Serial.println(TxData);
    }
  if (TxData !="0" && (State=="200" || State=="300")){ //Condition for sent to mega press
      if (SwSerial.available()){  // If mega will sent state to sem_rx
      Serial.println("C'è qualcosa in seriale");
      State=SwSerial.readString(); //Assign system status to state
      }
      if (State=="200" || State=="300"){//if nothing has change from previous if-statements
      SwSerial.println(TxData); //Send Txdata from rx to Mega
      Serial.println("Scrivo in seriale");
      Serial.println(TxData);
      }
    TxData="0"; //Reset condition for set data
  }
 }
 Currentmillis=millis();
if ((Currentmillis-Timesend)>Delaysend && (OldState != State)) { //If statements who control that state was change and
  ResponseStatus rc = e22ttl.sendFixedMessage(0,1,3,State);
  OldState=State;
}
}
 
