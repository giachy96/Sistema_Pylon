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
#include "timestructure.h"

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);
int pulsante = 9;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int dcase = 0;

LoRa_E22 e22ttl(2, 3);             // Arduino RX --> e22 TX - Arduino TX --> e22 RX
unsigned long previousMillis = 0;  // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long CurrentPress = 0;
unsigned long Lastpress = 0;
int changeState = 0;

// tempi
int lapcounter = -1;
unsigned long tempo_base;
unsigned long tempo_totale;
long tempo_parziale[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
long* poiparz = &tempo_parziale[0];
float tempo_flt[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int ultimo_tempo;
int ultimo_inviato = 0;
float* poitem = &tempo_flt[0];



// Inizio configurazione Telecomando
String Race = "3000";
String Show = "2000";
String End = "5000";
String Stop = "6000";
String Startup = "1000";
String State = Startup;          //Startup state
unsigned long interval = 30000;  // constants won't change:
unsigned long Delaypress = 2000;
unsigned long DelayStop = 30000;
String PressShow = "2530";
String PressTime = "4534";
String StartRace = "3533";
String end10lap = "5534";
String DoubleCut = "4035";
String StopTime = "6534";
int Key = 0;
int Add = 0;
int Chan = 40;
//Fine configurazione Telecomando

void setup() {
  pinMode(pulsante, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(pinbatt, INPUT);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();  // Startup all pins and UART
  u8x8.begin();
  u8x8.setPowerSave(0);
  Serial.println("start");  //debug
  Serial.println(State);    //debug
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
    if (voltage < 2.9) {
      digitalWrite(buzzer, HIGH);
    }
    currentMillis = millis();



    if (((currentMillis - previousMillis) >= interval)  && (State == Startup || State == end10lap || State == Stop || State == DoubleCut ||  State == StopTime )) {
      previousMillis = currentMillis;
      Serial.println("Case0");  //debug
      draw(0, u8x8);
      changeState = 0;
    }
    if (changeState == 1 && State == Show) {  // Casistica per display, se siamo in Show, metti la schermata 1
      Serial.println("Case1");  //debug
      draw(1, u8x8);
      tempo_base = 0;
      tempo_totale = 0;
      memset(tempo_parziale, 0, sizeof(tempo_parziale));
      memset(tempo_flt, 0, sizeof(tempo_flt));
      tone(buzzer, 2000, 200);
      ultimo_tempo = 0;
      ultimo_inviato = 0;
      lapcounter = -1;
      changeState = 0;
    }
    if (changeState == 1 && State == StartRace ) {  // Casistica per display, se siamo in StartRace
      Serial.println("Case2.1");  //debug
      tempo_base = millis();
      lapcounter = lapcounter + 1;
      Catturatempo(poitem, poiparz, lapcounter, tempo_base);  //balza l'indice a 1Lastpress = millis()
      float ultimo_tempo = tempo_flt[lapcounter];
      char bu[10];
      dtostrf(ultimo_tempo, 4, 3, bu);  //4 is mininum width, 6 is precision
      String msg = PressTime;
      msg.concat(",");
      msg.concat(lapcounter);
      msg.concat(",");
      msg.concat(bu);
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, msg);
      tone(buzzer, 2000, 200);
      draw(3, u8x8);
      changeState = 0;
      tone(buzzer, 2000, 200);
      Lastpress = millis();
      Serial.println("Dentro lo Start RAce");
      Serial.println(Lastpress);
    }
// NON SPOSTARE ASSOLUTAMENTE QUESTA PARTE ALTRIMENTI NON FUNZIONA NON SI SA PERCHé
    CurrentPress = millis();
    if ((CurrentPress - Lastpress) >= DelayStop && State == StartRace && lapcounter < 10 ) { //se sono passati DelayStop secondi è non ho avuto pressioni
      Serial.println("Differenza millis");  //debug
      Serial.println((CurrentPress - Lastpress));  //debug
      Lastpress = millis();
      previousMillis = currentMillis;
      float tot = CalcoloTempo(tempo_flt);
      tempo_flt[11] = tot;
      tempo_parziale[11] = (int)(tot * 100);  // per inserire il totale sulla stringa con virgole
      float ultimo_tempo = tempo_flt[11];
      char bu[10];
      dtostrf(ultimo_tempo, 4, 3, bu);  //4 is mininum width, 6 is precision
      String msg = StopTime;
      msg.concat(",");
      msg.concat("11");
      msg.concat(",");
      msg.concat(bu);
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, msg);
      draw(8, u8x8);
      tempo_base = 0;
      tempo_totale = 0;
      memset(tempo_parziale, 0, sizeof(tempo_parziale));
      memset(tempo_flt, 0, sizeof(tempo_flt));
      ultimo_tempo = 0;
      ultimo_inviato = 0;
      lapcounter = -1;
      changeState = 0;
      tone(buzzer, 2000, 200);
      lapcounter = -1;
      State = StopTime;

      Serial.println("fuori tempo max");  //debug

    }
// FINE PARTE DA NON SPOSTARE
    if (changeState == 1 && State == Race) {  // Casistica per display, se siamo in Race, metti la schermata 2
      Serial.println("Case2");  //debug
      draw(2, u8x8);
      tempo_base = 0;
      tempo_totale = 0;
      memset(tempo_parziale, 0, sizeof(tempo_parziale));
      memset(tempo_flt, 0, sizeof(tempo_flt));
      tone(buzzer, 2000, 200);
      ultimo_tempo = 0;
      ultimo_inviato = 0;
      lapcounter = -1;
      changeState = 0;
    }


    if (changeState == 1 && State == Stop) {  // Casistica per display, se siamo in STOP
      previousMillis = currentMillis;
      draw(5, u8x8);
      tempo_base = 0;
      tempo_totale = 0;
      memset(tempo_parziale, 0, sizeof(tempo_parziale));
      memset(tempo_flt, 0, sizeof(tempo_flt));
      tone(buzzer, 2000, 200);
      ultimo_tempo = 0;
      ultimo_inviato = 0;
      lapcounter = -1;
      Serial.println("Case5");  //debug
      changeState = 0;
    }
    if (changeState == 1 && State == DoubleCut) {  // Casistica per display, se siamo in DOPPIO TAGLIO
      previousMillis = currentMillis;
      draw(7, u8x8);
      tone(buzzer, 2000, 200);
      lapcounter = -1;
      Serial.println("Case7");  //debug
      changeState = 0;
    }

    if (e22ttl.available() > 1) {                      //se il lora riceve qualcosa
      Serial.println("ricevo qualcosa");               //debug
      ResponseContainer rc = e22ttl.receiveMessage();  // Receive message
      State = rc.data;                                 //Assign incoming data on TxData variable
      Serial.println("Nuovo stato");                   //debug
      Serial.println(State);                           //debug
      changeState = 1;                                 //attivo il flag di cambiostato per i display
      Serial.println("flag");       //debug
      Serial.println(changeState);  //debug
    }
    CurrentPress = millis();
    if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == Show) {  //se il pulsante è premuto, sono in show e la sicurezza sulle pressioni ripetute è passata
      Lastpress = millis();                                                                           //memorizzo l'ultima pressione
      Serial.println("ricevopressione");
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, PressShow);  // invio il codice di show
      tone(buzzer, 2000, 200);
    }
    CurrentPress = millis();
    if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress && State == StartRace && lapcounter < 10) {  //se il pulsante del semaforo è premuto, sono in race e la sicurezza sulle pressioni ripetute è passata
      Lastpress = millis();
      lapcounter = lapcounter + 1;
      Catturatempo(poitem, poiparz, lapcounter, tempo_base);  //balza l'indice a 1Lastpress = millis()
      float ultimo_tempo = tempo_flt[lapcounter];
      char bu[10];
      dtostrf(ultimo_tempo, 4, 3, bu);  //4 is mininum width, 6 is precision
      String msg = PressTime;
      msg.concat(",");
      msg.concat(lapcounter);
      msg.concat(",");
      msg.concat(bu);
      Lastpress = millis();
      draw(3, u8x8);
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, msg);
      tone(buzzer, 2000, 200);
      Serial.println("invio");  //debug
    }
    CurrentPress = millis();

    if (lapcounter == 10) {  //se il pulsante del semaforo è premuto, sono in race e la sicurezza sulle pressioni ripetute è passata
      float tot = CalcoloTempo(tempo_flt);
      tempo_flt[11] = tot;
      tempo_parziale[11] = (int)(tot * 100);  // per inserire il totale sulla stringa con virgole
      float ultimo_tempo = tempo_flt[11];
      char bu[10];
      dtostrf(ultimo_tempo, 4, 3, bu);  //4 is mininum width, 6 is precision
      String msg = end10lap;
      msg.concat(",");
      msg.concat("11");
      msg.concat(",");
      msg.concat(bu);
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, msg);
      State = end10lap;
      draw(4, u8x8);
      lapcounter = -1;
      previousMillis = millis();
    }
  }
}
