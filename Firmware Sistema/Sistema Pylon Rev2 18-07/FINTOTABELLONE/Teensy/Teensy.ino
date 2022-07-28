#include"SoftwareSerial.h"
#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial14.h>


// You can change to a smaller font (two lines) by commenting this line,
// and uncommenting the line after it:
//const uint8_t *FONT = Arial14;
const uint8_t *FONT = SystemFont5x7;



#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 3

SoftDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);  // DMD controls the entire display
DMD_TextBox box(dmd);  // "box" provides a text box to automatically write to/scroll the display

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  dmd.drawString(1, 5, "Paolo");
  dmd.drawString(1, 22, "Alessandro");
  dmd.drawString(1, 40, "Daniele");
}
