#include "Arduino.h"
#include "LoRa_E22.h"
#include <SoftwareSerial.h> //MT - include softwareserial library
 
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
#define SOFTRX 10 // MT - define rx pin 10
#define SOFTTX 11 // MT - define tx pin 11
SoftwareSerial SwSerial(SOFTRX,SOFTTX);
String State="000";
String TxData="0";

void setup() {
   pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
   pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500); 
  SwSerial.begin(9600);
  delay(500);
 
  // Startup all pins and UART
  e22ttl.begin();
 
 
}
 
void loop() { 
  if (e22ttl.available()>1) {  // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    if (rc.status.code!=1){ // If there is some problem
      rc.status.getResponseDescription(); //Get report
    }
    else{ //If there isn't any problem we're going to receive press 
      TxData=rc.data; //Assign incoming data on TxData variable
    }
  if (TxData !="0" && (State=="200" && State=="300")){ //Condition for sent to mega press
    if (SwSerial.available()){
      State=SwSerial.read();
      }
      if (State=="200" && State=="300"){ //If nothing has change in the previous condition
    SwSerial.println(TxData); //Send Txdata from rx to Mega
    }
    TxData="0"; //Reset condition for set data
  }
 }
 }
