#include "SoftwareSerial.h"
#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial14.h>

// You can change to a smaller font (two lines) by commenting this line,
// and uncommenting the line after it:
// const uint8_t *FONT = Arial14;
const uint8_t *FONT = SystemFont5x7;

SoftwareSerial SSerial(2,3); // RX, TX

String dataS; //Initialized variable to store recieved data

#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 3

SoftDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN); // DMD controls the entire display
DMD_TextBox box(dmd);                        // "box" provides a text box to automatically write to/scroll the display

// the setup routine runs once when you press reset:
void setup()
{
  pinMode(4,INPUT);
  pinMode(5,OUTPUT);
  Serial.begin(9600);
  SSerial.begin(9600);
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
}

// the loop routine runs over and over again forever:
void loop()
{
  if (SSerial.available()) {
    // SSerial.readBytes(mystr,5); //Read the serial data and store in var
   dataS = SSerial.readStringUntil('@'); //Read the serial data and store in var
    delay(100);
    Serial.println(dataS); //Print data on Serial Monitor

    if (dataS=="STOP")
    {
      dmd.drawString(1, 5, "STOP");
      dmd.drawString(1, 19, "STOP");
      dmd.drawString(1, 38, "STOP");
    }
    else {
      dmd.clearScreen();
    }
  }
}
