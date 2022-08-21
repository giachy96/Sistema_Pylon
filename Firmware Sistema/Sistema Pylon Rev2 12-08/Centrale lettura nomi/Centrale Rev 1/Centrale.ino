#include <Metro.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "LoRa_E22.h"
#include "lcdcases.h"
#include "commacode.h"
#include "printercases.h"
#include "exceldecode.h"


int buzzer = 12 ;
int pstop = 10;
int pgo = 9;
int r_print = 8;
int pinbatt = A0;
int sendfirst = 0; //per iniziare la trasmissione dei dati dopo aver ricvuto il sinc
int secure = 0; // valore per evitare la pressione doppia
int seriale_occupata = 0;
int  codice = 0;
unsigned long sinc = 0;
unsigned long previousMillis = 0; // will store last time voltage was updated
unsigned long lastsinc = 0;
char condfirst[20] = "FIRST";
// constants won't change:
long interval = 3000;  // refresh schermo
unsigned long intervallo_sinc = 3000; // chimata per i tempi
float tempi_verde[11];
float old_verde[sizeof(tempi_verde) / sizeof(float)];
int v = 0;
float tempi_rosso[11];
float old_rosso[sizeof(tempi_rosso) / sizeof(float)];
int r = 0;
float tempi_blu[11];
float old_blu[sizeof(tempi_blu) / sizeof(float)];
int b = 0;
int tmp_array_decode[3];
int dati_ricevuti = 0;


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
  sinc = millis() - lastsinc;

  if (tensionebatt( pinbatt ) < 9.5) { // allarme tensione batteria
    tone( buzzer, 5000, 500);
  }
  if (currentMillis - previousMillis >= interval && sendfirst == 0) {
    // save the last time you read
    previousMillis = currentMillis;
    // read the tension of the battery
    float vout = tensionebatt( pinbatt);
    lcd.setCursor(3, 3);
    lcd.print("V.Batt.");
    lcd.print(vout);
  }

  if ((sinc >= intervallo_sinc  && sendfirst == 1  && (seriale_occupata == 0 || seriale_occupata >= 3)) || (sinc > 7000 && sendfirst == 1)) {
    seriale_occupata = 0;
    lastsinc = millis();
    codice = 201; // send
    ResponseStatus rs = e22ttl.sendBroadcastFixedMessage(6, String(codice));
    sinc = 0;


  }
  if (digitalRead(pgo) == LOW && secure == 0) {
    // Send BroadCast message
    lastsinc = millis();
    sinc = 0;
    codice = 101; // Sinc
    ResponseStatus rs = e22ttl.sendBroadcastFixedMessage(6, String(codice));
    tone( buzzer, 5000, 200);
    Serial.println("800,0,0,0,0,0,0,0,0,0");
    secure = 1 ;// qui è lingjippo tra la doppia pressione  e il non aspettare il send first
    draw(1, lcd);
  }
  if ( digitalRead(pstop) == LOW && secure == 1) {
    // Send BroadCast message
    tone( buzzer, 4000, 200);
    codice = 500; // Stop
    ResponseStatus rs = e22ttl.sendBroadcastFixedMessage(6, String(codice));
    delay(200);
    stampatotali (gara, manche_rx, round_rx , nome_rosso,  tempi_rosso, nome_verde,  tempi_verde, nome_blu,  tempi_blu) ;
    memcpy(old_verde , tempi_verde , sizeof(tempi_verde));
    memcpy(old_rosso , tempi_rosso , sizeof(tempi_rosso));
    memcpy(old_blu , tempi_blu , sizeof(tempi_blu));
    secure = 0;
    sendfirst = 0;
    dati_ricevuti = 0;
    r = 0;
    v = 0;
    b = 0;
    memset(tempi_verde, 0, sizeof(tempi_verde));// resetto gli array a 0
    memset(tempi_rosso, 0, sizeof(tempi_rosso));
    memset(tempi_blu, 0, sizeof(tempi_blu));
    draw(3, lcd);

  }

  if ( digitalRead(r_print) == LOW ) { // ristampa tempi
    stampatotali (gara, manche_rx, round_rx , nome_rosso,  old_rosso, nome_verde,  old_verde, nome_blu,  old_blu) ;
    tone( buzzer, 3000, 200);
  }

  if ( tempi_verde[10] != 0  && tempi_rosso[10] != 0 && tempi_blu[10] != 0     ) {
    stampatotali (gara, manche_rx, round_rx , nome_rosso,  tempi_rosso, nome_verde,  tempi_verde, nome_blu,  tempi_blu) ;
    memcpy(old_verde , tempi_verde , sizeof(tempi_verde));
    memcpy(old_rosso , tempi_rosso , sizeof(tempi_rosso));
    memcpy(old_blu , tempi_blu , sizeof(tempi_blu));
    //invia al computer
    //    resetta le variabili
    draw(4, lcd);
    secure = 0;
    sendfirst = 0;
    dati_ricevuti = 0;
    r = 0;
    v = 0;
    b = 0;
    memset(tempi_verde, 0, sizeof(tempi_verde));// resetto gli array a 0
    memset(tempi_rosso, 0, sizeof(tempi_rosso));
    memset(tempi_blu, 0, sizeof(tempi_blu));


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
    seriale_occupata++;
    // Print the data received
    if (rc.data == "200") { // SendFirst
      sendfirst = 1;
      draw(2, lcd);
    }

    else {
      decodecomma(rc.data , tmp_array_decode );
      if (tmp_array_decode[1] == 02) {
        if (tmp_array_decode [2] != 999) {
          tempi_rosso[r] = float(tmp_array_decode [2]) / 100;
          r++;
          draw(2, lcd);
        }
      }
      else if (tmp_array_decode[1] == 01) {
        if (tmp_array_decode [2] != 999) {
          tempi_verde[v] = float(tmp_array_decode [2]) / 100;
          v++;
          draw(2, lcd);
        }
      }
      else if (tmp_array_decode[1] == 03) {
        if (tmp_array_decode [2] != 999) {
          tempi_blu[b] = float(tmp_array_decode [2]) / 100;
          b++;
          draw(2, lcd);
        }
      }

    }

  }


}
