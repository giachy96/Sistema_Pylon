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

// constants won't change:
long interval = 20000;

float fmap(float x, float out_max , float out_min , float in_max , float in_min)
{
  float k1 = (out_max - out_min) / (in_max - in_min);
  float k2 = out_min - (in_min * k1);
  return x * k1 + k2;
}


float readvoltage (int pin) {
  float tensione;
  tensione = analogRead(pin);
  tensione = (tensione / 1023) * 4.35;
  return tensione;
}


void draw(void) {

  tensione_float = readvoltage(pinbatt);
  dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!

  //tensione_batt = char(tensione_float );// using a float and the decimal places
  u8g.setFont(u8g_font_9x15);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
  u8g.drawFrame(0, 0, 125, 64);
  u8g.drawStr(0, 5 , " Voltaggio");
  u8g.drawStr(0, 18 , " Batteria");
  u8g.drawStr(60, 40 , " V");
  u8g.drawStr(10, 40, vout );
  //Serial.println(vout);
}


void setup() {
  pinMode(pulsante , INPUT_PULLUP);
  pinMode (buzzer , OUTPUT);
  pinMode (pinbatt , INPUT);
  Serial.begin(9600);
  delay(500);


  // Startup all pins and UART
  e22ttl.begin();
  u8g.begin();

  //  // Send message
  //  ResponseStatus rs = e22ttl.sendFixedMessage(0,1,23,"Hello, world?");
  //  // Check If there is some problem of succesfully send
  //  Serial.println(rs.getResponseDescription());
}

void loop() {

  unsigned long currentMillis = millis();

  while (bootup < 10) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // picture loop
    u8g.firstPage();
    do {
      draw();
    } while ( u8g.nextPage() );
    bootup++;
  }
  while (!bootup < 10) {
    
// ----------------------------------- controllo batteria 
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer , HIGH);

    } else {

      digitalWrite(buzzer, LOW);

    }


    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      // picture loop
      u8g.firstPage();
      do {
        draw();
      } while ( u8g.nextPage() );

    }




    //    // If something available
    //  if (e22ttl.available()>1) {
    //      // read the String message
    //    ResponseContainer rc = e22ttl.receiveMessage();
    //    // Is something goes wrong print error
    //    if (rc.status.code!=1){
    //        rc.status.getResponseDescription();
    //    }else{
    //        // Print the data received
    //        Serial.println(rc.data);
    //    }
    //  }

    if (digitalRead(pulsante) == LOW) {
      ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 5, "OK");
      Serial.println(rs.getResponseDescription());
      Serial.println("Pulsante Premuto");
      digitalWrite(buzzer , HIGH);
      delay(1000);
      digitalWrite(buzzer, LOW);

    }

  }
}
