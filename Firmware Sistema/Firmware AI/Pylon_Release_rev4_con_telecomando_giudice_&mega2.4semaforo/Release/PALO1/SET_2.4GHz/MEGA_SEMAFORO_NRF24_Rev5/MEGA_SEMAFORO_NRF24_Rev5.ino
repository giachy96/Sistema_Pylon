// ===== MEGA_SEMAFORO_NRF24_Rev5 - revisione incrementata (precedente: MEGA_SEMAFORO_NRF24_Rev4) - vedi CAMBIAMENTI.txt =====
// Mega del SEMAFORO (palo 1) - telecomandi via nRF24 2.4 GHz + E22 verso il tabellone
// Logica identica nei due set (2.4 GHz e LoRa): cambia solo la radio verso i telecomandi.
// Correzioni rispetto al riferimento MEGA_SEMAFORO_NRF24_Rev2:
//  - luci di partenza (timer 61,6 s / +1,1 s / +2,1 s) presenti in ENTRAMBI i set (nel set LoRa mancavano)
//  - un cambio di stato (3000/3001/Show/Stop...) annulla i lampeggi dei tagli in corso: prima la luce
//    restava accesa fino alla pressione successiva e onR/onV/onB restavano a 1 (il lampeggio dopo non partiva)
//  - coda di invio verso il tabellone (8 messaggi): prima un solo messaggio per colore e uno nuovo
//    sovrascriveva quello ancora da inviare
//  - messaggio LoRa ricevuto ripulito (trim) prima del confronto con i codici
//  - test da PC (readString) disattivato di default: #define TEST_DA_PC
//Inclusione delle librerie
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include "LoRa_E22.h"

//#define TEST_DA_PC   // togliere il commento SOLO per provare il semaforo a casa dal monitor seriale

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
RF24 radio(7, 8);  // CE, CSN

const byte numChars = 32;
char receivedChars1[numChars];
char receivedChars2[numChars];
char receivedChars3[numChars];

//Inizio Configurazione Mega
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Stop = "6000";
String end10lapB = "5534";
String end10lapV = "5524";
String end10lapR = "5514";
String DoubleCutR = "4015";
String DoubleCutV = "4025";
String DoubleCutB = "4035";
String StopTimeR = "6514";
String StopTimeV = "6524";
String StopTimeB = "6534";
String PressShowR = "2110";
String PressShowV = "2120";
String PressShowB = "2130";
String PressRaceR = "3112";
String PressRaceV = "3122";
String PressRaceB = "3132";
String State;
unsigned long lastsend;

int Key = 0;
int Add = 0;
int Chan = 10;
//Fine Configurazione Mega

boolean newData1 = false;
boolean newData2 = false;
boolean newData3 = false;
boolean newDatalora = false;

// coda di invio LoRa verso il tabellone
const byte QLEN = 8;
char coda[QLEN][numChars + 1];
byte qHead = 0, qTail = 0, qCount = 0;

// variabili per luci
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

void accoda(const String &s) {
  if (s.length() == 0) return;
  if (qCount >= QLEN) {  // coda piena: scarto il piu' vecchio
    qHead = (qHead + 1) % QLEN;
    qCount--;
  }
  s.toCharArray(coda[qTail], numChars + 1);
  qTail = (qTail + 1) % QLEN;
  qCount++;
}

void svuotaCoda() {  // un invio per giro, distanziati di almeno 100 ms
  if (qCount > 0 && (millis() - lastsend) > 100) {
    String m = coda[qHead];
    qHead = (qHead + 1) % QLEN;
    qCount--;
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, m);
    lastsend = millis();
  }
}

// annulla i lampeggi dei tagli in corso: se la luce era accesa per il lampeggio (e non per la partenza) la spegne.
// Prima la luce restava accesa fino alla pressione successiva e il lampeggio dopo non partiva.
void azzeraLuciTaglio() {
  if (onR == 1 && onRstart == 0) digitalWrite(relayrosso, LOW);
  if (onV == 1 && onVstart == 0) digitalWrite(relayverde, LOW);
  if (onB == 1 && onBstart == 0) digitalWrite(relayblu, LOW);
  onlightR = 0; onR = 0;
  onlightV = 0; onV = 0;
  onlightB = 0; onB = 0;
}

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
  delay(300);

  //Inizializzo la radio 2.4GHZ
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);  // RF24_PA_MIN -18 dBm, LOW -12, HIGH -6, MAX 0 dBm

  //Apro un canale in lettura per ogni telecomando
  radio.openReadingPipe(1, indirizzo[0]);  // rosso
  radio.openReadingPipe(2, indirizzo[2]);  // verde
  radio.openReadingPipe(3, indirizzo[4]);  // blu
  radio.startListening();

  pinMode(relayrosso, OUTPUT);  //Output pin to Relay Red
  pinMode(relayverde, OUTPUT);  //Output pin to Relay Green
  pinMode(relayblu, OUTPUT);    //Output pin to Relay Blu
}

void loop() {
  recvSerial1();
  recvSerial2();
  recvSerial3();
  if (newData1 == true) {  // ricevo qualcosa dalla seriale VERDE
    String r = receivedChars1;
    Serial.println(r);
    if (r.indexOf(PressRaceV) != -1) {
      onlightV = 1;
    }
    if (r.indexOf(PressShowV) != -1) {
      digitalWrite(relayverde, HIGH);
    }
    accoda(r);
    newData1 = false;
  }

  if (newData2 == true) {  // ricevo qualcosa dalla seriale ROSSA
    String r = receivedChars2;
    Serial.println(r);
    if (r.indexOf(PressRaceR) != -1) {
      onlightR = 1;
    }
    if (r.indexOf(PressShowR) != -1) {
      digitalWrite(relayrosso, HIGH);
    }
    accoda(r);
    newData2 = false;
  }
  if (newData3 == true) {  // ricevo qualcosa dalla seriale BLU
    String r = receivedChars3;
    Serial.println(r);
    if (r.indexOf(PressRaceB) != -1) {
      onlightB = 1;
    }
    if (r.indexOf(PressShowB) != -1) {
      digitalWrite(relayblu, HIGH);
    }
    accoda(r);
    newData3 = false;
  }

  svuotaCoda();

  uint8_t pipe;
  byte letti = 0;
  while (letti < 4 && radio.available(&pipe)) {  // pacchetti 2.4 GHz dai telecomandi (max 4 per giro)
    letti++;
    // il payload RF24 e' di 32 byte: buffer azzerato e terminato
    char buff[33];
    memset(buff, 0, sizeof(buff));
    uint8_t lenPayload = radio.getPayloadSize();
    if (lenPayload > 32) lenPayload = 32;
    radio.read(buff, lenPayload);
    buff[32] = '\0';
    String Rx24str = String(buff);
    Rx24str.trim();
    Serial.print("ricevo in 2.4 dal pipe : ");
    Serial.println(pipe);
    Serial.println(Rx24str);

    if (pipe == 1) {  // pipe rosso
      if (Rx24str.indexOf(PressRaceR) != -1) {
        onlightR = 1;
      } else if (Rx24str.indexOf(PressShowR) != -1) {
        digitalWrite(relayrosso, HIGH);
        accoda(Rx24str);
      } else {
        accoda(Rx24str);
      }
    }
    if (pipe == 2) {  // pipe verde
      if (Rx24str.indexOf(PressRaceV) != -1) {
        onlightV = 1;
      } else if (Rx24str.indexOf(PressShowV) != -1) {
        digitalWrite(relayverde, HIGH);
        accoda(Rx24str);
      } else {
        accoda(Rx24str);
      }
    }
    if (pipe == 3) {  // pipe blu
      if (Rx24str.indexOf(PressRaceB) != -1) {
        onlightB = 1;
      } else if (Rx24str.indexOf(PressShowB) != -1) {
        digitalWrite(relayblu, HIGH);
        accoda(Rx24str);
      } else {
        accoda(Rx24str);
      }
    }
  }

  // per test a casa senza sistema (disattivato di default)
#ifdef TEST_DA_PC
  if (Serial.available()) {
    String myString = Serial.readStringUntil('\n');
    myString.trim();
    if (myString.length() > 0) {
      Serial.println(myString);
      State = myString;
      newDatalora = true;
    }
  }
#endif

  if (e22ttl.available() > 1) {                      //If there's something incoming from lora
    ResponseContainer rc = e22ttl.receiveMessage();  //Receive message
    State = rc.data;
    State.trim();
    newDatalora = true;
  }

  if (newDatalora == true) {
    Serial.println(State);
    if (State != end10lapB && State != end10lapR && State != end10lapV && State != DoubleCutR && State != DoubleCutB && State != DoubleCutV && State != StopTimeR && State != StopTimeV && State != StopTimeB) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println(str);  // verde
      Serial2.println(str);  // rosso
      Serial3.println(str);  // blu
      sendStringMessageRF24(State, 1);  // mando al 2.4 rosso
      sendStringMessageRF24(State, 3);  // mando al 2.4 verde
      sendStringMessageRF24(State, 5);  // mando al 2.4 blu
      if (State == Race) {
        azzeraLuciTaglio();
        onRstart = 1;
        digitalWrite(relayrosso, HIGH);
        onVstart = 1;
        digitalWrite(relayverde, HIGH);
        onBstart = 1;
        digitalWrite(relayblu, HIGH);
        timeonlightstart = millis();
      } else if (State == StartRace) {
        azzeraLuciTaglio();
      } else {
        azzeraLuciTaglio();
        onRstart = 0;
        digitalWrite(relayrosso, LOW);
        onVstart = 0;
        digitalWrite(relayverde, LOW);
        onBstart = 0;
        digitalWrite(relayblu, LOW);
      }
    }
    if (State == end10lapR || State == DoubleCutR || State == StopTimeR) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial2.println(str);  // rosso
      sendStringMessageRF24(State, 1);  // mando al 2.4 rosso
    }
    if (State == end10lapV || State == DoubleCutV || State == StopTimeV) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println(str);  // verde
      sendStringMessageRF24(State, 3);  // mando al 2.4 verde
    }
    if (State == end10lapB || State == DoubleCutB || State == StopTimeB) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial3.println(str);  // blu
      sendStringMessageRF24(State, 5);  // mando al 2.4 blu
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
  if (onRstart == 1 && (millis() - timeonlightstart) > DelayLightstart) {
    digitalWrite(relayrosso, LOW);
    onRstart = 0;
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
  if (onVstart == 1 && (millis() - timeonlightstart) > (DelayLightstart + 1100)) {
    digitalWrite(relayverde, LOW);
    onVstart = 0;
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
  if (onBstart == 1 && (millis() - timeonlightstart) > (DelayLightstart + 2100)) {
    digitalWrite(relayblu, LOW);
    onBstart = 0;
  }
}

void recvSerial1() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial1.available() > 0 && newData1 == false) {
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
    } else if (rc == startMarker) {
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
    } else if (rc == startMarker) {
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
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void sendStringMessageRF24(String valore, int ind) {
  char charBuf[32];  // 32 = payload RF24
  valore.toCharArray(charBuf, sizeof(charBuf));
  radio.stopListening();  // put radio in TX mode
  radio.openWritingPipe(indirizzo[ind]);
  radio.write(&charBuf, sizeof(charBuf));
  radio.startListening();  // put radio in RX mode
}
