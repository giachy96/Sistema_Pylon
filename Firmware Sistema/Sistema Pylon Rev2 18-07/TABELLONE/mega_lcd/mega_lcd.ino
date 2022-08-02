#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>

#include "SystemFont5x7.h"
#include "Arial_black_16.h"

const byte numChars = 5;
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

unsigned long currentMillis;
unsigned long lastdiplayupdate;
unsigned long intervalstandby = 5000;
unsigned long milliscountdown;
int sec = 63;
int sirena = 10;

boolean flagcount;
void setup() {

  pinMode(sirena, OUTPUT);
  dmd.setBrightness(255);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  delay(500);
  Serial.begin(9600);
  delay(200);
  Serial1.begin(9600);

}


void loop() {
  currentMillis = millis();
  recvWithStartEndMarkers( Serial1,  receivedChars1) ;
  showNewData( receivedChars1);

  if (flagcount == true) {
    countdown(flagcount);
  }



  if (strcmp(receivedChars1, oldreceivedChars1) != 0) {
    updatescreen = 0;

    if (strcmp(receivedChars1, "STOP") == 0 && updatescreen == 0 ) {
      dmd.clearScreen();
      dmd.drawString(0, 4 , "STOP");
      dmd.drawString(0, 20, "STOP");
      dmd.drawString(0, 36, "STOP");

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
      dmd.drawString(0, 4, "SHOW");
      dmd.drawString(0, 20, "SHOW");
      dmd.drawString(0, 36, "SHOW");


      updatescreen = 1;
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

void countdown(boolean flag) {

  if (flag == true) {


    if (currentMillis - milliscountdown >= 1000) {
      if (sec > 0) {
        char cstr[10];

        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(0, 4, cstr);
        dmd.drawString(0, 20, cstr);
        dmd.drawString(0, 36, cstr);
        milliscountdown = millis();

        sec = sec - 1;
      } else if (sec = 3) {
        char cstr[10];
        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(0, 4, cstr);
        dmd.drawString(0, 20, cstr);
        dmd.drawString(0, 36, "GO");
        milliscountdown = millis();

        sec = sec - 1;
      }
      else if (sec = 2) {
        char cstr[10];
        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(0, 4, cstr);
        dmd.drawString(0, 20, "GO");
        dmd.drawString(0, 36, "GO");
        milliscountdown = millis();

        sec = sec - 1;
      } else if (sec = 1) {
        char cstr[10];

        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(0, 4, "GO");
        dmd.drawString(0, 20, "GO");
        dmd.drawString(0, 36, "GO");
        milliscountdown = millis();

        sec = sec - 1;
      } else if (sec = 30) {
        char cstr[10];



        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(0, 4, cstr);
        dmd.drawString(0, 20, cstr);
        dmd.drawString(0, 36, cstr);
        milliscountdown = millis();
        sirena.tone(500);

        sec = sec - 1;
      }
      else {
        char cstr[10];

        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(0, 4, cstr);
        dmd.drawString(0, 20, cstr);
        dmd.drawString(0, 36, cstr);
        milliscountdown = millis();
        flag = false;
        sec = 63;
      }
    }
  }
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

void showNewData(char receivedChars[numChars]) {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}
void readMasterPort() {

  while (Serial1.available()) {

    delay(10);
    if (Serial1.available() > 0) {

      char c = Serial1.read();  //gets one byte from serial buffer
      msg += c; //makes the string readString
    }
  }
  Serial1.flush();
  Serial.println(msg);

}
