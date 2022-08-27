#include <Metro.h>
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
int secure = 0;                    // valore per evitare la pressione doppia
unsigned long previousMillis = 0;  // will store last time voltage was updated
unsigned long oldPress = 0;
long intervalcheckPC = 1000;
long oldcheckPC = 0;
long interval = 3000;  // refresh schermo
long intervalPress = 1000;  // refresh schermo
int doublePress = 0;
String arraytempirosso[12];
String arraytaglirosso[11];
String arraytempiverde[12];
String arraytagliverde[11];
String arraytempiblu[12];
String arraytagliblu[11];
int datiblu = 0;
int datirosso = 0;
int dativerde = 0;
String stringonerosso ;
String stringoneverde ;
String stringoneblu ;
String punteggiorosso;
String punteggioverde;
String punteggioblu;

extern int code_rx;
extern String gara ;
extern int manche_rx;
extern int round_rx;
extern String nome_rosso;
extern String cognome_rosso;
extern String nome_verde;
extern String cognome_verde;
extern String nome_blu;
extern String cognome_blu;
extern bool newDataPC;

String RxData = "";

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
// lora parametri
LoRa_E22 e22ttl(&Serial2);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX



void setup() {
  Serial.begin(9600);            // imposta la comunicazione seriale
  Serial3.begin(19200);           // Initialize HwSerial per la Stampanate
  pinMode(buzzer, OUTPUT);       // imposta come input il pin 10
  pinMode(pstop, INPUT_PULLUP);  // imposta come input il pin 11
  pinMode(pgo, INPUT_PULLUP);    // imposta come input il pin 12
  pinMode(pinbatt, INPUT);
  pinMode(r_print, INPUT_PULLUP);  // imposta come input il pin 12
  pinMode(checkPC, INPUT);
  pinMode(pshow, INPUT_PULLUP);
  pinMode(ledstatusPC, OUTPUT);
  Serial2.begin(9600);
  e22ttl.begin();
  lcd.init();
  lcd.backlight();
  draw(0, lcd);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);


}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - oldcheckPC >= intervalcheckPC) { // intervallo per vedere se il pc è connesso
    if (digitalRead(checkPC) == HIGH) {
      digitalWrite(ledstatusPC, HIGH);
      statusPC = true;
    } else {
      digitalWrite(ledstatusPC, LOW);
      gara = "" ;
      manche_rx = 0 ;
      round_rx = 0 ;
      nome_rosso = "";
      cognome_rosso = "";
      nome_verde = "";
      cognome_verde = "";
      nome_blu = "";
      cognome_blu = "";
      statusPC = false;
    }
    oldcheckPC = millis();
  }

  if (currentMillis - oldPress >= intervalPress) { // intervallo per la doppia pressione del pulsante
    doublePress = 0;
  }

  //  if (tensionebatt(pinbatt) < 9.5) {  // allarme tensione batteria
  //    tone(buzzer, 5000, 500);
  //  }
  if (currentMillis - previousMillis >= interval) { // intervallo lettura batteria
    // save the last time you read
    previousMillis = currentMillis;
    // read the tension of the battery
    float vout = tensionebatt(pinbatt);
    lcd.setCursor(3, 3);
    lcd.print("V.Batt.");
    lcd.print(vout);
  }

  if ( RxData == "700") { // ho ricevuto il comando INDIETRO
    tone(buzzer, 2000, 200);
    draw(11, lcd);
    if (statusPC == true) {
      Serial.println("700,0,0,0,0,0,0,0,0,0");
    } else {
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "750, ,0,0, ,NO PC, ,NO PC, ,NO PC");
    }
    doublePress = 1;
    oldPress = millis();
    RxData = "";
  }
  if ( RxData == "800") { // ho ricevuto il comando AVANTI
    tone(buzzer, 4000, 200);
    draw(1, lcd);
    if (statusPC == true) {
      Serial.println("800,0,0,0,0,0,0,0,0,0");
    } else {
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "850, ,0,0, ,NO PC, ,NO PC, ,NO PC");
    }

    doublePress = 1;
    oldPress = millis();
    RxData = "";
  }

  if ((digitalRead(pshow) == LOW && doublePress == 0 ) || RxData == "2000") { // caso show
    tone(buzzer, 4000, 200);
    doublePress = 1;
    oldPress = millis();
    if (RxData != "2000") {
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "2000");
    }
    draw(2, lcd);
    RxData = "";
  }

  if ((digitalRead(pgo) == LOW && doublePress == 0  ) || RxData == "3000") { // caos go
    tone(buzzer, 4500, 200);
    //Serial.println("800,0,0,0,0,0,0,0,0,0");
    if (RxData != "3000") {
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "3000");
    }
    draw(3, lcd);
    doublePress = 1;
    oldPress = millis();
    RxData = "";
  }
  if ((digitalRead(pstop) == LOW && doublePress == 0) ||  RxData == "6000") { // caso di stop
    tone(buzzer, 5000, 200);
    draw(5, lcd);
    if (RxData != "6000") {
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "6000");
    }
    doublePress = 1;
    oldPress = millis();
    RxData = "";
  }

  if (digitalRead(r_print) == LOW && doublePress == 0) {  // ristampa tempi
    stampatotali( gara, manche_rx, round_rx, nome_rosso, cognome_rosso, nome_verde, cognome_verde, nome_blu, cognome_blu);
    doublePress = 1;
    oldPress = millis();
  }


  if (RxData.indexOf("5514") != -1 || RxData.indexOf("5534") != -1 || RxData.indexOf("5524") != -1) {  // se ricevo fine 10 giri
    //   draw(4, lcd);
    // tone(buzzer, 3000, 200);
    if (RxData.indexOf("5514") != -1) {
      decodestringone(RxData, 1);
      int onecut = 0;
      for (int i = 0; i < 11; i++) {
        if (arraytaglirosso[i].indexOf("P") != -1) {
          onecut++;
        }
      }
      if (onecut > 0) {
        float calc10;
        calc10 =  arraytempirosso[11].toFloat();
        calc10= calc10*1.10;
        punteggiorosso = String(calc10, 3);
  

      } else {
        punteggiorosso =  arraytempirosso[11];
      }
      stringonerosso = RxData;
      datirosso = 1;
    }

    if (RxData.indexOf("5524") != -1) {
      decodestringone(RxData, 2);
      stringoneverde = RxData;
      dativerde = 1;
      punteggioverde =  arraytempiverde[11];
    }
    if (RxData.indexOf("5534") != -1) {
      decodestringone(RxData, 3);
      stringoneblu = RxData;
      datiblu = 1;
      punteggioblu =  arraytempiblu[11];
    }
    RxData = "";
  }

  if (RxData.indexOf("4015") != -1 || RxData.indexOf("4025") != -1 || RxData.indexOf("4035") != -1) {  // se ricevo DOPPIO TAGLIO
    draw(4, lcd);
    tone(buzzer, 3000, 200);
    if (RxData.indexOf("4015") != -1) {
      decodestringone(RxData, 1);
      stringonerosso = RxData;
      datirosso = 1;
      punteggiorosso = "200";
    }
    if (RxData.indexOf("4025") != -1) {
      decodestringone(RxData, 2);
      stringoneverde = RxData;
      dativerde = 1;
      punteggioverde = "200";
    }
    if (RxData.indexOf("4035") != -1) {
      decodestringone(RxData, 3);
      stringoneblu = RxData;
      datiblu = 1;
      punteggioblu = "200";
    }
    RxData = "";
  }
  if (RxData.indexOf("6514") != -1 || RxData.indexOf("6524") != -1 || RxData.indexOf("6534") != -1) {  // se ricevo TIMEOUT
    draw(4, lcd);
    tone(buzzer, 3000, 200);
    if (RxData.indexOf("6514") != -1) {
      decodestringone(RxData, 1);
      stringonerosso = RxData;
      datirosso = 1;
      punteggiorosso = "200";
    }
    if (RxData.indexOf("6524") != -1) {
      decodestringone(RxData, 2);
      stringoneverde = RxData;
      dativerde = 1;
      punteggioverde = "200";
    }
    if (RxData.indexOf("6534") != -1) {
      decodestringone(RxData, 3);
      stringoneblu = RxData;
      datiblu = 1;
      punteggioblu = "200";
    }
    RxData = "";
  }
  if ( dativerde == 1 &&  datirosso == 1 &&  datiblu == 1 ) { // una volta che ho ricevuto tutto stampo e mando al PC
    stampatotali( gara, manche_rx, round_rx, nome_rosso, cognome_rosso, nome_verde, cognome_verde, nome_blu, cognome_blu);
    datirosso = 0;
    dativerde = 0;
    datiblu = 0;
    Serial.println( stringonerosso);
    Serial.println( stringoneverde);
    Serial.println( stringoneblu);
  }

  // 805,Q500,2,3,Matteo,Giacomo,Francesco,Luca,Paolo,Pippo
  // lettura seriale
  if (statusPC == true) {
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      my_str[pos] = incomingByte;
      pos++;
      if (incomingByte == 10) {  //10 is line feed character
        splitCommaSeparated();
        //clear the string for next time
        for (int i = 0; i <= len - 1; i++) {
          my_str[i] = 0;
        }
        pos = 0;
      }
    }
  }

  if (newDataPC == true) {

    String stringanomi = "";
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

    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, stringanomi);

    newDataPC = false;
  }

  // If something available
  if (e22ttl.available() > 1) {
    // read the  message
    ResponseContainer rc = e22ttl.receiveMessage();
    RxData = rc.data;
    Serial.println(RxData);

  }
}
