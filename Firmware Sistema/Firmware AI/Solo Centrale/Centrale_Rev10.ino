// Centrale Rev10 - FT Models
// Stessa logica e stesso protocollo LoRa/PC della Rev9, con correzioni di stabilità:
//  - niente memset sulle String (perdita di memoria heap)
//  - controlli sui limiti di array e buffer seriale
//  - allarme batteria con isteresi, una sola volta per intervallo
//  - riconoscimento pacchetti solo sul primo campo (codice)
//  - STOP fisico non bloccato dagli altri pulsanti
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "LoRa_E22.h"
#include "lcdcases.h"
#include "commacode.h"
#include "printercases.h"
#include "exceldecode.h"


byte Key = 0;
byte Add = 0;
byte Chan = 5;


int buzzer = 12;
int pstop = 2;
int pgo = 3;
int pshow = 6;
int r_print = 8;
int ledstatusPC = 4;
bool statusPC = false;
int checkPC = 7;
int pinbatt = A0;

const unsigned long intervalcheckPC = 1000;  // controllo PC connesso
const unsigned long interval = 3000;         // lettura batteria / refresh riga batteria
const unsigned long intervalPress = 1000;    // anti doppia pressione

// batteria: allarme sotto SOGLIA_BATT_ALLARME, rientra sopra SOGLIA_BATT_RIENTRO.
// Sotto BATT_MIN_VALIDA si assume ingresso scollegato/alimentazione USB: nessun allarme.
const float SOGLIA_BATT_ALLARME = 10.2;
const float SOGLIA_BATT_RIENTRO = 10.6;
const float BATT_MIN_VALIDA = 5.0;
bool allarmeBatt = false;

unsigned long previousMillis = 0;
unsigned long oldPress = 0;
unsigned long oldcheckPC = 0;
unsigned long oldStop = 0UL - intervalPress;  // così lo STOP funziona anche subito dopo l'accensione
int doublePress = 0;

String arraytempirosso[12];
String arraytaglirosso[12];
String arraytempiverde[12];
String arraytagliverde[12];
String arraytempiblu[12];
String arraytagliblu[12];
int datiblu = 0;
int datirosso = 0;
int dativerde = 0;
String stringonerosso;
String stringoneverde;
String stringoneblu;
String punteggiorosso;
String punteggioverde;
String punteggioblu;

String RxData = "";

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
// lora parametri
LoRa_E22 e22ttl(&Serial2);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX


void invioLoRa(const String &msg) {
  e22ttl.sendFixedMessage(Key, Add, Chan, msg);
}

void softReset() {
#ifdef __AVR__
  noInterrupts();
  asm volatile (" jmp 0");
#else
  while (true) {}
#endif
}

// Svuota i dati della gara. Assegnare "" alle String riusa il loro buffer:
// il memset della Rev9 azzerava i puntatori senza liberare la memoria (leak).
void azzeraDati() {
  for (int i = 0; i < 12; i++) {
    arraytempirosso[i] = "";
    arraytaglirosso[i] = "";
    arraytempiverde[i] = "";
    arraytagliverde[i] = "";
    arraytempiblu[i] = "";
    arraytagliblu[i] = "";
  }
  datiblu = 0;
  datirosso = 0;
  dativerde = 0;
  stringonerosso = "";
  stringoneverde = "";
  stringoneblu = "";
  punteggiorosso = "";
  punteggioverde = "";
  punteggioblu = "";
}

// punteggio = tempo totale; con almeno un taglio (P1/P2/P3) il tempo è maggiorato del 10%
String calcolaPunteggio(String tagli[], String tempi[]) {
  for (int i = 0; i < 11; i++) {
    if (tagli[i].indexOf("P") != -1) {
      return String(tempi[11].toFloat() * 1.10, 3);
    }
  }
  return tempi[11];
}

// Registra il pacchetto ricevuto (RxData) per un pilota.
// colore: 1 rosso, 2 verde, 3 blu. punteggioFisso: doppio taglio / timeout -> 200
void registraPilota(int colore, bool punteggioFisso) {
  String *tempi;
  String *tagli;
  String *punteggio;
  String *stringone;
  int *flag;

  switch (colore) {
    case 1:
      tempi = arraytempirosso; tagli = arraytaglirosso;
      punteggio = &punteggiorosso; stringone = &stringonerosso; flag = &datirosso;
      break;
    case 2:
      tempi = arraytempiverde; tagli = arraytagliverde;
      punteggio = &punteggioverde; stringone = &stringoneverde; flag = &dativerde;
      break;
    case 3:
      tempi = arraytempiblu; tagli = arraytagliblu;
      punteggio = &punteggioblu; stringone = &stringoneblu; flag = &datiblu;
      break;
    default:
      return;
  }

  decodestringone(RxData, tempi, tagli);
  if (punteggioFisso) {
    *punteggio = "200";
  } else {
    *punteggio = calcolaPunteggio(tagli, tempi);
  }
  *stringone = RxData;
  *flag = 1;
}


void setup() {
  Serial.begin(9600);            // comunicazione con il PC
  Serial3.begin(19200);          // stampante
  pinMode(buzzer, OUTPUT);
  pinMode(pstop, INPUT_PULLUP);
  pinMode(pgo, INPUT_PULLUP);
  pinMode(pinbatt, INPUT);
  pinMode(r_print, INPUT_PULLUP);
  pinMode(checkPC, INPUT);
  pinMode(pshow, INPUT_PULLUP);
  pinMode(ledstatusPC, OUTPUT);
  Serial2.begin(9600);
  e22ttl.begin();
#ifdef WIRE_HAS_TIMEOUT
  Wire.setWireTimeout(25000, true);  // un disturbo sull'I2C non blocca più il loop
#endif
  lcd.init();
  lcd.backlight();
  draw(0, lcd);
  tone(buzzer, 4000, 200);
}

void loop() {
  unsigned long currentMillis = millis();

  // ---- PC connesso? ----
  if (currentMillis - oldcheckPC >= intervalcheckPC) {
    oldcheckPC = currentMillis;
    if (digitalRead(checkPC) == HIGH) {
      digitalWrite(ledstatusPC, HIGH);
      statusPC = true;
    } else {
      digitalWrite(ledstatusPC, LOW);
      gara = "";
      manche_rx = 0;
      round_rx = 0;
      nome_rosso = "";
      cognome_rosso = "";
      nome_verde = "";
      cognome_verde = "";
      nome_blu = "";
      cognome_blu = "";
      statusPC = false;
      flushSerialePC();  // niente righe vecchie o a metà alla prossima connessione
    }
  }

  // ---- anti doppia pressione ----
  if (currentMillis - oldPress >= intervalPress) {
    doublePress = 0;
  }

  // ---- batteria: lettura, display e allarme (una volta per intervallo) ----
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float vout = tensionebatt(pinbatt);
    stampaBatteria(lcd, vout);
    if (vout >= BATT_MIN_VALIDA) {
      if (vout < SOGLIA_BATT_ALLARME) {
        allarmeBatt = true;
      } else if (vout > SOGLIA_BATT_RIENTRO) {
        allarmeBatt = false;
      }
    } else {
      allarmeBatt = false;
    }
    if (allarmeBatt) {
      tone(buzzer, 5000, 500);
    }
  }

  // ---- comando INDIETRO dal tabellone ----
  if (RxData == "700") {
    if (statusPC == true) {
      Serial.println("700,0,0,0,0,0,0,0,0,0");
    } else {
      invioLoRa("750,NO PC ,NO PC ,NO PC, ,NO PC, ,NO PC, ,NO PC");
    }
    doublePress = 1;
    oldPress = millis();
    RxData = "";
  }

  // ---- comando AVANTI dal tabellone ----
  if (RxData == "800") {
    if (statusPC == true) {
      Serial.println("800,0,0,0,0,0,0,0,0,0");
    } else {
      invioLoRa("850,NO PC ,NO PC,NO PC, ,NO PC, ,NO PC, ,NO PC");
    }
    doublePress = 1;
    oldPress = millis();
    RxData = "";
  }

  // ---- SHOW ----
  if ((digitalRead(pshow) == LOW && doublePress == 0) || RxData == "2000" || code_rx == 2000) {
    tone(buzzer, 4000, 200);
    azzeraDati();
    doublePress = 1;
    oldPress = millis();
    invioLoRa("1500");  // invio reset
    delay(500);
    if (RxData != "2000") {
      invioLoRa("2000");  // invio show
    }
    draw(2, lcd);
    RxData = "";
    code_rx = 0;
  }

  // ---- GO ----
  if ((digitalRead(pgo) == LOW && doublePress == 0) || RxData == "3000" || code_rx == 3000) {
    tone(buzzer, 4500, 200);
    azzeraDati();
    if (RxData != "3000") {
      invioLoRa("3000");
    }
    draw(3, lcd);
    doublePress = 1;
    oldPress = millis();
    RxData = "";
    code_rx = 0;
  }

  // ---- STOP (il tasto fisico non dipende dagli altri pulsanti, ha solo il suo anti-rimbalzo) ----
  bool stopFisico = (digitalRead(pstop) == LOW) && (currentMillis - oldStop >= intervalPress);
  if (stopFisico || RxData == "6000" || code_rx == 6000) {
    tone(buzzer, 5000, 200);
    draw(5, lcd);
    if (RxData != "6000") {
      invioLoRa("6000");
    }
    doublePress = 1;
    oldPress = millis();
    oldStop = millis();
    RxData = "";
    code_rx = 0;
  }

  // ---- ristampa tempi ----
  if (digitalRead(r_print) == LOW && doublePress == 0) {
    stampatotali(gara, manche_rx, round_rx, nome_rosso, cognome_rosso, nome_verde, cognome_verde, nome_blu, cognome_blu);
    doublePress = 1;
    oldPress = millis();
  }

  // ---- pacchetti dai piloti ----
  // Il tipo si riconosce SOLO dal primo campo (il codice): un tempo che contiene
  // per caso "5514" o simili non viene più scambiato per un pacchetto diverso.
  if (RxData.length() > 0) {
    String head = RxData;
    int virgola = head.indexOf(',');
    if (virgola >= 0) {
      head = head.substring(0, virgola);
    }

    int colore = 0;
    bool fisso = false;  // true = punteggio 200 (doppio taglio / timeout)
    if (head.indexOf("5514") != -1)      { colore = 1; }  // fine 10 giri
    else if (head.indexOf("5524") != -1) { colore = 2; }
    else if (head.indexOf("5534") != -1) { colore = 3; }
    else if (head.indexOf("4015") != -1) { colore = 1; fisso = true; }  // doppio taglio
    else if (head.indexOf("4025") != -1) { colore = 2; fisso = true; }
    else if (head.indexOf("4035") != -1) { colore = 3; fisso = true; }
    else if (head.indexOf("6514") != -1) { colore = 1; fisso = true; }  // timeout
    else if (head.indexOf("6524") != -1) { colore = 2; fisso = true; }
    else if (head.indexOf("6534") != -1) { colore = 3; fisso = true; }

    if (colore != 0) {
      draw(4, lcd);
      tone(buzzer, 3000, 200);
      registraPilota(colore, fisso);
    }
  }

  // ---- ricevuti tutti e tre: stampo e mando al PC ----
  if (dativerde == 1 && datirosso == 1 && datiblu == 1) {
    stampatotali(gara, manche_rx, round_rx, nome_rosso, cognome_rosso, nome_verde, cognome_verde, nome_blu, cognome_blu);
    datirosso = 0;
    dativerde = 0;
    datiblu = 0;
    Serial.println(stringonerosso);
    delay(200);
    Serial.println(stringoneverde);
    delay(200);
    Serial.println(stringoneblu);
  }

  // ---- lettura seriale dal PC ----
  // esempio riga: 805,Q500,2,3,Matteo,Giacomo,Francesco,Luca,Paolo,Pippo
  if (statusPC == true) {
    leggiSeriale();
  }

  // ---- inoltro al tabellone dei dati ricevuti dal PC ----
  if (newDataPC == true) {
    String stringanomi;
    stringanomi.reserve(120);
    stringanomi.concat(String(code_rx));
    stringanomi.concat(",");
    stringanomi.concat(gara);
    stringanomi.concat(",");
    stringanomi.concat(String(manche_rx));
    stringanomi.concat(",");
    stringanomi.concat(String(round_rx));
    stringanomi.concat(",");
    stringanomi.concat(nome_rosso);
    stringanomi.concat(",");
    stringanomi.concat(cognome_rosso);
    stringanomi.concat(",");
    stringanomi.concat(nome_verde);
    stringanomi.concat(",");
    stringanomi.concat(cognome_verde);
    stringanomi.concat(",");
    stringanomi.concat(nome_blu);
    stringanomi.concat(",");
    stringanomi.concat(cognome_blu);
    if (code_rx == 850) {
      tone(buzzer, 4000, 200);
      draw(1, lcd);
    }
    if (code_rx == 750) {
      tone(buzzer, 4000, 200);
      draw(11, lcd);
    }
    if (code_rx == 1400) {
      softReset();
    }
    invioLoRa(stringanomi);

    newDataPC = false;
  }

  // ---- ricezione LoRa ----
  // Tutto quello che non è stato riconosciuto sopra viene scartato, così un messaggio
  // sconosciuto non resta in RxData e non viene rivalutato a ogni giro.
  RxData = "";
  if (e22ttl.available() > 1) {
    ResponseContainer rc = e22ttl.receiveMessage();
    if (rc.status.code == 1) {  // 1 = E22_SUCCESS
      RxData = rc.data;
    }
  }
}
