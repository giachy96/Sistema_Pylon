#include <AltSoftSerial.h>

#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>

#include "SystemFont5x7.h"
#include "Arial_black_16.h"


AltSoftSerial altSerial;


char msg;

/*
  Pin Defination
  nOE - 5 era9
  A - 6
  B - 7
  CLK - 13
  SCLK - 4 era 8
  Data - 11
*/

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 3
SPIDMD dmd(DISPLAYS_ACROSS,DISPLAYS_DOWN, 5, 6, 7, 4); // DMD controls the entire display
//DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);


void setup() {


  dmd.setBrightness(255);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  delay(500);
  Serial.begin(9600);
  delay(200);
  altSerial.begin(9600);

}


void loop() {

   dmd.drawString(  1, 1, "GO" );

if (altSerial.available()) {
    msg = altSerial.read();
    Serial.println(msg);
  }


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
    
    dmd.drawString(  1, 1, "GO" );
    msg = "";

  }
}



//void readMasterPort() {
//   
//  while (ArduinoMaster.available()) {
//
//    delay(10);
//    if (ArduinoMaster.available() > 0) {
//         
//      char c = ArduinoMaster.read();  //gets one byte from serial buffer
//      msg += c; //makes the string readString
//    }
//  }
//  ArduinoMaster.flush();

//}
