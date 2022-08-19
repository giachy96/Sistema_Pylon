#include "Arduino.h"
#include <SoftwareSerial.h>
#include "EBYTE.h"
#include <AltSoftSerial.h>

#define PIN_RX 2
#define PIN_TX 3
#define PIN_M0 4
#define PIN_M1 5
#define PIN_AX 6

AltSoftSerial SwSerial;
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
int PinLightR = 4;
int PinLightG = 5;
int PinLightB = 6;
int ntagliR = 0;
int ntagliG = 0;
int ntagliB = 0;
int lampeggianteR = 0;
int lampeggianteG = 0;
int lampeggianteB = 0;
int onR = 0;
int onG = 0;
int onB = 0;
unsigned long lampmillisR;
unsigned long lampmillisG;
unsigned long lampmillisB;

//Inizio Configurazioni Ricevente
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Startup = "1000";
String Stop = "6000";
String PressShowR = "2110";
String PressRaceR = "3112";
String PressCutR = "3111";
String end10lapR = "5514";
String PressShowG = "2120";
String PressRaceG = "3122";
String PressCutG = "3121";
String end10lapG = "5524";
String PressShowB = "2130";
String PressRaceB = "3132";
String PressCutB = "3131";
String end10lapB = "5534";
String State = Startup;
unsigned long Delaysend = 200;
unsigned long DelayState = 350;
byte Key = 0;
byte Add = 1;
byte Chan = 10;
//Fine Configurazioni Ricevente

void setup() {
  Serial.begin(9600);
  delay(500);
  SwSerial.begin(9600);
  delay(500);
  ESerial.begin(9600);
  Transceiver.init();
  Serial.println("prova");
  pinMode(PinLightR, OUTPUT);
  pinMode(PinLightG, OUTPUT);
  pinMode(PinLightB, OUTPUT);
}

void loop() {
  RecStr();
  if (newData == true) { // If teensy will sent state to sem_rx
    State = RecCh;
    newData = false;
    FlagState = 1;
  }
  if (ESerial.available()) { // If there is something arrived from lora (Transmitter)
    Transceiver.GetStruct(&MyData, sizeof(MyData));
    TxData = MyData.string; //Assign incoming data on TxData variable
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
        SwSerial.print(TxData); //Send Txdata from rx to Mega
        Plotmillis = millis();
        Serial.print(Plotmillis);
        Serial.println(" ");
        Serial.println(TxData);
      }
      TxDatastr = TxData;
      TxData = "0"; //Reset condition for set data
    }
  }
  CurrentMillis = millis();

  if ((FlagState == 1) && ((CurrentMillis - Timesend) > Delaysend)) { //If statements who control that state was change and
    State.toCharArray(MyData.string, 32);
    Transceiver.SendStruct(&MyData, sizeof(MyData));
    Serial.println("Mando Stato aggiornato");
    Serial.println(State);
    FirstStateSend = millis();
    DoubleState = 1;
    digitalWrite( PinLightR, LOW);
    digitalWrite( PinLightG, LOW);
    digitalWrite( PinLightB, LOW);
    ntagliR = 0;
    ntagliG = 0;
    ntagliB = 0;
    lampeggianteR = 0;
    lampeggianteG = 0;
    lampeggianteB = 0;
    TxDatastr = "";
    FlagState = 0;
    Serial.println("PINS LOW");
  }


  CurrentMillis = millis();
  //  if (DoubleState == 1 && (CurrentMillis - FirstStateSend) > DelayState) {
  //    ResponseStatus rc = e22ttl.sendFixedMessage(Key, Add , Chan, State);
  //    DoubleState = 0;
  //  }
  if (State == Show) {
    if (TxDatastr == PressShowR) {
      digitalWrite( PinLightR, HIGH);
    }
    if (TxDatastr == PressShowG) {
      digitalWrite( PinLightG, HIGH);
    }
    if (TxDatastr == PressShowB) {
      digitalWrite( PinLightB, HIGH);
    }
  }
  if (State == StartRace) {
    if (TxDatastr == PressCutR) {
      ntagliR = ntagliR + 1;
      if (ntagliR == 1) {
        digitalWrite( PinLightR, HIGH);
      }
      if (ntagliR > 1) {
        lampeggianteR = 1;
        Serial.println("lampeggiante R");
      }
      TxDatastr = "";
    }
    if (TxDatastr == PressCutG) {
      ntagliG = ntagliG + 1;
      if (ntagliG == 1) {
        digitalWrite( PinLightG, HIGH);
      }
      if (ntagliG > 1) {
        lampeggianteG = 1;
        Serial.println("lampeggiante G");

      }
      TxDatastr = "";
    }
    if (TxDatastr == PressCutB) {
      ntagliB = ntagliB + 1;
      if (ntagliB == 1) {
        digitalWrite( PinLightB, HIGH);
      }
      if (ntagliB > 1) {
        lampeggianteB = 1;
        Serial.println("lampeggiante B");

      }
      TxDatastr = "";
    }
  }

  // codice lampeggio

  if (lampeggianteR == 1) {
    if (onR == 0 && millis()  - lampmillisR >= 500) {
      lampmillisR = millis();
      onR = 1;
      digitalWrite( PinLightR, HIGH);
      Serial.println(" R HIGH ");

    }
    if (onR == 1 && millis()  - lampmillisR >= 500 ) {
      digitalWrite( PinLightR, LOW);
      onR = 0;
      lampmillisR = millis();
      Serial.println(" R LOW ");
    }
  }


  if (lampeggianteG == 1) {
    if (onG == 0 && millis() - lampmillisG >= 500) {
      lampmillisG = millis();
      onG = 1;
      digitalWrite( PinLightG, HIGH);

    }
    if (onG == 1 && millis() - lampmillisG >= 500 ) {
      digitalWrite( PinLightG, LOW);
      onG = 0;
      lampmillisG = millis();
      Serial.println(" G LOW ");
    }
  }

  if (lampeggianteB == 1) {
    if (onB == 0 && millis() - lampmillisB >= 500) {
      lampmillisB = millis();
      onB = 1;
      digitalWrite( PinLightB, HIGH);

    }
    if (onB == 0 && millis() - lampmillisB >= 500 ) {
      digitalWrite( PinLightB, LOW);
      onB = 0;
      lampmillisB = millis();
      Serial.println(" B LOW ");
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
