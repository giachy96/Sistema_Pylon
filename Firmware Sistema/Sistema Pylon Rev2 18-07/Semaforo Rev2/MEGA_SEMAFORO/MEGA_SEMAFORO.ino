  //#include "LoRa_E22.h"
  //#include "Arduino.h"
 // #include "SoftwareSerial.h"
  LoRa_E22 e22ttl(10, 11); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
  
  unsigned long Timeserial1=0;
  unsigned long Timeserial2=0;
  unsigned long Timeserial3=0;
  unsigned long Timesend=0;
  unsigned long Currentmilliss=0;
  unsigned long Currentmillisx=0;
  unsigned long DelayLight=2000;
  unsigned long Delayserialx=0;
  unsigned long Delaysend=200;
  String Dateserial1="0";
  String Dateserial2="0";
  String Dateserial3="0";
  String Dateserial1p="0";
  String Dateserial2p="0";
  String Dateserial3p="0";
  String Dateserial;
  int RelayRed=4;
  int RelayGreen=5;
  int RelayBlue=6;
  String State="000"; //Startup state

void setup() {
  Serial1.begin(9600);
  delay(300);
  Serial2.begin(9600);
  delay(300);
  Serial3.begin(9600);
  delay(300);
  Serial.begin(9600);
  delay(300);
  
  e22ttl.begin();
  pinMode(RelayRed, OUTPUT);
  pinMode(RelayBlue, OUTPUT);
  pinMode(RelayGreen, OUTPUT);
}

void loop() {

Currentmillisx=millis(); 
 if ((Currentmillisx-Timeserial1) > Delayserialx && (State=="200" || State=="100")) { //Delay time for receive serial data, if necessary...
  if (Serial1.available()){
    Dateserial1=Serial1.read();  //Assign incoming data from Serial1 to Dateserial1
    digitalWrite(RelayRed,HIGH); //Turn on stoplight
    Timeserial1=millis();
    }
  }
 if ((Currentmillisx-Timeserial1) > DelayLight) { // if red light is on for more than Delaylight
   digitalWrite(RelayRed,LOW); //Turn off stoplight
   }

 Currentmillisx=millis();
 if ((Currentmillisx-Timeserial2) > Delayserialx && (State=="200" || State=="100")) {
  if (Serial2.available()){
    Dateserial2=Serial2.read();
    digitalWrite(RelayGreen,HIGH);
   Timeserial2=millis();
    }
  }
  if ((Currentmillisx-Timeserial2) > DelayLight) {
    digitalWrite(RelayGreen,LOW);
    }

 Currentmillisx=millis();
 if ((Currentmillisx-Timeserial3) > Delayserialx && (State=="200" || State=="100")) {
  if (Serial3.available()){
    Dateserial3=Serial3.read();
    digitalWrite(RelayBlue,HIGH);
    Timeserial3=millis();
    }
  }
  if ((Currentmillisx-Timeserial3) > DelayLight) {
    digitalWrite(RelayBlue,LOW);
    }

 Currentmilliss=millis(); // millis assignments for send delay control
  if ((Currentmilliss-Timesend) >= Delaysend && (State=="200" || State=="100")) {  //If the last trasmission is older than Delaysend, and state is race
   if (Dateserial1 != "0" || Dateserial2 != "0" || Dateserial3 !="0")  { //If some button was pressed
    Dateserial=(Dateserial1); // Compose message for send with Lora
    Dateserial.concat(",");
    Dateserial.concat(Dateserial2);
    Dateserial.concat(",");
    Dateserial.concat(Dateserial3);
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 1, 7, Dateserial); // Send fixedmessage Dateseria that contain all update of Dateserial1/2/3
    Timesend=millis();
    Dateserial1="0"; //Reset condition for received press button
    Dateserial2="0";
    Dateserial3="0";
    }
    }
  
  if (e22ttl.available()>1) {
  ResponseContainer rc = e22ttl.receiveMessage();
    // Is something goes wrong print error
    if (rc.status.code!=1){
        rc.status.getResponseDescription();
    }
    else{ 
     State=rc.data;    
   }
    }
}