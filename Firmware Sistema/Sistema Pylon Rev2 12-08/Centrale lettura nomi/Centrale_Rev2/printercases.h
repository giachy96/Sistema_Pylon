//#include "Adafruit_Thermal.h"
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

//Adafruit_Thermal Serial3(&Serial3,5);      // Or Serial2, Serial3, etc.


void stampatotali (String gara , int manche_rx , int round_rx, String nome_rosso , String nome_verde  , String nome_blu ) {

  Serial3.print(F("GARA "));
  Serial3.println(gara);
  Serial3.println(" ");
  Serial3.print("MANCHE N :");
  Serial3.println(manche_rx);
  Serial3.print("ROUND N :");
  Serial3.println(round_rx);

  Serial3.println(" ");
  Serial3.println("Pilota ROSSO ");
  Serial3.print("NOME:");
  Serial3.print(nome_rosso);
  Serial3.println(" ");
  for (int y = 0 ; y < 10; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y + 1);
    Serial3.print("  Tempo: ");
    Serial3.print(arraytempirosso[y]);
    Serial3.print("s     ");
    Serial3.println("[]");
  }
  Serial3.print("Tempo Totale: ");
  Serial3.print(arraytempirosso[10]);
  Serial3.print("s");
  Serial3.println(" ");
  Serial3.print("Numero Tagli: ");
  Serial3.println(" ");
  Serial3.print("PUNTEGGIO: ");




  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println("Pilota VERDE ");
  Serial3.print("NOME:");
  Serial3.print(nome_verde);
  Serial3.println(" ");
  for (int y = 0 ; y < 10; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y + 1);
    Serial3.print("  Tempo: ");
    Serial3.print(arraytempiverde[y]);
    Serial3.print("s     ");
    Serial3.println("[]");
  }
  Serial3.print("Tempo Totale: ");
  Serial3.print(arraytempiverde[10]);
  Serial3.print(F("s"));
  Serial3.println(" ");
  Serial3.print("Numero Tagli: ");
  Serial3.println(" ");
  Serial3.print("PUNTEGGIO: ");


  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println("Pilota BLU ");
  Serial3.print("NOME:");
  Serial3.print(nome_blu);
  Serial3.println(" ");
  for (int y = 0 ; y < 10; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y + 1);
    Serial3.print("  Tempo: ");
    Serial3.print(arraytempiblu[y]);
    Serial3.print("s     ");
    Serial3.println("[]");
  }
  Serial3.print("Tempo Totale: ");
  Serial3.print(arraytempiblu[10]);
  Serial3.print("s");
  Serial3.println(" ");
  Serial3.print("Numero Tagli: ");
  Serial3.println(" ");
  Serial3.print("PUNTEGGIO: ");

  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");


}
