
#include <AltSoftSerial.h>

#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>

#include "SystemFont5x7.h"
#include "Arial_black_16.h"

const byte numChars = 64;
char receivedChars1[numChars];
boolean newData = false;
String msg;

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

void setup() {


  dmd.setBrightness(255);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  delay(500);
  Serial.begin(9600);
  delay(200);
  Serial1.begin(9600);

}


void loop() {

  recvWithStartEndMarkers( Serial1,  receivedChars1) ;
  showNewData( receivedChars1);


  if (msg == "STOP") {
    //    dmd.drawString(1, 0, "STOP");
    //    dmd.drawString(1, 19, "STOP");
    //    dmd.drawString(1, 38, "STOP");

    dmd.drawString( 1,  1, "STOP" );
    msg = "";
  }
  else {
    //    dmd.drawString(1, 0, "GO");
    //    dmd.drawString(1, 19, "GO");
    //    dmd.drawString(1, 38, "GO");
    dmd.clearScreen();
    dmd.drawString(  1, 1, "FORSE" );
    delay(5000);
    dmd.clearScreen();
    dmd.drawString(  1, 1, "FUNZIONA" );
    msg = "";
    delay(5000);

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
