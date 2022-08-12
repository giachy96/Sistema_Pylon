#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>

#include "SystemFont5x7.h"
#include "Arial_Black_16.h"

const byte numChars = 32;
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
SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 11, 6, 7, 8); // DMD controls the entire display
#include "countdown.h"
unsigned long currentMillis;
unsigned long lastdiplayupdate;
unsigned long intervalstandby = 5000;
unsigned long milliscountdown;
unsigned long sirenamillis;
unsigned long startsirena;
int sec;
int sirena = 10;
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
  Serial1.begin(9600);

}


void loop() {
  currentMillis = millis();
  recvWithStartEndMarkers( Serial1,  receivedChars1) ;


  if (newData == true) {    // se ricevo un nuovo dato in seriale
    String Rxs;
    Rxs = receivedChars1;

    if (Rxs.indexOf("220") != -1 || Rxs.indexOf("210") != -1 || Rxs.indexOf("230") != -1) { // codice show
      values[0] = "0";
      decodecomma(Rxs, values); // decodifico i valori

      if (values[0] == "220") {
        temporosso[0] = values[0];
        showcrorosso = true;
      } else if (values[0] == "230") {
        tempoblu[0] = values[0];
        showcroblu = true;
      } else if (values[0] == "210") {
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


    if (Rxs.indexOf("423") != -1 || Rxs.indexOf("433") != -1 || Rxs.indexOf("413") != -1) { // controllo se ci sta il codice della ricezione tempo
      values[0] = "0";
      decodecomma(Rxs, values); // decodifico i valori

      if (values[0] == "423") {
        temporosso[0] = values[0];
        temporosso[1] = values[1];
        temporosso[2] = values[2];
      } else if (values[0] == "433") {
        tempoblu[0] = values[0];
        tempoblu[1] = values[1];
        tempoblu[2] = values[2];
      } else if (values[0] == "413") {
        tempoverde[0] = values[0];
        tempoverde[1] = values[1];
        tempoverde[2] = values[2];
      }
      dmd.clearScreen();
      dmd.drawString(0, 33, temporosso[1]);// 24/33
      dmd.drawString(20, 33, temporosso[2]);// 24/33
      dmd.drawString(0, 1, tempoblu[1]);// 24/33
      dmd.drawString(20, 1, tempoblu[2]);// 24/33
      dmd.drawString(0, 17, tempoverde[1]);// 24/33
      dmd.drawString(20, 17, tempoverde[2]);// 24/33
    }


    if (Rxs.indexOf("523") != -1 ||  Rxs.indexOf("533") != -1 || Rxs.indexOf("513") != -1) { // codice ricezione 10 giri
      decodecomma(Rxs, values);
      if (values[0] == "523") {
        temporosso[0] = values[0];
        temporosso[3] = values[2];
      } else if (values[0] == "533") {
        tempoblu[0] = values[0];
        tempoblu[3] = values[2];
      } if (values[0] == "513") {
        tempoverde[0] = values[0];
        tempoverde[3] = values[2];
      }

    }
    newData = false;
  }
 String strx = receivedChars1;
  if (strx.indexOf("523") != -1  || strx.indexOf("513") != -1 || strx.indexOf("533") != -1) {

    if (millis() - old523update >= 1000) {
      if (bounce == 0) {
        dmd.clearScreen();
        dmd.drawString(0, 33, temporosso[1]);// 24/33
        dmd.drawString(20, 33, temporosso[2]);// 24/33
        dmd.drawString(0, 1, tempoblu[1]);// 24/33
        dmd.drawString(20, 1, tempoblu[2]);// 24/33
        dmd.drawString(0, 17, tempoverde[1]);// 24/33
        dmd.drawString(20, 17, tempoverde[2]);// 24/33
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
  if (strcmp(receivedChars1, oldreceivedChars1) != 0) {
    updatescreen = 0;

    if (strcmp(receivedChars1, "STOP") == 0 && updatescreen == 0 ) {
      dmd.clearScreen();
      dmd.drawString(8, 1, "STOP");
      dmd.drawString(8, 17, "STOP");
      dmd.drawString(8, 33, "STOP");

      flagcount = false;

      updatescreen = 1;
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));
    }
    if (strcmp(receivedChars1, "GO") == 0 && updatescreen == 0  ) {
      //      dmd.clearScreen();
      //      dmd.drawString(0, 4, "GO");
      //      dmd.drawString(0, 20, "GO");
      //      dmd.drawString(0, 36, "GO");
      flagcount = true;
      updatescreen = 1;
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));

    }
    if (strcmp(receivedChars1, "SHOW") == 0 && updatescreen == 0  ) {
      dmd.clearScreen();
      //      dmd.drawString(8, 1, "SHOW");
      //      dmd.drawString(8, 17, "SHOW");
      //      dmd.drawString(8, 33, "SHOW");
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
       flagcount = false;
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));
      memcpy(oldreceivedChars1, receivedChars1, sizeof(receivedChars1));
    }

  }

  //  if (  currentMillis -  lastdiplayupdate >= intervalstandby  && strcmp(receivedChars1, oldreceivedChars1) == 0 && updatescreen ==  ) {
  //
  //
  //    dmd.clearScreen();
  //    dmd.drawString(0, 4, "ATTESA");
  //    dmd.drawString(0, 20, "SISTEMA IN");
  //    dmd.drawString(0, 36, "FT MODELS");
  //    lastdiplayupdate = millis();
  //
  //  }
}









void  recvWithStartEndMarkers( HardwareSerial &port , char receivedChars[numChars] ) {
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




int decodecomma (String str , String tempi[]) {
  int lungh_str = str.length();
  char buff[lungh_str + 1];
  str.toCharArray(buff, lungh_str + 1);
  //Serial.println(parseData(buff));
  int i = 0;
  char* p;
  //Serial.println("Example of splitting a string into tokens: ");
  // Serial.print("The input string is: '");
  // Serial.print(buff);
  // Serial.println("'");

  p = strtok(buff, "{,}"); //2nd argument is a char[] of delimiters
  while (p != '\0') { //not equal to NULL
    //Serial.println(p);
    tempi[i] = p;

    p = strtok('\0', "{,}");  //expects NULL for string on subsequent calls
    i++;
  }

  return tempi;
}

void checkboxcrono (int xs , int ys , int xe , int ye, boolean flg ) {

  if (flg == true) {
    dmd.drawFilledBox(xs, ys, xe, ye);
  } else {
    dmd.drawBox(xs, ys, xe, ye);
  }
}
