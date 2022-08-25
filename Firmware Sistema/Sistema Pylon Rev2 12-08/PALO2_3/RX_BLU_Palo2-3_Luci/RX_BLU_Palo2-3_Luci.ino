#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial SwSerial;
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
String TxData = "0";
String TxDatastr = "0";
unsigned long Timesend = 0;
unsigned long CurrentMillis = 0;
unsigned long FirstStateSend = 0;
unsigned long Plotmillis = 0;
unsigned long millisc = 0;
const byte numChars = 5;
char RecCh[numChars];
boolean newData = false;
int DoubleState = 0;
int FlagState = 0;
int FlagPalo = 0;
int PinLight2 = 6;
int PinLight3 = 7;
int ntagliP2 = 0;
int ntagliP3 = 0;
int lampeggianteP2 = 0;
int lampeggianteP3 = 0;
int onP2 = 0;
int onP3 = 0;
unsigned long lampmillisP2;
unsigned long lampmillisP3;

//Inizio Configurazioni Ricevente
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Startup = "1000";
String Stop = "6000";
String end10lapB = "5534";
String PressShow2 = "2230";
String PressShow3 = "2330";
String PressCut2 = "3231";
String PressCut3 = "3331";
String DoubleCut = "4035";
String StopTime = "6534";
String State = Startup;
unsigned long Delaysend = 200;
unsigned long DelayState = 350;
byte Chan = 55;
//Fine Configurazioni Ricevente

void setup() {
  Serial.begin(9600);
  delay(500);
  SwSerial.begin(9600);
  delay(500);
  e22ttl.begin();
  Serial.println("prova");
  pinMode(PinLight2, OUTPUT);
  pinMode(PinLight3, OUTPUT);
}

void loop() {
  RecStr();
  if (newData == true) { // If teensy will sent state to sem_rx
    State = RecCh;
    newData = false;
    FlagState = 1;
  }
  if (e22ttl.available() > 1) { // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    //If there isn't any problem we're going to receive press
    TxData = rc.data; //Assign incoming data on TxData variable
    Timesend = millis();
    if (TxData != "0" && (State == Show || State == StartRace)) { //Condition for sent to mega cut
      RecStr();//valutare se disabilitare
      if (newData == true) { //(SwSerial.available()>0){  // If mega will sent state to sem_rx
        Serial.println("C'è qualcosa in seriale - riassegno uno stato aggiornato");
        State = RecCh;
        Serial.println(State);
        newData = false;
        FlagState = 1;
      }
      if (State == Show || State == StartRace) { //if nothing has change from previous if-statements
        String str = "<";
        str.concat(TxData);
        str.concat(">");
        SwSerial.print(str); //Send Txdata from rx to Mega
        Plotmillis = millis();
        Serial.print(Plotmillis);
        Serial.println(" ");
        Serial.println(str);
      }
      TxDatastr = TxData;
      TxData = "0"; //Reset condition for set data
    }
  }
  CurrentMillis = millis();

  if ((FlagState == 1) && ((CurrentMillis - Timesend) > Delaysend)) { //If statements who control that state was change and
    if (State.indexOf("850") == -1) {
      ResponseStatus rc = e22ttl.sendBroadcastFixedMessage(Chan, State);
      Serial.println("Mando Stato aggiornato");
      Serial.println(State);
      FirstStateSend = millis();
      DoubleState = 1;
    }

    if (State.indexOf(DoubleCut) == -1  ) { //se NON ricevo lo stato di doppio taglio
      if (State.indexOf(StopTime) == -1 ) { //se NON ricevo lo stato di StopTIME
        digitalWrite( PinLight2, LOW);
        digitalWrite( PinLight3, LOW);
        ntagliP2 = 0;
        ntagliP3 = 0;
        lampeggianteP2 = 0;
        lampeggianteP3 = 0;
        Serial.println("PINS LOW");
      }
    }
    TxDatastr = "";
    FlagState = 0;

  }


  CurrentMillis = millis();
  if (DoubleState == 1 && (CurrentMillis - FirstStateSend) > DelayState) {
    ResponseStatus rc = e22ttl.sendBroadcastFixedMessage(Chan, State);
    DoubleState = 0;
  }
  if (State == Show) {
    if (TxDatastr == PressShow2) {
      digitalWrite( PinLight2, HIGH);
    }
    if (TxDatastr == PressShow3) {
      digitalWrite( PinLight3, HIGH);
    }
  }
  if (State == StartRace) {
    if (TxDatastr == PressCut2) {
      ntagliP2 = ntagliP2 + 1;
      if (ntagliP2 == 1) {
        digitalWrite( PinLight2, HIGH);
      }
      if (ntagliP2 > 1) {
        lampeggianteP2 = 1;
        Serial.println("lampeggiante P2");
      }
      TxDatastr = "";
    }
    if (TxDatastr == PressCut3) {
      ntagliP3 = ntagliP3 + 1;
      if (ntagliP3 == 1) {
        digitalWrite( PinLight3, HIGH);
      }
      if (ntagliP3 > 1) {
        lampeggianteP3 = 1;
        Serial.println("lampeggiante P3");

      }
      TxDatastr = "";
    }
  }

  // codice lampeggio

  if (lampeggianteP2 == 1) {
    if (onP2 == 0 && millis()  - lampmillisP2 >= 500) {
      lampmillisP2 = millis();
      onP2 = 1;
      digitalWrite( PinLight2, HIGH);
      Serial.println(" P2 HIGH ");

    }
    if (onP2 == 1 && millis()  - lampmillisP2 >= 500 ) {
      digitalWrite( PinLight2, LOW);
      onP2 = 0;
      lampmillisP2 = millis();
      Serial.println(" P2 LOW ");
    }
  }


  if (lampeggianteP3 == 1) {
    if (onP3 == 0 && millis() - lampmillisP3 >= 500) {
      lampmillisP3 = millis();
      onP3 = 1;
      digitalWrite( PinLight3, HIGH);

    }
    if (onP3 == 1 && millis() - lampmillisP3 >= 500 ) {
      digitalWrite( PinLight3, LOW);
      onP3 = 0;
      lampmillisP3 = millis();
      Serial.println(" P3 LOW ");
    }
  }
  // fine codice lampeggio
}

void RecStr() { // Programma Funzionante
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
