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


int buzzer = 12 ;
int pstop = 10;
int pgo = 9;
int r_print = 8;
//int pinbatt = A0; ho disabilitato la batteria sennò me sona il buzzer

int pinbatt = A3;
int secure = 0; // valore per evitare la pressione doppia
unsigned long previousMillis = 0; // will store last time voltage was updated

long interval = 3000;  // refresh schermo
String arraytempirosso[11];
String arraytaglirosso[10];
String arraytempiverde[11];
String arraytagliverde[10];
String arraytempiblu[11];
String arraytagliblu[10];

String RxData = "";

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
// lora parametri
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX



void setup() {
  Serial.begin(9600);        // imposta la comunicazione seriale
  printerSerial.begin(9600);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)
  pinMode(buzzer, OUTPUT);        // imposta come input il pin 10
  pinMode(pstop, INPUT_PULLUP);        // imposta come input il pin 11
  pinMode(pgo, INPUT_PULLUP );        // imposta come input il pin 12
  pinMode (pinbatt , INPUT);
  pinMode( r_print , INPUT_PULLUP );       // imposta come input il pin 12


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


  if (tensionebatt( pinbatt ) < 9.5) { // allarme tensione batteria
    tone( buzzer, 5000, 500);
  }
  if (currentMillis - previousMillis >= interval ) {
    // save the last time you read
    previousMillis = currentMillis;
    // read the tension of the battery
    float vout = tensionebatt( pinbatt);
    lcd.setCursor(3, 3);
    lcd.print("V.Batt.");
    lcd.print(vout);
  }

  if (digitalRead(pgo) == LOW && secure == 0) {
    tone( buzzer, 5000, 200);
    Serial.println("800,0,0,0,0,0,0,0,0,0");
    secure = 1 ;// qui è lingjippo tra la doppia pressione  e il non aspettare il send first
    draw(1, lcd);
  }
  if ( digitalRead(pstop) == LOW && secure == 1) {
    // Send BroadCast message
    tone( buzzer, 4000, 200);
    // stampatotali (gara, manche_rx, round_rx , nome_rosso,  tempi_rosso, nome_verde,  tempi_verde, nome_blu,  tempi_blu) ;
    draw(3, lcd);

  }

  if ( digitalRead(r_print) == LOW ) { // ristampa tempi
    // stampatotali (gara, manche_rx, round_rx , nome_rosso,  old_rosso, nome_verde,  old_verde, nome_blu,  old_blu) ;
    tone( buzzer, 3000, 200);
  }

  if (RxData == "2000") {
    draw(1, lcd);
    tone( buzzer, 3000, 200);
    RxData = "";
  }
  if (RxData.indexOf("5514")!= -1 ) {
    draw(1, lcd);
    decodestringone(RxData);
      tone( buzzer, 3000, 200);
    stampatotali ("Q500" , 2 , 1, "Pippo" , "Pluto"  ,  "Paperino");
    
      RxData = "";
    }

    // 805,Q500,2,3,Matteo,Giacomo,Francesco,Luca,Paolo,Pippo
    // lettura seriale
    if (Serial.available() > 0 ) {
      // read the incoming byte:
      incomingByte = Serial.read();

      my_str[pos] = incomingByte;
      pos++;


      if (incomingByte == 10) {                 //10 is line feed character
        splitCommaSeparated();

        //clear the string for next time
        for (int i = 0; i <= len - 1; i++) {
          my_str[i] = 0;
        }

        pos = 0;
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
