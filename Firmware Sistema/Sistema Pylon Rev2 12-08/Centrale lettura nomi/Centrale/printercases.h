#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"

#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on Stampanate
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on Stampanate

extern String nome_rosso ;


SoftwareSerial printerSerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&printerSerial, 4);    // Pass addr to printer constructor

void stampatotali (String gara , int manche_rx , int round_rx, String nome_rosso , float tempi_rosso[11], String nome_verde  , float tempi_verde[11], String nome_blu , float tempi_blu[11]) {
  printer.setSize('M');        // Set type size, accepts 'S', 'M', 'L'
  printer.print(F("GARA "));
  printer.println(gara);
  printer.feed(1);
  printer.print(F("MANCHE N :"));
  printer.println(manche_rx);
  printer.print(F("ROUND N :"));
  printer.println(round_rx);

  printer.feed(1);
  printer.setSize('S');
  printer.println(F("Pilota ROSSO "));
  printer.print(F("NOME:"));
  printer.setSize('M');
  printer.print(nome_rosso);
  printer.feed(1);
  printer.setSize('S');
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
  printer.print(F("NOME:"));
  printer.setSize('M');
  printer.print(nome_verde);
  printer.feed(1);
  printer.setSize('S');
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
  printer.print(F("NOME:"));
  printer.setSize('M');
  printer.print(nome_blu);
  printer.feed(1);
  printer.setSize('S');
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
