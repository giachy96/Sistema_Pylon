#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>

AltSoftSerial SwSerial;
 
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
String Race="<300>";
String Show="<200>";
String Startup="<100>";
String State=Startup; //Startup state
String OldState=State;
String TxData="0";
unsigned long Timesend=0;
unsigned long Currentmillis=0;
unsigned long Delaysend=200;
const byte numChars=32;
char RecCh[numChars];
boolean newData = false;

void setup() {
   //pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
   //pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500); 
  SwSerial.begin(9600);
  delay(500);
  e22ttl.begin(); 
  Serial.println("prova");
}
void loop() { 

RecStr(RecCh);
  
   if (SwSerial.available()>0){  // If mega will sent state to sem_rx
      Serial.println("C'è qualcosa in seriale");
      State=SwSerial.readString(); //Assign system status to state
      RecStr();
      String State(RecCh); 
      Serial.print(State);
      }
  if (e22ttl.available()>1) {  // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    Serial.println("ricevo dal lora qualcosa , assegno timesend");
    if (rc.status.code!=1){ // If there is some problem
      rc.status.getResponseDescription(); //Get report
      }
    else{ //If there isn't any problem we're going to receive press 
      TxData=rc.data; //Assign incoming data on TxData variable
      Serial.println("Ricevo dal lora");
      Serial.println(TxData);
    }
  if (TxData !="0" && (State==Show || State==Race || State==Startup)){ //Condition for sent to mega press
      if (SwSerial.available()>0){  // If mega will sent state to sem_rx
      Serial.println("C'è qualcosa in seriale");
      RecStr();
      String State(RecCh); 
      Serial.println(State);
      }
      if (State==Show || State==Race){//if nothing has change from previous if-statements
      SwSerial.print(TxData); //Send Txdata from rx to Mega
      Serial.println("Scrivo in seriale");
      Serial.println(TxData);
      }
    TxData="0"; //Reset condition for set data
  }
 }
if ((OldState != State)) { //If statements who control that state was change and
  ResponseStatus rc = e22ttl.sendFixedMessage(0,1,3,State);
  Serial.println("Mando Stato nuovo");
  Serial.println(State);
  Serial.println("plotto vecchio Stato");
  Serial.println(OldState);
  OldState=State;
}
}
 
void RecStr( char receivedChars[numChars] ) {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (SwSerial.available() > 0 && newData == false) {
        rc = SwSerial.read();

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
    SwSerial.flush();
}

void showNewData(char receivedChars[numChars]) {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}
