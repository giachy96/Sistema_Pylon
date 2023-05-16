/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11, pin 7 on Teensy with audio board
 ** MISO - pin 12
 ** CLK - pin 13, pin 14 on Teensy with audio board
 ** CS - pin 4,  pin 10 on Teensy with audio board

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.
*/

#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>

// Teensy 3.5 & 3.6 & 4.1 on-board: BUILTIN_SDCARD

const int chipSelect = BUILTIN_SDCARD;
String nome_file = "Backup_";


void setup()
{

  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // the card initialized:
  SD.begin(chipSelect);
  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  nome_file.concat(day());
  nome_file.concat("_");
  nome_file.concat(month());
  nome_file.concat("_");
  nome_file.concat(year());
  nome_file.concat(".txt");

}

void loop()
{
  // make a string for assembling the data to log:
  String dataString = hour();
  dataString += ":";
  dataString += minute();
  dataString += ":";
  dataString += second();
  dataString += "    CIAO";

  //  // read three sensors and append to the string:
  //  for (int analogPin = 0; analogPin < 3; analogPin++) {
  //    int sensor = analogRead(analogPin);
  //    dataString += String(sensor);
  //    if (analogPin < 2) {
  //      dataString += ",";
  //    }
  //  }

  // open the file.

  int str_len = nome_file.length() + 1;
  char nome_file_char[str_len];
  nome_file.toCharArray(nome_file_char, str_len);
  File dataFile = SD.open(nome_file_char, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println("Ho scritto su SD");
  } else {
    // if the file isn't open, pop up an error:
    Serial.println("error opening file.txt");
  }
  digitalClockDisplay();
  delay(1000); // run at a reasonable not-too-fast speed

}



void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    return pctime;
    if ( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
