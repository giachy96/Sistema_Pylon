#include "Adafruit_Thermal.h"
//#include "SoftwareSerial.h"

//#define TX_PIN 5// Arduino transmit  YELLOW WIRE  labeled RX on Stampanate
//#define RX_PIN 6// Arduino receive   GREEN WIRE   labeled TX on Stampanate

extern String nome_rosso ;

extern String arraytempirosso[11];
extern String arraytaglirosso[10];
extern String arraytempiverde[11];
extern String arraytagliverde[10];
extern String arraytempiblu[11];
extern String arraytagliblu[10];

Adafruit_Thermal printer(&Serial3,5);      // Or Serial2, Serial3, etc.


void provstampa() {
  printer.setSize('M');        // Set type size, accepts 'S', 'M', 'L'
  printer.print(F("GARA "));

  printer.feed(1);
  printer.print(F("MANCHE N :"));

  printer.print(F("ROUND N :"));


}
void stampatotali (String gara , int manche_rx , int round_rx, String nome_rosso , String nome_verde  , String nome_blu ) {
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
    printer.print(arraytempirosso[y]);
    printer.print("s     ");
    printer.println("[]");
  }
  printer.print(F("Tempo Totale: "));
  printer.print(arraytempirosso[10]);
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
    printer.print(arraytempiverde[y]);
    printer.print("s     ");
    printer.println("[]");
  }
  printer.print(F("Tempo Totale: "));
  printer.print(arraytempiverde[10]);
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
    printer.print(arraytempiblu[y]);
    printer.print("s     ");
    printer.println("[]");
  }
  printer.print(F("Tempo Totale: "));
  printer.print(arraytempiblu[10]);
  printer.print(F("s"));
  printer.println(F(" "));
  printer.print("Numero Tagli: ");

  printer.feed(3);


}
