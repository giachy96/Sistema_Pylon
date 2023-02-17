/*
   LoRa E22
   Write on serial to transfer a message to other device
   https://www.mischianti.org

   E22        ----- Arduino UNO
   M0         ----- GND
   M1         ----- GND
   TX         ----- PIN 2 (PullUP)
   RX         ----- PIN 3 (PullUP & Voltage divider)
   AUX        ----- Not connected
   VCC        ----- 3.3v/5v
   GND        ----- GND

*/
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include "U8x8lib.h"
#include "Arduino.h"
#include "lcdcases.h"

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


U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);

int pulsante = 9;
int taglio = 10;
int buzzer =  5;
int pinbatt = A0;
int bootup = 1;
int dcase = 0;
int altupdatelcd = 0;

unsigned long previousMillis = 0;  // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long CurrentPress = 0;
unsigned long Lastpress = 0;
unsigned long LastBuzzer = 0;
int changeState = 0;

// Inizio configurazione Telecomando
String Race = "3000";
String Show = "2000";
String End = "5000";
String Stop = "6000";
String Startup = "1000";
String State = Startup;          //Startup state
unsigned long interval = 60000;  // constants won't change:
unsigned long Delaypress = 3500;
unsigned long Delaysend = 200;
String PressShow = "2110";
String PressRace = "3112";
String PressCut = "3111";
String StartRace = "3001";
String end10lap = "5514";
String DoubleCut = "4015";
String StopTime = "6514";
int ntagli = 0;
int Key = 0;
int Add = 0;
int Chan = 15;
//Fine configurazione Telecomando

void setup() {
  pinMode(pulsante, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(pinbatt, INPUT);
  pinMode(taglio, INPUT_PULLUP);
  Serial.begin(9600);
  delay(500);
  u8x8.begin();
  u8x8.setPowerSave(0);

  //Inizializzo la radio 2.4GHZ
  radio.begin();
  /*
     La radio può lavorare a diverse potenze: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH e RF24_PA_MAX
     Che corrispondono a: -18dBm, -12dBm,-6dBM, e 0dBm
  */
  //Setto la potenza della radio a LOW
  radio.setPALevel(RF24_PA_LOW);

  //Apro un canale in lettura sull'indirizzo specificato
  radio.openReadingPipe(1, indirizzo[1]);

  //Apro un canale in scrittura sull'indirizzo specificato
  radio.openWritingPipe(indirizzo[0]);

  //Metto la radio in ascolto
  radio.startListening();

  Serial.println("start");  //debug
  Serial.println(State);    //debug
  tone(buzzer, 1000, 200);
}

void loop() {

  currentMillis = millis();

  while (bootup < 2) {
    previousMillis = currentMillis;
    Serial.println("Bootup Case");  //debug
    draw(0, u8x8);
    bootup++;
  }

  while (bootup >= 2) {
    float voltage = readvoltage(pinbatt);
    if (voltage < 3.2) {
      digitalWrite(buzzer, HIGH);
    }
    currentMillis = millis();

    if (changeState == 1 && State == Show) {  // Casistica per display, se siamo in Show, metti la schermata 1 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case1");  //debug
      draw(1, u8x8);
      changeState = 0;
      ntagli = 0;
    }
    if (changeState == 1 && State == Race) {  // Casistica per display, se siamo in Race, metti la schermata 2 e aggiorna solo quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case2");  //debug
      draw(2, u8x8);
      changeState = 0;
      ntagli = 0;
    }
    if (changeState == 1 && State == StartRace) {  // Casistica per display, se siamo in Race, metti la schermata 2 e aggiorna solo quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case2.1");  //debug
      draw(3, u8x8);
      changeState = 0;
      tone(buzzer, 1000, 200);
    }
    if (changeState == 1 && State == end10lap) {  // Casistica per display, se siamo in End, metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      draw(6, u8x8);
      Serial.println("Case6");  //debug
      changeState = 0;
      previousMillis = currentMillis;
    }

    if (changeState == 1 && State == Stop) {  // Casistica per display, se siamo in End, metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      draw(5, u8x8);
      Serial.println("Case5");  //debug
      changeState = 0;
      ntagli = 0;
      previousMillis = currentMillis;
    }
    if (changeState == 1 && State == DoubleCut) {  // Casistica per display, se siamo in End, metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      draw(7, u8x8);
      Serial.println("Case7");  //debug
      changeState = 0;
      ntagli = 0;
      previousMillis = currentMillis;
    }
    if (changeState == 1 && State == StopTime) {  // Casistica per display, se siamo in End, metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      draw(8, u8x8);
      Serial.println("Case8");  //debug
      changeState = 0;
      ntagli = 0;
      previousMillis = currentMillis;
    }
    if (((currentMillis - previousMillis) >= interval)  && (State == Startup || State == end10lap || State == Stop || State == DoubleCut ||  State == StopTime )) {  // Casistica per display, se non siamo in Show, Race o End, metti la schermata 0 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case0");  //debug
      draw(0, u8x8);
      changeState = 0;
    }

    if (radio.available()) {  // se ricevo qualcosa da 2.4GHz

      //Creo una variabile di appoggio
      char buff[50]; // this must match dataToSend in the TX
      //Leggo i dati sul buffer e li salvo nella variabile di appoggio
      radio.read( &buff, sizeof(buff) );
      //Invio al monitor seriale il valore appena letto
      State = String(buff);
      Serial.println("ricevo qualcosa");               //debug
      Serial.println(State); //debug
      changeState = 1;                                 //attivo il flag di cambiostato per i display
      if (millis() - LastBuzzer > 1000) {
        tone(buzzer, 1000, 200);
        LastBuzzer = millis();
      }
      Serial.println("flag");       //debug
      Serial.println(changeState);  //debug

    }

    CurrentPress = millis();
    if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == Show) {  //se il pulsante è premuto, sono in show e la sicurezza sulle pressioni ripetute è passata
      Lastpress = millis();                                                                           //memorizzo l'ultima pressione
      Serial.println("ricevopressione");
      sendStringMessageRF24( PressShow);  // invio il codice di show
      tone(buzzer, 1000, 200);
      Serial.println("invio");                         //debug
      Serial.println(currentMillis - previousMillis);  //debug
    }
    CurrentPress = millis();
    if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == StartRace) {  //se il pulsante del semaforo è premuto, sono in race e la sicurezza sulle pressioni ripetute è passata
      Lastpress = millis();
      sendStringMessageRF24( PressRace);  //invio il codice race
      tone(buzzer, 1000, 200);
      Serial.println("invio");  //debug
    }
    CurrentPress = millis();
    if (digitalRead(taglio) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == StartRace) {  //se il pulsante del taglio è premuto, sono in race e non sono stati premuti altri tasti prima di delaypress
      Lastpress = millis();
      sendStringMessageRF24(PressCut);  //invio il codice di taglio
      ntagli = ntagli + 1;
      tone(buzzer, 1000, 1000);
      Serial.println("invio");  //debug
      changeState = 1;
    }
  }
}

void sendStringMessageRF24(String valore  ) {
  char charBuf[50];
  valore.toCharArray(charBuf, 50);
  radio.stopListening();  // put radio in TX mode
  //Invio il valore per radio
  radio.write(&charBuf, sizeof(charBuf));
  //radio.write(valore.c_str(), valore.length());
  radio.startListening();  // put radio in RX mode

}
