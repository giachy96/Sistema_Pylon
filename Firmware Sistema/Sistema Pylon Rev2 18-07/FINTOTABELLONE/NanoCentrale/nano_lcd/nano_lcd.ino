#include <SoftwareSerial.h>
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

SoftwareSerial ArduinoMaster(4, 5);

String msg;
const int scandelaylcd = 0;
unsigned long CurrentMillis;
unsigned long OldMillis;


/*
  Pin Defination
  nOE - 9
  A - 6
  B - 7
  CLK - 13
  SCLK - 8
  Data - 11
*/
//SPIDMD dmd(2,3, 9, 6, 7, 8); // DMD controls the entire display
//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 3

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

void ScanDMD()
{
  dmd.scanDisplayBySPI();
}

void setup() {


  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

  //clear/init the DMD pixels held in RAM
  dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)



  delay(500);
  Serial.begin(9600);
  delay(200);
  ArduinoMaster.begin(9600);

}


void loop() {
  //  CurrentMillis = millis();
  //  if (CurrentMillis - OldMillis >= scandelaylcd) {
  //    //dmd.scan_running_dmds();
  //    OldMillis = millis();
  //
  //  }
  readMasterPort();
  // Send answer to master
  if (msg != "") {
    Serial.print("Master sent : " );
    Serial.println(msg);

  }



  if (msg == "STOP") {
    //    dmd.drawString(1, 0, "STOP");
    //    dmd.drawString(1, 19, "STOP");
    //    dmd.drawString(1, 38, "STOP");
    dmd.selectFont(Arial_Black_16);
    dmd.drawString( 1,  1, "OK", 5, GRAPHICS_NORMAL );
    msg = "";
  }
  else {
    //    dmd.drawString(1, 0, "GO");
    //    dmd.drawString(1, 19, "GO");
    //    dmd.drawString(1, 38, "GO");
    dmd.selectFont(Arial_Black_16);
    dmd.drawString(  1, 1, "ronic", 5, GRAPHICS_NORMAL );
    msg = "";

  }
}



void readMasterPort() {
   
  while (ArduinoMaster.available()) {

    delay(10);
    if (ArduinoMaster.available() > 0) {
         
      char c = ArduinoMaster.read();  //gets one byte from serial buffer
      msg += c; //makes the string readString
    }
  }
  ArduinoMaster.flush();

}
