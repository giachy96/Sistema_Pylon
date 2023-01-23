#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial SwSerial;
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX

//Inizio Configurazione Ricevente
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Startup = "1000";
String Stop = "6000";
String State = Startup; //Startup state
unsigned long DelayLight = 1500;
String PressShow = "2110";
String PressRace = "3112";
String end10lap = "5514";
String DoubleCut = "4015";
String StopTime = "6514";
int Key = 0;
int Add = 1;
int Chan = 15;
//Fine Configurazione Ricevente

int Light = 12;
String TxData = "0";

unsigned long timeonlight;
int onlight;
int on;
const byte numChars = 32;
char RecCh[numChars];
boolean newData = false;
boolean newDataLora = false;




void setup() {
  //pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
  //pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500);
  SwSerial.begin(9600);
  delay(500);
  e22ttl.begin();
  pinMode(Light, OUTPUT); //Output pin to Relay Red
  //Serial.println("prova");
}
void loop() {
  RecStr();
  if (newData == true) { //(SwSerial.available()>0){  // If mega will sent state to sem_rx
    State = RecCh;
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, State);
    if (State == Show || State == Race || State == Stop ) {
      onlight = 0;
      digitalWrite(Light, LOW);
    }
    newData = false;
    Serial.println(State);
  }


  if (newDataLora == true) { //(SwSerial.available()>0){  // If mega will sent state to sem_rx

    if (TxData != PressRace) {
      String str = "<";
      str.concat(TxData);
      str.concat(">");
      SwSerial.println(str);
    }

    if (State == Show && TxData == PressShow ) {
      digitalWrite(Light, HIGH);
    }
    if (State == StartRace && TxData == PressRace ) {
      onlight = 1;
    }


    newDataLora = false;
  }

  if (e22ttl.available() > 1) {
    ResponseContainer rc = e22ttl.receiveMessage();
    TxData = rc.data;
    Serial.println(TxData);
    newDataLora = true;
  }


  //DIOBESTIA TUTTE STE SEGHE PER UNA LUCE
  if (onlight == 1) {
    if (on == 0 ) {
      digitalWrite(Light, HIGH);
      timeonlight = millis();
      on = 1;
    } else {
      if ((millis() - timeonlight) >= DelayLight &&  on == 1) {
        digitalWrite(Light, LOW);
        onlight = 0;
        on = 0;
      }
    }
  }
  //FINE SEGHE PER UNA LUCE

}




void RecStr() {
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
