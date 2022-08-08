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
int taglio = 6;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int dcase = 0;
int altupdatelcd = 0;
char t[5] = { '3', ',', '3', '3',  '\0' };
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
unsigned long previousMillis = 0;        // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long CurrentPress=0;
unsigned long Lastpress=0;
unsigned long Delaypress=2500;
unsigned long Delaysend=200;
int changeState=0;
String Race="<300>";
String Show="<200>";
String End="<600>";
String Startup="<100>";
String State=Startup; //Startup state
unsigned long interval = 10000;// constants won't change:

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
  Serial.println("start");
  Serial.println(State);
}

void loop() {

 currentMillis = millis();

  while (bootup < 2) {
    previousMillis = currentMillis;
    Serial.println("Bootup Case");
    draw(0, u8x8);
    bootup++;
  }

  while (bootup >= 2) {
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer , HIGH);
    }
    currentMillis = millis();
    if ((((currentMillis - previousMillis)>=interval) || changeState==1) && (State !=Show || State != Race || State != End)) {
      previousMillis = currentMillis;
      Serial.println("Case0");
      draw(0, u8x8);
      changeState=0;
    }
if ((((currentMillis - previousMillis)>=interval) || changeState==1) && State==Show) {
      previousMillis = currentMillis;
      Serial.println("Case1");
      draw(1, u8x8);
      changeState=0;
    }
    if (changeState==1 && State==Race) {
      previousMillis = currentMillis;
      Serial.println("Case2");
      draw(2, u8x8);
      changeState=0;
    }
    if ((((currentMillis - previousMillis)>=interval) || changeState==1) && State==End) {
      previousMillis = currentMillis;
      draw(6, u8x8);
      Serial.println("Case6");
      changeState=0;
    }
    if (e22ttl.available()>1){
      Serial.println("ricevo qualcosa");
      ResponseContainer rc = e22ttl.receiveMessage();// Receive message
      if (rc.status.code!=1){ // If there is some problem
        rc.status.getResponseDescription(); //Get report
        }
      else { //If there isn't any problem we're going to receive press 
        State=rc.data; //Assign incoming data on TxData variable
        Serial.println("Nuovo stato");
        Serial.print(State);
        changeState=1;
      }
    } 
    CurrentPress=millis();
      if (digitalRead(pulsante) == LOW && (CurrentPress-Lastpress)>=Delaypress && State==Show) {
      Lastpress=millis();
      tone(buzzer, 1000, 200);
      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 3,"<212>");
      Serial.println("invio");
      Serial.println(currentMillis - previousMillis);
    }  
      CurrentPress=millis();
      if (digitalRead(pulsante) == LOW && (CurrentPress-Lastpress)>=Delaypress && State==Race) {
      Lastpress=millis();
      tone(buzzer, 1000, 200);
      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 3,"<312>");
      Serial.println("invio");
    }  
    CurrentPress=millis();
      if (digitalRead(taglio) == LOW && (CurrentPress-Lastpress)>=Delaypress && State==Race) {
      Lastpress=millis();
      tone(buzzer, 1000, 1000);
      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 3,"<311>");
      Serial.println("invio");
    }  
}
}
