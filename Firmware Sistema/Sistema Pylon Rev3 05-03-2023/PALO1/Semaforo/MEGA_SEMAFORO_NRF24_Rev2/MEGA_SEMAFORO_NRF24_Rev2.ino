//Inclusione delle librerie
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include "LoRa_E22.h"

LoRa_E22 e22ttl(10, 11);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX

//definizione indirizzo sul quale stabilire la comunicazione
const byte indirizzo[6] = {
  0x7878787878LL,  // telecomando rosso in TX
  0xB3B4B5B6F1LL,  // telecomando rosso in RX
  0xB3B4B5B6CDLL,  // telecomando verde in TX
  0xB3B4B5B6A3LL,  // telecomando verde in RX
  0xB3B4B5B60FLL,  //telecomando blu in TX
  0xB3B4B5B605LL   //telecomando blu in RX
};
//Creo un'istanza della "radio" passandogli il numero dei pin collegati a CE e CSN del modulo

RF24 radio(7, 8);  // CE, CSN


const byte numChars = 32;
char receivedChars1[numChars];
char receivedChars2[numChars];
char receivedChars3[numChars];
int FlagState = 0;

//Inizio Configurazione Mega
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Stop = "6000";
String Startup = "1000";
String end10lapB = "5534";
String end10lapV = "5524";
String end10lapR = "5514";
String DoubleCutR = "4015";
String DoubleCutV = "4025";
String DoubleCutB = "4035";
String StopTimeR = "6514";
String StopTimeV = "6524";
String StopTimeB = "6534";
String State;
int flag1 = 0;
int flag2 = 0;
int flag3 = 0;
String Rxs1;
String Rxs2;
String Rxs3;
unsigned long lastsend;

int Key = 0;
int Add = 0;
int Chan = 10;
//Fine Configurazione Mega

boolean newData1 = false;
boolean newData2 = false;
boolean newData3 = false;
boolean newDatalora = false;

// varibiali per luci
int relayrosso = 4;
int relayverde = 5;
int relayblu = 6;
unsigned long DelayLight = 1500;
unsigned long timeonlightR;
int onlightR;
int onR;
unsigned long timeonlightV;
int onlightV;
int onV;
unsigned long timeonlightB;
int onlightB;
int onB;
unsigned long DelayLightstart = 61600;
unsigned long timeonlightstart;
int onRstart;
int onVstart;
int onBstart;

void setup() {
  Serial1.begin(9600);  // Serial Verde
  delay(300);
  Serial2.begin(9600);  // Serial Rosso
  delay(300);
  Serial3.begin(9600);  // Serial Blu
  delay(300);
  Serial.begin(9600);  // Serial Pc interface
  delay(300);

  e22ttl.begin();  //Start e22ttl
  // Serial.println("prova");
  delay(300);

  //Inizializzo la radio 2.4GHZ
  radio.begin();

  /*
     La radio può lavorare a diverse potenze: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH e RF24_PA_MAX
     Che corrispondono a: -18dBm, -12dBm,-6dBM, e 0dBm
  */
  //Setto la potenza della radio a LOW
  radio.setPALevel(RF24_PA_MAX);

  //Apro un canale in lettura sull'indirizzo specificato
  radio.openReadingPipe(1, indirizzo[0]); // rosso
  radio.openReadingPipe(2, indirizzo[2]); // verde
  radio.openReadingPipe(3, indirizzo[4]); //blu


  //Metto la radio in ascolto
  radio.startListening();

  pinMode(relayrosso, OUTPUT);  //Output pin to Relay Red
  pinMode(relayverde, OUTPUT);  //Output pin to Relay Green
  pinMode(relayblu, OUTPUT);    //Output pin to Relay Blu
}
void loop() {
  //  Currentmillis = millis();
  recvSerial1();
  recvSerial2();
  recvSerial3();
  if (newData1 == true) {  // ricevo qualcosa dalla seriale VERDE
    Rxs1 = receivedChars1;
    Serial.println(Rxs1);
    if (Rxs1.indexOf("3122") != -1) {
      onlightV = 1;
    }
    if (Rxs1.indexOf("2120") != -1) {
      digitalWrite(relayverde, HIGH);
    }
    newData1 = false;
  }

  if (newData2 == true) {  // ricevo qualcosa dalla seriale ROSSA
    Rxs2 = receivedChars2;
    Serial.println(Rxs2);
    if (Rxs2.indexOf("3112") != -1) {
      onlightR = 1;
    }
    if (Rxs2.indexOf("2110") != -1) {
      digitalWrite(relayrosso, HIGH);
    }
    newData2 = false;
  }
  if (newData3 == true) {  // ricevo qualcosa dalla seriale BLU
    Rxs3 = receivedChars3;
    Serial.println(Rxs3);
    if (Rxs3.indexOf("3132") != -1) {
      onlightB = 1;
    }
    if (Rxs3.indexOf("2130") != -1) {
      digitalWrite(relayblu, HIGH);
    }
    newData3 = false;
  }

  if (Rxs1 != "" && (millis() - lastsend) > 100) {
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs1);
    flag1 = 0;
    lastsend = millis();
    Rxs1 = "";
  }
  if (Rxs2 != "" && (millis() - lastsend) > 100) {
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs2);
    flag2 = 0;
    lastsend = millis();
    Rxs2 = "";
  }
  if (Rxs3 != "" && (millis() - lastsend) > 100) {
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs3);
    flag3 = 0;
    lastsend = millis();
    Rxs3 = "";
  }

  uint8_t pipe;
  if (radio.available(&pipe)) {// se ricevo qualcosa da 2.4GHz
    Serial.print("ricevo in 2.4 dal pipe : ");
    Serial.println(pipe);
    //Creo una variabile di appoggio
    char buff[50]; // this must match dataToSend in the TX
    //Leggo i dati sul buffer e li salvo nella variabile di appoggio
    radio.read( &buff, sizeof(buff) );
    //Invio al monitor seriale il valore appena letto
    String Rx24str = String(buff);
    Serial.println(Rx24str);

    if (pipe == 1 ) { // pipe rosso
      if (Rx24str.indexOf("3112") != -1) {  //PressRaceR
        onlightR = 1;
      }
      else if (Rx24str.indexOf("2110") != -1) {  // PressShowR
        digitalWrite(relayrosso, HIGH);
        Rxs2 = Rx24str;
      }
      else {
        Rxs2 = Rx24str;
      }
    }

    if (pipe == 2 ) { // pipe verde
      if (Rx24str.indexOf("3122") != -1) {  //PressRaceV
        onlightV = 1;
      }
      else if (Rx24str.indexOf("2120") != -1) {  // PressShowV
        digitalWrite(relayverde, HIGH);
        Rxs1 = Rx24str;
      }
      else {
        Rxs1 = Rx24str;
      }
    }

    if (pipe == 3 ) {
      if (Rx24str.indexOf("3132") != -1) {  //PressRaceB
        onlightB = 1;
      }
      else if (Rx24str.indexOf("2130") != -1) {  // PressShowB
        digitalWrite(relayblu, HIGH);
        Rxs3 = Rx24str;
      }
      else {
        Rxs3 = Rx24str;
      }
    }
    
    Rx24str = "";
  }


  // per test a casa senza sistema
  if (Serial.available()) {
    String myString;
    myString = Serial.readString();
    Serial.println(myString);
    State =  myString;
    newDatalora = true;
  }
  // fine parte per test da casa


  if (e22ttl.available() > 1) {                      //If there's something incoming from lora
    ResponseContainer rc = e22ttl.receiveMessage();  //Receive message
    State = rc.data;
    newDatalora = true;
  }

  if (newDatalora == true) {
    Serial.println(State);
    if (State != end10lapB && State != end10lapR && State != end10lapV && State != DoubleCutR && State != DoubleCutB && State != DoubleCutV && State != StopTimeR && State != StopTimeV && State != StopTimeB ) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println(str);       // verde
      Serial2.println(str);       // rosso
      Serial3.println(str);       // blu
      sendStringMessageRF24(State, 1);  // mando all 2.4 rosso
      sendStringMessageRF24(State, 3);  // mando all 2.4 verde
      sendStringMessageRF24(State, 5);  // mando all 2.4 blu
      if (State == Race) {
        onlightR = 0;
        onRstart = 1;
        digitalWrite(relayrosso, HIGH);
        onlightV = 0;
        onVstart = 1;
        digitalWrite(relayverde, HIGH);
        onlightB = 0;
        onBstart = 1;
        digitalWrite(relayblu, HIGH);
        timeonlightstart=millis();
      }
      else if (State == StartRace) {
        onlightR = 0;
        onlightV = 0;
        onlightB = 0;
      }
      else {
        onlightR = 0;
        onRstart = 0;
        digitalWrite(relayrosso, LOW);
        onlightV = 0;
        onVstart = 0;
        digitalWrite(relayverde, LOW);
        onlightB = 0;
        onBstart = 0;
        digitalWrite(relayblu, LOW);
        }
    }
    if (State == end10lapR || State == DoubleCutR || State == StopTimeR) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial2.println(str);       // rosso
      sendStringMessageRF24(State, 1);  // mando all 2.4 rosso
    }
    if (State == end10lapV || State == DoubleCutV || State == StopTimeV) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println(str);       // verde
      sendStringMessageRF24(State, 3);  // mando all 2.4 verde
    }
    if (State == end10lapB || State == DoubleCutB || State == StopTimeB) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial3.println(str);       // blu
      sendStringMessageRF24(State, 5);  // mando all 2.4 blu
    }
    newDatalora = false;
  }

  // Luce Rosso

  if (onlightR == 1) {
    if (onR == 0) {
      digitalWrite(relayrosso, HIGH);
      timeonlightR = millis();
      onR = 1;
    } else {
      if ((millis() - timeonlightR) >= DelayLight && onR == 1) {
        digitalWrite(relayrosso, LOW);
        onlightR = 0;
        onR = 0;
      }
    }
  }

  if (onRstart==1 && (millis()-timeonlightstart)>DelayLightstart){
    digitalWrite(relayrosso, LOW);
    onRstart=0;
  }


  // Luce Verde


  if (onlightV == 1) {
    if (onV == 0) {
      digitalWrite(relayverde, HIGH);
      timeonlightV = millis();
      onV = 1;
    } else {
      if ((millis() - timeonlightV) >= DelayLight && onV == 1) {
        digitalWrite(relayverde, LOW);
        onlightV = 0;
        onV = 0;
      }
    }
  }

  if (onVstart==1 && (millis()-timeonlightstart)>(DelayLightstart+1100)){
    digitalWrite(relayverde, LOW);
    onVstart=0;
  }

  // Luce Blu

  if (onlightB == 1) {
    if (onB == 0) {
      digitalWrite(relayblu, HIGH);
      timeonlightB = millis();
      onB = 1;
    } else {
      if ((millis() - timeonlightB) >= DelayLight && onB == 1) {
        digitalWrite(relayblu, LOW);
        onlightB = 0;
        onB = 0;
      }
    }
  }
    if (onBstart==1 && (millis()-timeonlightstart)>(DelayLightstart+2100)){
    digitalWrite(relayblu, LOW);
    onBstart=0;
  }
}



void recvSerial1() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial1.available() > 0 && newData1 == false) {
    //while (port.available() > 0 ) {
    rc = Serial1.read();


    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars1[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars1[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData1 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}


void recvSerial2() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial2.available() > 0 && newData2 == false) {
    //while (port.available() > 0 ) {
    rc = Serial2.read();


    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars2[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars2[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData2 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void recvSerial3() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial3.available() > 0 && newData3 == false) {
    //while (port.available() > 0 ) {
    rc = Serial3.read();


    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars3[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars3[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData3 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void sendStringMessageRF24(String valore , int ind ) {
  char charBuf[50];
  valore.toCharArray(charBuf, 50);
  radio.stopListening();  // put radio in TX mode
  radio.openWritingPipe(indirizzo[ind]);
  //Invio il valore per radio
  radio.write(&charBuf, sizeof(charBuf));
  //radio.write(valore.c_str(), valore.length());
  radio.startListening();  // put radio in RX mode

}
