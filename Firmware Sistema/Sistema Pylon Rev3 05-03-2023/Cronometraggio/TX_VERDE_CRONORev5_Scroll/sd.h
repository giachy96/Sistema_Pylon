#include <SD.h>

#define PIN_SPI_CS 4

File myFile;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(PIN_SPI_CS)) {
    Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
    while (1); // don't do anything more:
  }

  Serial.println(F("SD CARD INITIALIZED."));
  SD.remove("arduino.txt"); // delete the file if existed

  // create new file by opening file for writing
  myFile = SD.open("arduino.txt", FILE_WRITE);

  if (myFile) {
    myFile.println("Created by ArduinoGetStarted.com"); // write a line to Arduino
    myFile.println("Learn Arduino and SD Card"); // write another  line to Arduino
    myFile.close();
  } else {
    Serial.print(F("SD Card: error on opening file arduino.txt"));
  }

  // open file for reading
  myFile = SD.open("arduino.txt", FILE_READ);
  if (myFile) {
    while (myFile.available()) {
      char ch = myFile.read(); // read characters one by one from Micro SD Card
      Serial.print(ch); // print the character to Serial Monitor
    }
    myFile.close();
  } else {
    Serial.print(F("SD Card: error on opening file arduino.txt"));
  }
}
