#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial SwSerial;
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX

//Inizio configurazione
String Avanti = "850";
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Stop = "6000";
String Startup = "1000";
String State = Show; //Startup state
String OldState;
String PressShowR = "2110";
String PressRaceR = "3112";
String PressCutR = "3111";
String end10lapR = "5514";
String PressShowV = "2120";
String PressRaceV = "3122";
String PressCutV = "3121";
String end10lapV = "5524";
String PressShowB = "2130";
String PressRaceB = "3132";
String PressCutB = "3131";
String end10lapB = "5534";
String DoubleCutR = "4015";
String DoubleCutV = "4025";
String DoubleCutB = "4035";
String StopTimeR = "6514";
String StopTimeV = "6524";
String StopTimeB = "6534";
//Fine Configurazione

String TxData = "0";
const byte numChars = 32;
char RecCh[numChars];
boolean newData = false;
boolean newDataLora = false;

int lucerosso = 4;
int luceverde = 5;
int luceblu = 6;
int  ntaglirosso;
int  ntagliblu;
int  ntagliverde;
int lampeggianterosso = 0;
int onrosso = 0;
int lampeggianteverde = 0;
int onverde = 0;
int lampeggianteblu = 0;
int onblu = 0;

int Key = 0;
int Add = 1;
int Chan = 10;

unsigned long lampmillisrosso ;
unsigned long lampmillisverde ;
unsigned long lampmillisblu ;


void setup() {
  pinMode(luceverde, OUTPUT);
  pinMode(lucerosso, OUTPUT);
  pinMode(luceblu, OUTPUT);

  Serial.begin(9600);
  delay(500);
  SwSerial.begin(9600);
  delay(500);
  e22ttl.begin();

}
void loop() {

  RecStr();
  if (newData == true) {
    State = RecCh;
    Serial.println(State);
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, State);
    if (State == Show || State == Race ||  State == Stop ||  State == Avanti ) {
      digitalWrite(lucerosso, LOW);
      digitalWrite(luceverde, LOW);
      digitalWrite(luceblu, LOW);
      ntaglirosso = 0;
      lampeggianterosso = 0;
      ntagliverde = 0;
      lampeggianteverde = 0;
      ntagliblu = 0;
      lampeggianteblu = 0;
    }


    newData = false;
  }

  if (newDataLora == true) {
    String str = "<";
    str.concat(TxData);
    str.concat(">");
    SwSerial.println(str);
    Serial.println(TxData);
    if (State == Show && TxData == PressShowR ) {
      digitalWrite(lucerosso, HIGH);
    }
    if (State == Show && TxData == PressShowV) {
      digitalWrite(luceverde, HIGH);
    }
    if (State == Show && TxData == PressShowB ) {
      digitalWrite(luceblu, HIGH);
    }
    if ( TxData == PressCutR ) {
      ntaglirosso++;
    }
    if ( TxData == PressCutV ) {
      ntagliverde++;
    }
    if ( TxData == PressCutB ) {
      ntagliblu++;
    }
    newDataLora = false;
  }

  if (e22ttl.available() > 1) {
    ResponseContainer rc = e22ttl.receiveMessage();
    TxData = rc.data;
    newDataLora = true;

  }

  //tagli verde
  if (ntagliverde == 1) {
    digitalWrite( luceverde, HIGH);
  }
  if (ntagliverde > 1) {
    lampeggianteverde = 1;
  }
  // fine tagli verde

  //tagli rosso
  if (ntaglirosso == 1) {
    digitalWrite( lucerosso, HIGH);
  }
  if (ntaglirosso > 1) {
    lampeggianterosso = 1;
  }
  // fine tagli rosso

  //tagli blu
  if (ntagliblu == 1) {
    digitalWrite( luceblu, HIGH);
  }
  if (ntagliblu > 1) {
    lampeggianteblu = 1;
  }
  // fine tagli blu


  //codici per il lampeggio
  if (lampeggianterosso == 1) {
    if (onrosso == 0 && millis() - lampmillisrosso >= 500) {
      lampmillisrosso = millis();
      onrosso = 1;
      digitalWrite( lucerosso, HIGH);

    }
    if (onrosso == 1 && millis() - lampmillisrosso >= 500 ) {
      digitalWrite( lucerosso, LOW);
      onrosso = 0;
      lampmillisrosso = millis();

    }
  }

  if (lampeggianteverde == 1) {
    if (onverde == 0 && millis() - lampmillisverde >= 500) {
      lampmillisverde = millis();
      onverde = 1;
      digitalWrite( luceverde, HIGH);

    }
    if (onverde == 1 && millis() - lampmillisverde >= 500 ) {
      digitalWrite( luceverde, LOW);
      onverde = 0;
      lampmillisverde = millis();

    }
  }
  if (lampeggianteblu == 1) {
    if (onblu == 0 && millis() - lampmillisblu >= 500) {
      lampmillisblu = millis();
      onblu = 1;
      digitalWrite( luceblu, HIGH);

    }
    if (onblu == 1 && millis() - lampmillisblu >= 500 ) {
      digitalWrite( luceblu, LOW);
      onblu = 0;
      lampmillisblu = millis();

    }
  }
  // fine codici per il lampeggio
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
