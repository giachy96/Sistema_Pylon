#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial SwSerial;
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX

//Inizio Configurazione Ricevente
String Race = "3000";
String Show = "2000";
String Startup = "1000";
String Stop = "6000";
String State = Startup; //Startup state
String OldState = Startup;
unsigned long Delaysend = 200;
unsigned long DelayStop = 500;
unsigned long DelayLight = 1500;
String PressShow="2110";
String PressRace="3112";
int Key=0;
int Add=1;
int Chan=3;
//Fine Configurazione Ricevente

int RelayRed = 12;
String TxData = "0";
String TxDatastr;
unsigned long Timesend = 0;
unsigned long CurrentMillis = 0;
unsigned long StopSend = 0;
unsigned long timeonlight;
int onlight;
int on;
const byte numChars = 5;
char RecCh[numChars];
boolean newData = false;
int DoubleStop = 0;
unsigned long millisc = 0;

void setup() {
  //pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
  //pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500);
  SwSerial.begin(9600);
  delay(500);
  e22ttl.begin();
  pinMode(RelayRed, OUTPUT); //Output pin to Relay Red
  //Serial.println("prova");
}
void loop() {
  RecStr();
  if (newData == true) { //(SwSerial.available()>0){  // If mega will sent state to sem_rx
    Serial.println("C'è qualcosa in seriale - assegno nuovo stato");
    // State=SwSerial.readString(); //Assign system status to state
    State = RecCh;
    Serial.print(State);
    Serial.print(OldState);
    newData = false;
  }
  if (e22ttl.available() > 1) { // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    TxData = rc.data; //Assign incoming data on TxData variable
    Serial.println("Ricevo dal lora");
    millisc = millis();
    Serial.println(TxData);

    if (TxData != "0" && (State == Show || State == Race)) { //Condition for sent to mega press
      RecStr();//valutare se disabilitare
      if (newData == true) { //(SwSerial.available()>0){  // If mega will sent state to sem_rx
        Serial.println("C'è qualcosa in seriale - riassegno un nuovo stato ");
        State = RecCh;
        Serial.println(State);
        newData = false;
      }
      if (State == Show || State == Race) { //if nothing has change from previous if-statements
        SwSerial.print(TxData); //Send Txdata from rx to Mega
        Serial.println("Scrivo in seriale");
        millisc = millis();
        Serial.println(TxData);
        Serial.println(State);
      }
      if (State == Show || State == Race) { //if nothing has change from previous if-statements
        if (TxData.indexOf(PressShow) != -1 ||  TxData.indexOf(PressRace) != -1 )
          onlight = 1;
      }

      TxData = "0"; //Reset condition for set data
    }
  }
  //Serial.println("verifico se è cambiato stato");
  if ((State != OldState)) { //If statements who control that state was change and
    ResponseStatus rc = e22ttl.sendFixedMessage(Key, Add, Chan, State);
    Serial.println("Mando Stato nuovo");
    Serial.println(State);
    Serial.println("plotto vecchio Stato");
    Serial.println(OldState);
    if (State == Stop) {
      StopSend = millis();
      DoubleStop = 1;
    }
    OldState = State;
  }
  CurrentMillis = millis();
  if (DoubleStop == 1 && (CurrentMillis - StopSend) > DelayStop && State == Stop) {
    ResponseStatus rc = e22ttl.sendFixedMessage(Key, Add, Chan, State);
    DoubleStop = 0;
  }
  else if (State != Stop) {
    DoubleStop = 0;
  }

//DIOBESTIA TUTTE STE SEGHE PER UNA LUCE
  if (onlight == 1) {
    if (on == 0 ) {
      digitalWrite(RelayRed, HIGH);
      timeonlight = millis();
      on = 1;
    } else {
      if ((millis() - timeonlight) >= DelayLight &&  on == 1) {
        digitalWrite(RelayRed, LOW);
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
