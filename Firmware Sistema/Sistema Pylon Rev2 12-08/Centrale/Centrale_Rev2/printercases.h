//#include "Adafruit_Thermal.h"
//#include "SoftwareSerial.h"

//#define TX_PIN 5// Arduino transmit  YELLOW WIRE  labeled RX on Stampanate
//#define RX_PIN 6// Arduino receive   GREEN WIRE   labeled TX on Stampanate

extern String arraytempirosso[12];
extern String arraytaglirosso[11];
extern String arraytempiverde[12];
extern String arraytagliverde[11];
extern String arraytempiblu[12];
extern String arraytagliblu[11];
extern String punteggiorosso;
extern String punteggioverde;
extern String punteggioblu;

//Adafruit_Thermal Serial3(&Serial3,5);      // Or Serial2, Serial3, etc.


void stampatotali (String gara , int manche_rx , int round_rx, String nome_rosso , String cognome_rosso , String nome_verde , String cognome_verde , String nome_blu , String cognome_blu ) {
  Serial3.write(0x1D); //altezza grande ON
  Serial3.write(0x21);
  Serial3.write(0x01); // fine

  Serial3.print("GARA ");
  Serial3.println(gara);
  Serial3.print("MANCHE N :");
  if (manche_rx != 0) {
    Serial3.print(manche_rx);
  }
  Serial3.println("");
  Serial3.print("ROUND N :");
  if (round_rx != 0) {
    Serial3.print(round_rx);
  }
  Serial3.println("");
  Serial3.println("Pilota ROSSO ");
  Serial3.print("NOME:");
  Serial3.print(nome_rosso);
  Serial3.print(" ");
  Serial3.print(cognome_rosso);
  Serial3.println(" ");
  Serial3.write(0x1D);// altezza grande OFF
  Serial3.write(0x21);
  Serial3.write(0x00);//fine
  Serial3.write(0x1B); // grassetto on
  Serial3.write(0x45);
  Serial3.write(0x01);//fine
  for (int y = 1 ; y < 11; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y);
    Serial3.print(" T: ");
    Serial3.print(arraytempirosso[y]);
    Serial3.print("s     ");
    Serial3.print("[");
    Serial3.print(arraytaglirosso[y - 1]);
    Serial3.println("]");
  }
  Serial3.write(0x1B); // grassetto oFF
  Serial3.write(0x45);
  Serial3.write(0x00);//fine
  Serial3.write(0x1D); //altezza grande ON
  Serial3.write(0x21);
  Serial3.write(0x01); // fine
  Serial3.print("Tempo Totale: ");
  Serial3.print(arraytempirosso[11]);
  Serial3.print("s");
  Serial3.println(" ");
  Serial3.print("PUNTEGGIO: ");
  Serial3.print(punteggiorosso);




  Serial3.println(" ");
  Serial3.println(" ");



  Serial3.println("Pilota VERDE ");
  Serial3.print("NOME:");
  Serial3.print(nome_verde);
  Serial3.print(" ");
  Serial3.print(cognome_verde);
  Serial3.println(" ");
  Serial3.write(0x1D);//  altezza grande OFF
  Serial3.write(0x21);
  Serial3.write(0x00);//fine
  Serial3.write(0x1B); // grassetto on
  Serial3.write(0x45);
  Serial3.write(0x01);//fine

  for (int y = 1 ; y < 11; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y);
    Serial3.print(" T: ");
    Serial3.print(arraytempiverde[y]);
    Serial3.print("s     ");
    Serial3.print("[");
    Serial3.print(arraytagliverde[y - 1]);
    Serial3.println("]");
  }
  Serial3.write(0x1B); // grassetto oFF
  Serial3.write(0x45);
  Serial3.write(0x00);//fine
  Serial3.write(0x1D); //altezza grande ON
  Serial3.write(0x21);
  Serial3.write(0x01); // fine
  Serial3.print("Tempo Totale: ");
  Serial3.print(arraytempiverde[11]);
  Serial3.print(F("s"));
  Serial3.println(" ");
  Serial3.print("PUNTEGGIO: ");
  Serial3.print(punteggioverde);


  Serial3.println(" ");
  Serial3.println(" ");


  Serial3.println("Pilota BLU ");
  Serial3.print("NOME:");
  Serial3.print(nome_blu);
  Serial3.print(" ");
  Serial3.print(cognome_blu);
  Serial3.println(" ");
  Serial3.write(0x1D); //altezza grande OFF
  Serial3.write(0x21);
  Serial3.write(0x00); // fine
  Serial3.write(0x1B); // grassetto on
  Serial3.write(0x45);
  Serial3.write(0x01);//fine
  for (int y = 1 ; y < 11; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y);
    Serial3.print(" T: ");
    Serial3.print(arraytempiblu[y]);
    Serial3.print("s     ");
    Serial3.print("[");
    Serial3.print(arraytagliblu[y - 1]);
    Serial3.println("]");
  }
  Serial3.write(0x1B); // grassetto oFF
  Serial3.write(0x45);
  Serial3.write(0x00);//fine
  Serial3.write(0x1D); //altezza grande ON
  Serial3.write(0x21);
  Serial3.write(0x01); // fine
  Serial3.print("Tempo Totale: ");
  Serial3.print(arraytempiblu[11]);
  Serial3.print("s");
  Serial3.println(" ");
  Serial3.print("PUNTEGGIO: ");
  Serial3.print(punteggioblu);
  Serial3.println(" ");
  Serial3.write(0x1D); //altezza grande OFF
  Serial3.write(0x21);
  Serial3.write(0x00); // fine


  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");


}
