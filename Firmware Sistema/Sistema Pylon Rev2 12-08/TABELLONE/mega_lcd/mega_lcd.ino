#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include "codedecode.h"
#include "SystemFont5x7.h"
#include "Arial_Black_16.h"
#include "LoRa_E22.h"

LoRa_E22 e22ttl(&Serial2);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX

const byte numChars = 200;
char receivedChars1[numChars];
char oldreceivedChars1[numChars];
boolean newData = false;
String msg;
int updatescreen = 0;
/*
  Pin Defination
  nOE - 11
  A - 6
  B - 7
  CLK - 52
  SCLK - 8
  Data - 51
*/

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 3
SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 3, 6, 7, 8); // DMD controls the entire display
#include "countdown.h"
unsigned long currentMillis;
unsigned long lastdiplayupdate;
unsigned long intervalstandby = 5000;
unsigned long milliscountdown;
unsigned long sirenamillis;
unsigned long startsirena;
int sec;
int sirena = 2;
int sirenaflag = 0;
int k = 0;
boolean flagcount;
boolean showcroverde;
boolean showcrorosso;
boolean showcroblu;
String temporosso [5];
String tempoblu [5];
String tempoverde [5];
String values [5];
String arraytempirosso[11];
String arraytaglirosso[10];
String arraytempiverde[11];
String arraytagliverde[10];
String arraytempiblu[11];
String arraytagliblu[10];

String RxData;
byte Key = 0;
byte Add = 1;
byte Chan = 5;


unsigned long old523update;
int bounce = 0;

void setup() {

  pinMode(sirena, OUTPUT);
  dmd.setBrightness(255);
  // dmd.selectFont(SystemFont5x7);
  dmd.selectFont(Arial_Black_16);
  dmd.begin();
  delay(500);
  Serial.begin(9600);
  delay(200);
  Serial1.begin(9600); // SERIALE CON IL TEENSY
  delay(200);
  Serial2.begin(9600);
  e22ttl.begin();  //Start e22ttl

}


void loop() {
  currentMillis = millis();
  recvWithStartEndMarkers( Serial1,  receivedChars1) ;

  // If something available
  if (e22ttl.available() > 1) {
    // read the String message
    ResponseContainer rc = e22ttl.receiveMessage();
    RxData = rc.data;
    Serial.println(rc.data);

  }

  if (RxData == "2000") { // se ricevo lo show dalla centrale
    Serial1.println("<2000>");
  }
  if (RxData == "3000") { // se ricevo GO dalla centrale
    Serial1.println("<3000>");
  }
  if (RxData == "6000") { // se ricevo STOP dalla centrale
    Serial1.println("<6000>");
  }

  if (newData == true) {    // se ricevo un nuovo dato in seriale
    String Rxs;
    Rxs = receivedChars1;
    //  Serial.println(Rxs);

    if (Rxs.indexOf("2510") != -1 || Rxs.indexOf("2520") != -1 || Rxs.indexOf("2530") != -1) { // codice show
      values[0] = "0";
      decodecomma(Rxs, values); // decodifico i valori

      if (values[0] == "2510") {
        temporosso[0] = values[0];
        showcrorosso = true;
      } else if (values[0] == "2530") {
        tempoblu[0] = values[0];
        showcroblu = true;
      } else if (values[0] == "2520") {
        tempoverde[0] = values[0];
        showcroverde = true;
      }
      dmd.clearScreen();
      dmd.drawString(0, 1, "SHOW");
      dmd.drawString(0, 17, "SHOW");
      dmd.drawString(0, 33, "SHOW");
      checkboxcrono(52, 3, 60, 11 , showcroblu );
      checkboxcrono(52, 19, 60, 27, showcroverde );
      checkboxcrono(52, 35, 60, 43, showcrorosso );
    }


    if (Rxs.indexOf("4514") != -1 || Rxs.indexOf("4524") != -1 || Rxs.indexOf("4534") != -1) { // controllo se ci sta il codice della ricezione tempo
      values[0] = "0";
      decodecomma(Rxs, values); // decodifico i valori

      if (values[0] == "4514") {
        temporosso[0] = values[0];
        temporosso[1] = values[1];
        temporosso[2] = values[2];
      } else if (values[0] == "4534") {
        tempoblu[0] = values[0];
        tempoblu[1] = values[1];
        tempoblu[2] = values[2];
      } else if (values[0] == "4524") {
        tempoverde[0] = values[0];
        tempoverde[1] = values[1];
        tempoverde[2] = values[2];
      }
      dmd.clearScreen();
      dmd.drawString(0, 33, temporosso[1]);// 24/33
      dmd.drawString(18, 33, temporosso[2]);// 24/33
      dmd.drawString(0, 1, tempoblu[1]);// 24/33
      dmd.drawString(18, 1, tempoblu[2]);// 24/33
      dmd.drawString(0, 17, tempoverde[1]);// 24/33
      dmd.drawString(18, 17, tempoverde[2]);// 24/33
    }


    if (Rxs.indexOf("5514") != -1 ||  Rxs.indexOf("5524") != -1 || Rxs.indexOf("5534") != -1) { // codice ricezione 10 giri
      decodestringone(Rxs);
      //  memcpy(pacchettone.loraarraytempirosso, arraytempirosso, sizeof(arraytempirosso));
      ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, Rxs);
      if (values[0] == "5514") {
        //decodestringone(Rxs);
        //        temporosso[0] = values[0];
        //        temporosso[3] = values[2];
      } else if (values[0] == "5534") {
        //        tempoblu[0] = values[0];
        //        tempoblu[3] = values[2];
      } if (values[0] == "5524") {
        //        tempoverde[0] = values[0];
        //        tempoverde[3] = values[2];
      }

    }
    newData = false;
  }
  String strx = receivedChars1;
  if (strx.indexOf("5514") != -1  || strx.indexOf("5524") != -1 || strx.indexOf("5534") != -1) {

    if (millis() - old523update >= 1000) {
      if (bounce == 0) {
        dmd.clearScreen();
        dmd.drawString(0, 33, temporosso[1]);// 24/33
        dmd.drawString(18, 33, temporosso[2]);// 24/33
        dmd.drawString(0, 1, tempoblu[1]);// 24/33
        dmd.drawString(18, 1, tempoblu[2]);// 24/33
        dmd.drawString(0, 17, tempoverde[1]);// 24/33
        dmd.drawString(18, 17, tempoverde[2]);// 24/33
        bounce = 1;
      } else {

        dmd.clearScreen();
        dmd.drawString(10, 33, temporosso[3]);// 24/33
        dmd.drawString(10, 1, tempoblu[3]);// 24/33
        dmd.drawString(10, 17, tempoverde[3]);// 24/33
        bounce = 0;
      }
      old523update = millis();
    }

  }

  // GESTIONE DEL CONTO ALLA ROVESCIA
  flagcount = countdown(flagcount);

  // GESTIONE DELLA SIRENA
  if (sirenaflag == 1) {

    sirenamillis = millis();

    if (k == 0 ) {
      startsirena = millis();
      digitalWrite(sirena , HIGH);
      k = 1;
    }

    if (sirenamillis - startsirena >= 300  ) {
      digitalWrite(sirena , LOW);
      k = 0;
      sirenaflag = 0;
    }
  }

  //FINE GESTIONE SIRENA

  if (strcmp(receivedChars1, oldreceivedChars1) != 0) {
    updatescreen = 0;

    if (strcmp(receivedChars1, "STOP") == 0 && updatescreen == 0 ) {
      dmd.clearScreen();
      dmd.drawString(8, 1, "STOP");
      dmd.drawString(8, 17, "STOP");
      dmd.drawString(8, 33, "STOP");

      if (RxData != "6000") {
        ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "6000");
      }
      flagcount = false;
      RxData = "";
      updatescreen = 1;
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));
    }
    if (strcmp(receivedChars1, "GO") == 0 && updatescreen == 0  ) {
      flagcount = true;
      updatescreen = 1;
      memset(arraytempirosso, 0 , sizeof(arraytempirosso));
      memset(arraytaglirosso, 0 , sizeof(arraytaglirosso));
      memset(arraytempiverde, 0 , sizeof(arraytempiverde));
      memset(arraytagliverde, 0 , sizeof(arraytagliverde));
      memset(arraytempiblu, 0 , sizeof(arraytempiblu));
      memset(arraytagliblu, 0 , sizeof(arraytagliblu));

      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));

      if (RxData != "3000") {
        ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "3000");
      }
      RxData = "";


    }
    if (strcmp(receivedChars1, "SHOW") == 0 && updatescreen == 0  ) {
      memset(arraytempirosso, 0 , sizeof(arraytempirosso));
      memset(arraytaglirosso, 0 , sizeof(arraytaglirosso));
      memset(arraytempiverde, 0 , sizeof(arraytempiverde));
      memset(arraytagliverde, 0 , sizeof(arraytagliverde));
      memset(arraytempiblu, 0 , sizeof(arraytempiblu));
      memset(arraytagliblu, 0 , sizeof(arraytagliblu));
      if (RxData != "2000") {
        ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "2000");
      }

      dmd.clearScreen();
      dmd.drawString(0, 1, "SHOW");
      dmd.drawString(0, 17, "SHOW");
      dmd.drawString(0, 33, "SHOW");
      dmd.drawBox(52, 3, 60, 11);
      dmd.drawBox(52, 19, 60, 27);
      dmd.drawBox(52, 35, 60, 43);
      //   dmd.drawFilledBox(8, 12, 9, 13);
      showcroverde = false;
      showcrorosso = false;
      showcroblu = false;
      updatescreen = 1;
      RxData = "";
      flagcount = false;
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));
    }

  }
}









void  recvWithStartEndMarkers( HardwareSerial & port , char receivedChars[numChars] ) {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (port.available() > 0 && newData == false) {
    rc = port.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
  port.flush();

}


void checkboxcrono (int xs , int ys , int xe , int ye, boolean flg ) {

  if (flg == true) {
    dmd.drawFilledBox(xs, ys, xe, ye);
  } else {
    dmd.drawBox(xs, ys, xe, ye);
  }
}
