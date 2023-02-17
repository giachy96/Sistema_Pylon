
//Arduino nRF24L01 comunicazione base

//Autore  : Andrea Lombardo
//Web     : http://www.lombardoandrea.com
//Post    : https://wp.me/p27dYH-Oi

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
RF24 radio(5, 4);

//Definisco quali saranno i pin per il LED e per il Potenziometro
int pinLed = 13;
int pinPot = 10;

//Creo due variabili nelle quali andrò a salvare lo stato del led ed il valore del potenziometro
boolean ledStatus;
int valPot;

void setup() {
  //Inizializzo la comunicazione seriale per il monitor
  Serial.begin(115200);
  //Inizializzo le due variabili
  ledStatus = false;
  valPot = 0;

  //Stabilisco la modalità dei pin
  pinMode(pinLed, OUTPUT);
  pinMode(pinPot, INPUT_PULLUP);

  //Spengo il led
  digitalWrite(pinLed, ledStatus);

  //Inizializzo la radio
  radio.begin();


  /*
     La radio può lavorare a diverse potenze: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH e RF24_PA_MAX
     Che corrispondono a: -18dBm, -12dBm,-6dBM, e 0dBm
  */
  //Setto la potenza della radio a LOW
  radio.setPALevel(RF24_PA_LOW);

  //Apro un canale di comunicazione sull'indirizzo specificato (sarà lo stesso per il ricevitore)
  // radio.openWritingPipe(indirizzo[0]);
  //adio.openWritingPipe(address[role]);


  //Richiamando questo metodo sto impostando la radio come trasmettitore
  // radio.stopListening();


  Serial.println(F("Avvio trasmevitore in rx"));


  radio.openReadingPipe(0, indirizzo[3]);

  radio.openWritingPipe(indirizzo[2]);

  radio.startListening();  // put radio in RX mode
}

void loop() {

  //Leggo il valore del potenziometro
  int readPot = digitalRead(pinPot);

  //Serial.println(readPot);
  //  if (readPot != valPot) {
  //    //salvo il nuovo valore nella variabile valPot
  //    valPot = readPot;
  //    //Invio il valore per radio
  //    radio.write(&valPot, sizeof(valPot));
  //  }

  if (readPot == 0) {
    valPot = 200;
    sendMessage(valPot);
  }
  
  if (radio.available()) {
    //Creo una variabile di appoggio
    int buff = 0;
    //Leggo i dati sul buffer e li salvo nella variabile di appoggio
    radio.read(&buff, sizeof(buff));
    //Invio al monitor seriale il valore appena letto
    Serial.println(buff);
  }
}

void sendMessage(int valore ) {

  radio.stopListening();  // put radio in TX mode
  //Invio il valore per radio
  radio.write(&valore, sizeof(valore));
  radio.startListening();  // put radio in RX mode

}
