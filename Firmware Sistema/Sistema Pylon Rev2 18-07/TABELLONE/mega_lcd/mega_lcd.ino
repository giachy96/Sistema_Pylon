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
  //showNewData( receivedChars1); attenazione al new Data

 if (newData == true) {
    String Rxs;
    Rxs = receivedChars1;
    if (Rxs.indexOf("423") != -1) {
    String values [5];
    decodecomma(Rxs,values);
    Serial.println("valore1");
    Serial.println(values[1]);
    Serial.println("valore2");
    Serial.println(values[2]);
    Serial.println("valore3");
    Serial.println(values[3]);
    dmd.clearScreen();
    dmd.drawString(0, 33, values[1]);// 24/33
    dmd.drawString(20, 33, values[2]);// 24/33
    }
    newData= false;
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
      dmd.drawString(8, 1, "SHOW");
      dmd.drawString(8, 17, "SHOW");
      dmd.drawString(8, 33, "SHOW");


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

boolean countdown(boolean flag) {

  if (flag == true) {


    if (currentMillis - milliscountdown >= 1000) {
      dmd.selectFont(Arial_Black_16);
      if (sec > 5 && sec != 32  && sec != 12  ) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        sirenaflag = 0;
        sec = sec - 1;
      }  else if (sec == 32) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 12) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 5) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 4) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 3) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 2) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        sirenaflag = 1;
        sec = sec - 1;
      }
      else if (sec == 1) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 17, "GO");
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        sirenaflag = 1;
        sec = sec - 1;
      }
      else if (sec == 0) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 1, "GO");
        dmd.drawString(24, 17, "GO");
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        sirenaflag = 1;
        sec = sec - 1;
      }
      else if (sec == -1) {
        char cstr[10];

        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 1, "GO");
        dmd.drawString(24, 17, "GO");
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        sirenaflag = 0;
        flag = false;
        sec = 62;
        Serial.println(sec);
        Serial.println(flag);
      }

    }
  } else {

    milliscountdown = millis();
    flag = false;
    sec = 62;


  }
  return flag;
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


int decodecomma (String str , String tempi[]){
   int lungh_str = str.length();
   char buff[lungh_str+1];
    str.toCharArray(buff, lungh_str+1);
   //Serial.println(parseData(buff));
      int i =0;
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
