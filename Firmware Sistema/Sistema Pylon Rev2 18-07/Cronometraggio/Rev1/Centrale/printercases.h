#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"

#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on Stampanate
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on Stampanate

SoftwareSerial printerSerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&printerSerial, 4);    // Pass addr to printer constructor

void stampatotali (float tempi_verde[11], float tempi_rosso[11], float tempi_blu[11]) {
  printer.setSize('M');        // Set type size, accepts 'S', 'M', 'L'
  printer.println(F("GARA _________"));
  printer.feed(1);
  printer.println(F("MANCHE N : ____ "));

  printer.feed(2);
  printer.setSize('S');
  printer.println(F("Pilota ROSSO "));
  printer.println(F("NOME __________________  Taglio"));
  printer.feed(1);
  for (int y = 0 ; y < 10; y++) {
    printer.print("LAP N: ");
    printer.print(y + 1);
    printer.print("  Tempo: ");
    printer.print(tempi_rosso[y]);
    printer.print("s     ");
    printer.println("[]");
  }
  printer.print(F("Tempo Totale: "));
  printer.print(tempi_rosso[10]);
  printer.print(F("s"));
  printer.println(F(" "));
  printer.print("Numero Tagli: ");



  printer.feed(2);
  printer.setSize('S');
  printer.println(F("Pilota VERDE "));
  printer.println(F("NOME __________________  Taglio"));
  printer.feed(1);
  for (int y = 0 ; y < 10; y++) {
    printer.print("LAP N: ");
    printer.print(y + 1);
    printer.print("  Tempo: ");
    printer.print(tempi_verde[y]);
    printer.print("s     ");
    printer.println("[]");
  }
  printer.print(F("Tempo Totale: "));
  printer.print(tempi_verde[10]);
  printer.print(F("s"));
  printer.println(F(" "));
  printer.print("Numero Tagli: ");


  printer.feed(2);
  printer.setSize('S');
  printer.println(F("Pilota BLU "));
  printer.println(F("NOME __________________  Taglio"));
  printer.feed(1);
  for (int y = 0 ; y < 10; y++) {
    printer.print("LAP N: ");
    printer.print(y + 1);
    printer.print("  Tempo: ");
    printer.print(tempi_blu[y]);
    printer.print("s     ");
    printer.println("[]");
  }
  printer.print(F("Tempo Totale: "));
  printer.print(tempi_blu[10]);
  printer.print(F("s"));
  printer.println(F(" "));
  printer.print("Numero Tagli: ");

  printer.feed(3);


}
