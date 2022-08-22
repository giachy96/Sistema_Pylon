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
String arraytempirosso[11];
String arraytaglirosso[10];
String arraytempiverde[11];
String arraytagliverde[10];
String arraytempiblu[11];
String arraytagliblu[10];

String RxData = "";

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
// lora parametri
LoRa_E22 e22ttl(10, 11);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX



void setup() {
  Serial.begin(9600);            // imposta la comunicazione seriale
  Serial2.begin(9600);           // Initialize HwSerial per la Stampanate
  printer.begin();               // Init printer (same regardless of serial type)
  pinMode(buzzer, OUTPUT);       // imposta come input il pin 10
  pinMode(pstop, INPUT_PULLUP);  // imposta come input il pin 11
  pinMode(pgo, INPUT_PULLUP);    // imposta come input il pin 12
  pinMode(pinbatt, INPUT);
  pinMode(r_print, INPUT_PULLUP);  // imposta come input il pin 12
  pinMode(checkPC, INPUT);
  pinMode(pshow, INPUT_PULLUP);
  pinMode(ledstatusPC, OUTPUT);
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

  if (currentMillis - oldcheckPC >= intervalcheckPC) {
    if (digitalRead(checkPC) == HIGH) {
      digitalWrite(ledstatusPC, HIGH);
      statusPC = true;
    } else {
      digitalWrite(ledstatusPC, LOW);
      statusPC = false;
    }
    oldcheckPC = millis();
  }

  if (currentMillis - oldPress >= intervalPress) {
    doublePress = 0;
  }

  if (tensionebatt(pinbatt) < 9.5) {  // allarme tensione batteria
    tone(buzzer, 5000, 500);
  }
  if (currentMillis - previousMillis >= interval) {
    // save the last time you read
    previousMillis = currentMillis;
    // read the tension of the battery
    float vout = tensionebatt(pinbatt);
    lcd.setCursor(3, 3);
    lcd.print("V.Batt.");
    lcd.print(vout);
  }

  if (digitalRead(pshow) == LOW && doublePress == 0) {
    tone(buzzer, 4000, 200);
    doublePress = 1;
    oldPress = millis();
    //Serial.println("800,0,0,0,0,0,0,0,0,0");
    draw(2, lcd);
  }

  if (digitalRead(pgo) == LOW && doublePress == 0) {
    tone(buzzer, 4000, 200);
    //Serial.println("800,0,0,0,0,0,0,0,0,0");
    draw(3, lcd);
    doublePress = 1;
    oldPress = millis();
  }
  if (digitalRead(pstop) == LOW && doublePress == 0) {
    tone(buzzer, 5000, 200);
    // stampatotali (gara, manche_rx, round_rx , nome_rosso,  tempi_rosso, nome_verde,  tempi_verde, nome_blu,  tempi_blu) ;
    draw(5, lcd);
    doublePress = 1;
    oldPress = millis();
  }

  if (digitalRead(r_print) == LOW && doublePress == 0) {  // ristampa tempi
    // stampatotali (gara, manche_rx, round_rx , nome_rosso,  old_rosso, nome_verde,  old_verde, nome_blu,  old_blu) ;
    tone(buzzer, 3000, 200);
    doublePress = 1;
    oldPress = millis();
  }

  if (RxData == "2000") {  // se ricevo show
    draw(2, lcd);
    tone(buzzer, 3000, 200);
    RxData = "";
  }

    if (RxData == "3000") {  // se ricevo show
    draw(3, lcd);
    tone(buzzer, 3000, 200);
    RxData = "";
  }

     if (RxData == "6000") {  // se ricevo show
    draw(5, lcd);
    tone(buzzer, 5000, 200);
    RxData = "";
  }

  if (RxData.indexOf("5514") != -1) {  // se ricevo fine 10 giri rosso
    draw(1, lcd);
    decodestringone(RxData);
    tone(buzzer, 3000, 200);
    stampatotali("Q500", 2, 1, "Pippo", "Pluto", "Paperino");
    RxData = "";
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





  // If something available
  if (e22ttl.available() > 1) {
    // read the  message
    ResponseContainer rc = e22ttl.receiveMessage();
    Serial.println(rc.data);
    RxData = rc.data;

    // Print the data received
  }
}