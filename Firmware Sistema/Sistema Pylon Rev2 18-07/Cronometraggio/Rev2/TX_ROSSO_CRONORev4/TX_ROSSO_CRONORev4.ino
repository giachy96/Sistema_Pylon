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
#include <U8x8lib.h>
#include "Arduino.h"
#include "LoRa_E22.h"
#include "lcdcases.h"
#include "timestructure.h"

int pulsante = 9;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int altupdatelcd = 0;
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
boolean newData = false ;
int Press = 0;
String State ;
long interval = 20000;  // constants won't change:

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);



void setup() {
  pinMode(pulsante, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(pinbatt, INPUT);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();  // Startup all pins and UART

  u8x8.begin();
  u8x8.setPowerSave(0);
}

void loop() {



  // fase di avvio
  while (bootup < 5) {
    previousMillis = currentMillis;
    // picture loop
    draw(0, u8x8);

    bootup++;
  }
  // fine fase di avvio

  while (!bootup < 5) {
    currentMillis = millis();

    // ----------------------------------- controllo batteria
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer, HIGH);
    } else {
      digitalWrite(buzzer, LOW);
    }

    //    // allarme tensione
    if (readvoltage(pinbatt) < 2.9) {
      tone(buzzer, 5000, 200);
    }

    if (currentMillis - previousMillis >= interval  && altupdatelcd == 0) {

      previousMillis = currentMillis;
      altupdatelcd = 0;
      // picture loop
      draw(0, u8x8);
    }

    if (currentMillis - previousMillis >= 10000  && altupdatelcd == 2) {

      altupdatelcd = 0;
    }

    if (e22ttl.available() > 1) {
      ResponseContainer rc = e22ttl.receiveMessage();  // Receive message
      if (rc.status.code != 1) {                       // If there is some problem
        rc.status.getResponseDescription();            //Get report
        Timesend = millis();
      } else {            //If there isn't any problem we're going to receive press
        State = rc.data;  //Assign incoming data on TxData variable
        newData = true;
      }
    }
    CurrentPress = millis();

    if (State == "200" ) {  // SSHOW
      

      if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress) {
        ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, "220");
        tone(buzzer, 4000, 200);
        Lastpress = millis();
      }
      if (newData == true) {
        tone(buzzer, 4000, 200);
        draw(1, u8x8);
        altupdatelcd = 1;
        newData = false;
      }
    }

    if (State == "300") {  // START
       if (newData == true) {
        tone(buzzer, 4000, 200);
        newData = false;
      }
     
      altupdatelcd = 1;
      if (lapcounter == -2) {
        draw(2 , u8x8);
        lapcounter = -1;
      }
      if (lapcounter == -1 && digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress) {
        tempo_base = millis();
      }
      if (lapcounter == 10) {
        float tot =  CalcoloTempo(tempo_flt);
        tempo_flt[11] = tot;
        tempo_parziale[11] = (int)(tot * 100); // per inserire il totale sulla stringa con virgole
        float ultimo_tempo = tempo_flt[11];
        char bu[10];
        dtostrf(ultimo_tempo, 4, 3, bu);  //4 is mininum width, 6 is precision
        String msg = "523,";
        msg.concat("11");
        msg.concat(",");
        msg.concat(bu);
        ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, msg);
        State = "";
        draw(4 , u8x8);
        lapcounter = -2;
        altupdatelcd = 2;
        previousMillis = millis();

      }
      if (digitalRead(pulsante) == LOW && (CurrentPress - Lastpress) >= Delaypress) {

        lapcounter = lapcounter + 1;
        Catturatempo(poitem, poiparz, lapcounter, tempo_base);  //balza l'indice a 1Lastpress = millis()
        float ultimo_tempo = tempo_flt[lapcounter];
        char bu[10];
        dtostrf(ultimo_tempo, 4, 3, bu);  //4 is mininum width, 6 is precision
        String msg = "423,";
        msg.concat(lapcounter);
        msg.concat(",");
        msg.concat(bu);
        ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 6, msg);
        tone(buzzer, 4000, 200);
        Lastpress = millis();
        draw(3 , u8x8);
        altupdatelcd = 1;

      }
    }

    if (State == "500" ) {  // STOP
      if (newData == true) {
        tone(buzzer, 4000, 200);
        draw(5, u8x8);
        altupdatelcd = 1;
        lapcounter = -2;
        newData = false;
      }
    }





//    CurrentPress = millis();
//    if (Press == 1 && (CurrentPress - Timesend) >= Delaysend) {
//      String msg = "312,";
//      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 7, msg);
//      Serial.println("invio");
//      Press = 0;
//    }
  }
}
