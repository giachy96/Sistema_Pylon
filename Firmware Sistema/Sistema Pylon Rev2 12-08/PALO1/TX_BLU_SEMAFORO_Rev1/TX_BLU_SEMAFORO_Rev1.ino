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
#include "U8x8lib.h"
#include "Arduino.h"
#include "LoRa_E22.h"
#include "lcdcases.h"
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
int pulsante = 9;
int taglio = 10;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int dcase = 0;
int altupdatelcd = 0;
//char t[5] = { '3', ',', '3', '3',  '\0' };
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
unsigned long previousMillis = 0;        // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long CurrentPress = 0;
unsigned long Lastpress = 0;
int changeState = 0;

// Inizio configurazione Telecomando
String Race = "3000";
String Show = "2000";
String End = "5000";
String Stop = "6000";
String Startup = "1000";
String State = Startup; //Startup state
unsigned long interval = 10000;// constants won't change:
unsigned long Delaypress = 2500;
unsigned long Delaysend = 200;
String PressShow = "2130";
String PressRace = "3132";
String PressCut = "3131";
String StartRace = "3001";
String end10lap = "5534";
int ntagli = 0;
int Key = 0;
int Add = 0;
int Chan = 25;
//Fine configurazione Telecomando

void setup() {
  pinMode(pulsante , INPUT_PULLUP);
  pinMode (buzzer , OUTPUT);
  pinMode (pinbatt , INPUT);
  pinMode (taglio , INPUT_PULLUP);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();  // Startup all pins and UART
  u8x8.begin();
  u8x8.setPowerSave(0);
  Serial.println("start"); //debug
  Serial.println(State);//debug
}

void loop() {

  currentMillis = millis();

  while (bootup < 2) {
    previousMillis = currentMillis;
    Serial.println("Bootup Case");//debug
    draw(0, u8x8);
    bootup++;
  }

  while (bootup >= 2) {
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer , HIGH);
    }
    currentMillis = millis();
    if ((((currentMillis - previousMillis) >= interval) || changeState == 1) && State == Startup) { // Casistica per display, se non siamo in Show, Race o End, metti la schermata 0 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case0");//debug
      draw(0, u8x8);
      changeState = 0;

    }
    if ( changeState == 1 && State == Show) { // Casistica per display, se siamo in Show, metti la schermata 1 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case1");//debug
      draw(1, u8x8);
      changeState = 0;
      ntagli = 0;

    }
    if (changeState == 1 && State == Race) { // Casistica per display, se siamo in Race, metti la schermata 2 e aggiorna solo quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case2");//debug
      draw(2, u8x8);
      changeState = 0;

    }
    if (changeState == 1 && State == StartRace) { // Casistica per display, se siamo in Race, metti la schermata 2 e aggiorna solo quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case2.1");//debug
      draw(3, u8x8);
      changeState = 0;
      tone(buzzer, 1000, 200);
    }
    if ( changeState == 1 && State == end10lap) { // Casistica per display, se siamo in End, metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      draw(6, u8x8);
      Serial.println("Case6");//debug
      changeState = 0;

    }

    if ( changeState == 1 && State == Stop) { // Casistica per display, se siamo in End, metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      draw(5, u8x8);
      Serial.println("Case5");//debug
      changeState = 0;

    }
    if (e22ttl.available() > 1) { //se il lora riceve qualcosa
      Serial.println("ricevo qualcosa");//debug
      ResponseContainer rc = e22ttl.receiveMessage();// Receive message
      State = rc.data; //Assign incoming data on TxData variable
      Serial.println("Nuovo stato");//debug
      Serial.println(State);//debug
      changeState = 1; //attivo il flag di cambiostato per i display
      tone(buzzer, 1000, 200);
      Serial.println("flag");//debug
      Serial.println(changeState);//debug

    }
    CurrentPress = millis();
    if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == Show) { //se il pulsante è premuto, sono in show e la sicurezza sulle pressioni ripetute è passata
      Lastpress = millis(); //memorizzo l'ultima pressione
      Serial.println("ricevopressione");
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, PressShow); // invio il codice di show
      tone(buzzer, 1000, 200);
      Serial.println("invio"); //debug
      Serial.println(currentMillis - previousMillis);//debug
    }
    CurrentPress = millis();
    if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == StartRace) { //se il pulsante del semaforo è premuto, sono in race e la sicurezza sulle pressioni ripetute è passata
      Lastpress = millis();
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, PressRace); //invio il codice race
      tone(buzzer, 1000, 200);
      Serial.println("invio");//debug
    }
    CurrentPress = millis();
    if (digitalRead(taglio) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == StartRace) { //se il pulsante del taglio è premuto, sono in race e non sono stati premuti altri tasti prima di delaypress
      Lastpress = millis();
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, PressCut); //invio il codice di taglio
      ntagli = ntagli + 1;
      tone(buzzer, 1000, 1000);
      Serial.println("invio");//debug
      changeState = 1;
    }
  }
}
