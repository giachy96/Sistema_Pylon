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
#include "Arduino.h"
#include "LoRa_E22.h"
#include "U8glib.h"
#include "lcdcases.h"
#include "timestructure.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
int pulsante = 9;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int aggiornamentolcd = 0;
int dcase = 0;
char t[5] = { '3', ',', '3', '3', '\0' };

// tempi
int lapcounter = -2;
unsigned long tempo_base;
unsigned long tempo_totale;
long tempo_parziale[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
long* poiparz = &tempo_parziale[0];
float tempo_flt[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int ultimo_tempo;
int ultimo_inviato = 0;
float* poitem = &tempo_flt[0];

LoRa_E22 e22ttl(2, 3);             // Arduino RX --> e22 TX - Arduino TX --> e22 RX
unsigned long previousMillis = 0;  // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long Timesend = 0;
unsigned long CurrentPress = 0;
unsigned long Lastpress = 0;
unsigned long Delaypress = 2000;
unsigned long Delaysend = 200;

int Press = 0;
String State ;
long interval = 20000;  // constants won't change:





void setup() {
  pinMode(pulsante, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(pinbatt, INPUT);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();  // Startup all pins and UART
  u8g.begin();
}

void loop() {

 

  // fase di avvio
  while (bootup < 5) {
    previousMillis = currentMillis;
    // picture loop
    u8g.firstPage();
    do {
      draw(0, u8g);
    } while (u8g.nextPage());
    bootup++;
  }
  // fine fase di avvio

  while (!bootup < 10) {
     currentMillis = millis();

    // ----------------------------------- controllo batteria
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer, HIGH);
    } else {
      digitalWrite(buzzer, LOW);
    }

    // allarme tensione
    if (readvoltage(pinbatt) < 2.9) {
      tone(buzzer, 5000, 200);
    }

    if (currentMillis - previousMillis >= interval || aggiornamentolcd == 1) {

      previousMillis = currentMillis;
      aggiornamentolcd = 0;
      // picture loop
      u8g.firstPage();
      do {
        draw(dcase, u8g);

      } while (u8g.nextPage());
    }

    if (e22ttl.available() > 1) {
      ResponseContainer rc = e22ttl.receiveMessage();  // Receive message
      if (rc.status.code != 1) {                       // If there is some problem
        rc.status.getResponseDescription();            //Get report
        Timesend = millis();
      } else {            //If there isn't any problem we're going to receive press
        State = rc.data;  //Assign incoming data on TxData variable
      }
    }
    CurrentPress = millis();

    if (State == "200") {  // SSHOW

      if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress) {
        ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, "220");
        tone(buzzer, 4000, 200);
        Lastpress = millis();
      }
      dcase = 1;
      aggiornamentolcd = 1;
    }

    if (State == "300") {  // START
      if (lapcounter == -2) {
        dcase = 2;
        lapcounter = -1;
      }
      if (lapcounter == -1 && digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress) {
        tempo_base = millis();
        dcase = 3;
      }
      if (lapcounter == 10) {
        float tot =  CalcoloTempo(tempo_flt);
        tempo_flt[11] = tot;
        tempo_parziale[11] = (int)(tot * 100); // per inserire il totale sulla stringa con virgole
        float ultimo_tempo = tempo_flt[11];
        String msg = "523,";
        msg.concat("11");
        msg.concat(",");
        msg.concat(String(ultimo_tempo));
        ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, msg);
        State = "400";
        lapcounter = -2;
         dcase = 4;
      }
      if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress) {
        Serial.println("SONODENTROIL DELAY");
        lapcounter = lapcounter + 1;
        Catturatempo(poitem, poiparz, lapcounter, tempo_base);  //balza l'indice a 1Lastpress = millis()
        float ultimo_tempo = tempo_flt[lapcounter];
        String msg = "423,";
        msg.concat(lapcounter);
        msg.concat(",");
        msg.concat(String(ultimo_tempo));
        ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, msg);
        tone(buzzer, 4000, 200);
        Lastpress = millis();
        dcase = 3;
      }


      aggiornamentolcd = 1;
    }





    CurrentPress = millis();
    if (Press == 1 && (CurrentPress - Timesend) >= Delaysend) {
      String msg = "312,";
      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, msg);
      Serial.println("invio");
      Press = 0;
    }
  }
}
