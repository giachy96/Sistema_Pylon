/*
   Arduino nRF24L01 comunicazione base

   Autore  : Andrea Lombardo
   Web     : http://www.lombardoandrea.com
   Post    : https://wp.me/p27dYH-Oi
*/
//Inclusione delle librerie
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>


//definizione indirizzo sul quale stabilire la comunicazione
const byte indirizzo[6] = { 0x7878787878LL, // telecomando rosso in TX
                            0xB3B4B5B6F1LL,// telecomando rosso in RX
                            0xB3B4B5B6CDLL,// telecomando verde in TX
                            0xB3B4B5B6A3LL,// telecomando verde in RX
                            0xB3B4B5B60FLL,
                            0xB3B4B5B605LL
                          };

//Creo un'istanza della "radio" passandogli il numero dei pin collegati a CE e CSN del modulo
RF24 radio(7, 8);

int relay = 10;

unsigned long lastime;

void setup() {

  //Inizializzo la comunicazione seriale per il monitor
  Serial.begin(9600);


  pinMode(relay, OUTPUT);


  Serial.println(F("Avvio ricevitore"));

  //Inizializzo la radio
  radio.begin();

  /*
     La radio può lavorare a diverse potenze: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH e RF24_PA_MAX
     Che corrispondono a: -18dBm, -12dBm,-6dBM, e 0dBm
  */
  //Setto la potenza della radio a LOW
  radio.setPALevel(RF24_PA_LOW);

  //Apro un canale in lettura sull'indirizzo specificato
  radio.openReadingPipe(0, indirizzo[0]);
  radio.openReadingPipe(1, indirizzo[2]);



  //Metto la radio in ascolto
  radio.startListening();

  Serial.println(F("Radio in ascolto!"));

}

void loop() {

  //Se ci sono dati in ricezione sulla radio
  if (radio.available()) {
    //Creo una variabile di appoggio
    char buff[50]; // this must match dataToSend in the TX
    //Leggo i dati sul buffer e li salvo nella variabile di appoggio
    radio.read( &buff, sizeof(buff) );
    //Invio al monitor seriale il valore appena letto
    String str = String(buff);
    Serial.println(str);


    //    if (buff == 0) {
    //      digitalWrite(relay, HIGH);
    //      Serial.print("Stato relay : ");
    //      Serial.println(buff);
    //      delay(1000);
    //      digitalWrite(relay, LOW);
    //      Serial.print("Stato relay : ");
    //      Serial.println(buff);
    //
    //    }
  }

  if (millis() - lastime > 5000) {
     int valore = 500;
    radio.stopListening();  // put radio in TX mode

    radio.openWritingPipe(indirizzo[3]);
    //Invio il valore per radio B
   
    Serial.println("mando 500 B");
    radio.write(&valore, sizeof(valore));

     radio.openWritingPipe(indirizzo[4]);
    //Invio il valore per radio A
    Serial.println("mando 500 A");
    radio.write(&valore, sizeof(valore));
    
    radio.startListening();  // put radio in RX mode
    lastime = millis();

  }

}
