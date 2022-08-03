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
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
int pulsante = 9;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
float tensione_float = 0;
char vout[8]; // Buffer big enough for 7-character float
char t[5] = { '3', ',', '3', '3',  '\0' };
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
unsigned long previousMillis = 0;        // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long Timesend=0;
unsigned long CurrentPress=0;
unsigned long Lastpress=0;
unsigned long CurrentLow = 0;
unsigned long Delaypress=2500;
unsigned long Delaysend=200;
unsigned long Delaybuzzer=300;
int Press=0;
String Race="<300>";
String Show="<200>";
String Startup="<100>";
String State=Show; //Startup state
long interval = 5000;// constants won't change:

float readvoltage (int pin) {
  float tensione;
  tensione = analogRead(pin);
  tensione = (tensione / 1023) * 4.35;
  return tensione;
}

void draw(String Statev) {
  if (Statev==Race){
    Statev="Race";
    }
  else if (Statev==Show){
    Statev="Show";
    }
  else {
    Statev="Other";
    }
  tensione_float = readvoltage(pinbatt);
  dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!
  u8g.setFont(u8g_font_9x15);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
  u8g.drawFrame(0, 0, 125, 64);
  u8g.drawStr(0, 5 , " Voltaggio");
  u8g.drawStr(0, 18 , " Batteria");
  u8g.drawStr(60, 31 , " V");
  u8g.drawStr(10, 31, vout );
  u8g.drawStr(10, 44, Statev.c_str());
}

void setup() {
  pinMode(pulsante , INPUT_PULLUP);
  pinMode (buzzer , OUTPUT);
  pinMode (pinbatt , INPUT);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();  // Startup all pins and UART
  u8g.begin();
  Serial.println("start");
  Serial.println(State);
}

void loop() {

 unsigned long currentMillis = millis();

  while (bootup < 10) {
       previousMillis = currentMillis; // save the last time you blinked the LED
        u8g.firstPage();// picture loop
    do {
      draw(State);
    } while ( u8g.nextPage() );
    bootup++;
  }
  while (!bootup < 10) {
    
    // ----------------------------------- controllo batteria 
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer , HIGH);
    // } else {
    //   digitalWrite(buzzer, LOW);
    //   Serial.println("Buzzer spento");
    }
    currentMillis=millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; // save the last time you blinked the LED
            u8g.firstPage();// picture loop
      do {
        draw(State);
      } while ( u8g.nextPage() );
    }
  
    if (e22ttl.available()>1){
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
      if (rc.status.code!=1){ // If there is some problem
      rc.status.getResponseDescription(); //Get report
      Timesend=millis();
      }
      else{ //If there isn't any problem we're going to receive press 
      State=rc.data; //Assign incoming data on TxData variable
      Serial.println("Nuovo stato");
      Serial.print(State);
      }
    } 
    CurrentPress=millis();
    if (digitalRead(pulsante) == LOW && (State==Show || State==Race) && (CurrentPress-Lastpress)>=Delaypress) {
      Lastpress=millis();
      Press=1;
      digitalWrite(buzzer , HIGH);
    }
    CurrentPress=millis();
    if (Press==1 && ((CurrentPress-Timesend)>=Delaysend)){
      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 3,"312");
      Serial.println("invio");
      Press=0;
    }  
    CurrentLow=millis();
    if ((CurrentLow-Lastpress)>=Delaybuzzer){
      digitalWrite(buzzer , LOW);
    }
  }
}
