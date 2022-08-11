#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial SwSerial;
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
String TxData="0";
unsigned long Timesend=0;
unsigned long CurrentMillis=0;
unsigned long StopSend=0;
unsigned long Plotmillis=0;
unsigned long millisc=0;
const byte numChars=5;
char RecCh[numChars];
boolean newData = false;
int DoubleStop=0;
int FlagState=0;

//Inizio Configurazioni Ricevente
String Race="300";
String Show="200";
String Startup="100";
String Stop="600";
String State=Startup;
unsigned long Delaysend=200;
unsigned long DelayStop=350;
int Chan=10;
//Fine Configurazioni Ricevente

void setup() {
  Serial.begin(9600);
  delay(500); 
  SwSerial.begin(9600);
  delay(500);
  e22ttl.begin(); 
  Serial.println("prova");
}

void loop() { 
  RecStr();
  if (newData==true){ // If teensy will sent state to sem_rx
    State=RecCh; 
    newData=false;
    FlagState=1;
    }
  if (e22ttl.available()>1) {  // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    if (rc.status.code!=1){ // If there is some problem
      rc.status.getResponseDescription(); //Get report
      }
    else{ //If there isn't any problem we're going to receive press 
      TxData=rc.data; //Assign incoming data on TxData variable
      Timesend=millis();
      }
  if (TxData !="0" && (State==Show || State==Race)){ //Condition for sent to mega cut
      RecStr();//valutare se disabilitare
      if (newData==true){//(SwSerial.available()>0){  // If mega will sent state to sem_rx
      Serial.println("C'è qualcosa in seriale - riassegno uno stato aggiornato");
      State=RecCh; 
      Serial.println(State);
      newData=false;
      FlagState=1;
      }
      if (State==Show || State==Race){//if nothing has change from previous if-statements
      SwSerial.print(TxData); //Send Txdata from rx to Mega
      Plotmillis=millis();
      Serial.print(Plotmillis);
      Serial.println(" ");
      Serial.println(TxData);
      }
    TxData="0"; //Reset condition for set data
  }
 }
 CurrentMillis=millis();
if ((FlagState==1) && ((CurrentMillis-Timesend)>Delaysend)) { //If statements who control that state was change and
  ResponseStatus rc = e22ttl.sendBroadcastFixedMessage(Chan,State);
  Serial.println("Mando Stato aggiornato");
  Serial.println(State);
  if (State==Stop){
  StopSend=millis();
  DoubleStop=1;
  }
  FlagState=0;
}
CurrentMillis=millis();
if (DoubleStop==1 && (CurrentMillis-StopSend)>DelayStop && State==Stop){
  ResponseStatus rc = e22ttl.sendBroadcastFixedMessage(Chan,State);
  
  DoubleStop=0;
}
if (State !=Stop){
  DoubleStop=0;
}
}

void RecStr() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                RecCh[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                RecCh[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
    Serial.flush();
}
 // void loop() { 
//   RecStr();
//   if (newData==true){ // If teensy will sent state to sem_rx
//     State=RecCh; 
//     newData=false;
//     }
//   if (e22ttl.available()>1) {  // If there is something arrived from lora (Transmitter)
//     ResponseContainer rc = e22ttl.receiveMessage();// Receive message
//     if (rc.status.code!=1){ // If there is some problem
//       rc.status.getResponseDescription(); //Get report
//       }
//     else{ //If there isn't any problem we're going to receive press 
//       TxData=rc.data; //Assign incoming data on TxData variable
//       Timesend=millis();
//       }
//   if (TxData !="0" && (State==Show || State==Race)){ //Condition for sent to mega cut
//       RecStr();//valutare se disabilitare
//       if (newData==true){//(SwSerial.available()>0){  // If mega will sent state to sem_rx
//       Serial.println("C'è qualcosa in seriale - riassegno un nuovo stato ");
//       State=RecCh; 
//       Serial.println(State);
//       newData=false;
//       }
//       if (State==Show || State==Race){//if nothing has change from previous if-statements
//       SwSerial.print(TxData); //Send Txdata from rx to Mega
//       Plotmillis=millis();
//       Serial.print(Plotmillis);
//       Serial.println(" ");
//       Serial.println(TxData);
      
//       }
//     TxData="0"; //Reset condition for set data
//   }
//  }
//  CurrentMillis=millis();
// if ((State != OldState) && ((CurrentMillis-Timesend)>Delaysend)) { //If statements who control that state was change and
//   ResponseStatus rc = e22ttl.sendBroadcastFixedMessage(9,State);
//   Serial.println("Mando Stato nuovo");
//   Serial.println(State);
//   Serial.println("plotto vecchio Stato");
//   Serial.println(OldState);
//   if (State==Stop){
//   StopSend=millis();
//   DoubleStop=1;
//   }
//   OldState=State;
// }
// CurrentMillis=millis();
// if (DoubleStop==1 && (CurrentMillis-StopSend)>DelayStop && State==Stop){
//   ResponseStatus rc = e22ttl.sendBroadcastFixedMessage(9,State);
  
//   DoubleStop=0;
// }
// else if (State !=Stop){
//   DoubleStop=0;
// }
// }
// void RecStr() { // Programma Funzionante
//     static boolean recvInProgress = false;
//     static byte ndx = 0;
//     char startMarker = '<';
//     char endMarker = '>';
//     char rc;
 
//     while (SwSerial.available() > 0 && newData == false) {
//         rc = SwSerial.read();

//         if (recvInProgress == true) {
//             if (rc != endMarker) {
//                 RecCh[ndx] = rc;
//                 ndx++;
//                 if (ndx >= numChars) {
//                     ndx = numChars - 1;
//                 }
//             }
//             else {
//                 RecCh[ndx] = '\0'; // terminate the string
//                 recvInProgress = false;
//                 ndx = 0;
//                 newData = true;
//             }
//         }

//         else if (rc == startMarker) {
//             recvInProgress = true;
//         }
//     }
//     SwSerial.flush();
// }

