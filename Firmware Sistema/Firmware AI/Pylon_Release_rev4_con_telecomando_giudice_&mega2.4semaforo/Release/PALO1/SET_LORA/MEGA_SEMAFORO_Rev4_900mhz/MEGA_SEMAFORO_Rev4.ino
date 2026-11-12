// ===== MEGA_SEMAFORO_Rev4 (set LoRa 900 MHz) - revisione incrementata (precedente: MEGA_SEMAFORO_Rev3) - vedi CAMBIAMENTI.txt =====
// Mega del SEMAFORO (palo 1) - telecomandi/RX del palo 1 via LoRa
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
#include "LoRa_E22.h"

//#define TEST_DA_PC   // togliere il commento SOLO per provare il semaforo a casa dal monitor seriale

LoRa_E22 e22ttl(10, 11);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX

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
    }
    if (State == end10lapV || State == DoubleCutV || State == StopTimeV) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial1.println(str);  // verde
    }
    if (State == end10lapB || State == DoubleCutB || State == StopTimeB) {
      String str = "<";
      str.concat(State);
      str.concat(">");
      Serial3.println(str);  // blu
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

