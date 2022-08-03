
#include "Arduino.h"
#include "LoRa_E22.h"
#include "U8glib.h"
#include "lcdcases.h"
#include "timestructure.h"


U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
int var = 0;
int pulsante = 9;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int aggiornamento = 0;
int dcase = 0;
int sinc = 0;
int flagsend = 0;
int  codice = 200;
int secure  = 0; // evitare pressione doppia
int in = 0; // serve perchè senno il codice rimane nella sezione di reset del tasto
int avviato = 0; // definire e qualcuno è partito
int n_pressione = 0; // conto le volte che ho premuto il pulsante

// tempi
int lapcounter = -1;
unsigned long tempo_base;
unsigned long tempo_totale;
long tempo_parziale[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
long* poiparz = &tempo_parziale[0];
float tempo_flt[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int ultimo_tempo;
int ultimo_inviato = 0;
float* poitem = &tempo_flt[0];

LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;        // will store last time voltage was updated
unsigned long lastsend = 0;
unsigned long send  = 0;
unsigned long ultima_pressione  = 0;
unsigned long pressione  = 0;

// intervalli per la gestione del pulsante e del refresh:
long interval = 20000; // aggiornamento della tensione batteria

long intervallo_doppiapresione = 3000;

// ROBA DA CAMBIARE A SECONDA DEL COLORE
long intervallo_wait = 700; //intervallo di attesa prima della risposta
String col = "02"; // ROSSO

void setup() {
  pinMode(pulsante , INPUT_PULLUP);
  pinMode (buzzer , OUTPUT);
  pinMode (pinbatt , INPUT);
  Serial.begin(9600);
  delay(500);


  // Startup all pins and UART
  e22ttl.begin();
  u8g.begin();

  //  // Send message
  //  ResponseStatus rs = e22ttl.sendFixedMessage(0,1,23,"Hello, world?");
  //  // Check If there is some problem of succesfully send
  //  Serial.println(rs.getResponseDescription());
}

void loop() {
  currentMillis = millis();

  // fase di avvio
  while (bootup < 5) {
    previousMillis = currentMillis;
    // picture loop
    u8g.firstPage();
    do {
      draw(0, u8g);
    } while ( u8g.nextPage() );
    bootup++;
  }
  // fine fase di avvio


  while (!bootup < 5) {
    currentMillis = millis();
    pressione = millis() - ultima_pressione;
    send = millis() - lastsend;

    // allarme tensione
    if ( readvoltage(pinbatt) < 2.9 ) {
      tone(buzzer , 5000, 200);
    }
    // aggiornamento display

    if (currentMillis - previousMillis >= interval || aggiornamento == 1 ) {

      previousMillis = currentMillis;
      aggiornamento = 0;
      // picture loop
      u8g.firstPage();
      do {
        draw(dcase, u8g);

      } while ( u8g.nextPage() );

    }


    // lettura dati in RX
    if (e22ttl.available() > 1) {
      // read the  message
      ResponseContainer rc = e22ttl.receiveMessage();

      if (rc.data == "101") { // sincronizzazione
        lastsend = millis();
        sinc = 1;
        aggiornamento = 1; // per fare il refresh del display
        dcase = 1 ; // per scegliero lo screen del display
      }

      if (rc.data == "201" ) { //  SEND
        flagsend = 1;
        lastsend = millis();
        send = millis() - lastsend;
        Serial.println("Ricevuto");
        Serial.println(send);

      }
      if (rc.data == "200") { // SEND FIRST
        flagsend = 1;
        lastsend = millis();
        tempo_base = millis();
        avviato = 1;
      }
      if (rc.data == "500") { // STOP

        aggiornamento = 1; // per fare il refresh del display
        dcase = 4 ; // per scegliero lo screen del display
        n_pressione = 0;
        avviato = 0;
        send = 0;
        sinc = 0;
        lapcounter = -1;
        ultimo_inviato = 0;

        //asm volatile ("jmp 0"); // resetto la scheda
      }

      // Print the data received
      Serial.println(rc.data );



    }

    // reset variabile di sicurezza per la doppia pressione
    if (pressione >= intervallo_doppiapresione && in == 1  ) {
      secure = 0;
      in = 0;
      // Serial.println("DEbug"); //attenzione se pigiato per + 3s doppio comando ugualmente

    }

    if (digitalRead(pulsante) == LOW && secure == 0 && sinc == 1 && lapcounter <= 9  ) { // pulsante pigiato
      int index = lapcounter; // perche se metto il lapcounter sotto me lo manda a 1 e non capisco come mai
      Catturatempo ( poitem, poiparz, index ,  tempo_base); //balza l'indice a 1
      // Catturatempo (tempo_parziale ,  lapcounter , tempo_base); // ho paura dell indice lapcounter
      secure = 1;
      ultima_pressione = millis();
      in = 1;
      n_pressione++;
      lapcounter++;
      aggiornamento = 1;
      dcase = 2;
      tone(buzzer , 4000, 200);





    }

    if (n_pressione == 1 && avviato == 0 ) {  // SendFirst
      codice = 200;
      ResponseStatus rs = e22ttl.sendBroadcastFixedMessage(6, String(codice));
      tempo_base = millis();
      avviato = 1;
    }

    if (lapcounter == 10) {            //fine 10 giri calcolo il tempo totale
      float tot =  CalcoloTempo(tempo_flt);
      tempo_flt[10] = tot;
      tempo_parziale[10] = (int)(tot * 100); // per inserire il totale sulla stringa con virgole
      // Serial.println(conversionecommastr(col , poiparz));
      aggiornamento = 1; // per fare il refresh del display
      dcase = 3 ; // per scegliero lo screen del display
      lapcounter ++;
      Serial.println(tempo_flt[10]);
      Serial.println(tempo_parziale[10]);


    }

    if (send > intervallo_wait && flagsend == 1  && ultimo_inviato < 11  && n_pressione >= 1) { // invio i tempi
      String pack1;
      Serial.println(ultimo_inviato);
      Serial.println(lapcounter);
      if (ultimo_inviato < lapcounter ) {
        ultimo_tempo = tempo_parziale[ultimo_inviato];
        if (ultimo_inviato >= 10) {
          codice = 300; // codice del caso in cui mi trovo riportato nella legenda fine 10 giri
          pack1 = String(codice);
          pack1.concat(",") ;
          pack1.concat(col) ;
          pack1.concat(",") ;
          pack1.concat(String(ultimo_tempo));
          Serial.println(pack1);
        }
        else {
          codice = 201; // codice del caso in cui mi trovo riportato nella legenda
          pack1 = String(codice);
          pack1.concat(",") ;
          pack1.concat(col) ;
          pack1.concat(",") ;
          pack1.concat(String(ultimo_tempo));
        }


        ResponseStatus rs = e22ttl.sendBroadcastFixedMessage(6, pack1);
        ultimo_inviato++;
        flagsend = 0;
        send = 0;


      }
      else {

        ultimo_tempo = 999;
        codice = 201; // codice del caso in cui mi trovo riportato nella legenda
        pack1 = String(codice);
        pack1.concat(",") ;
        pack1.concat(col) ;
        pack1.concat(",") ;
        pack1.concat(ultimo_tempo);
        ResponseStatus rs = e22ttl.sendBroadcastFixedMessage(6, pack1);
        flagsend = 0;
        send = 0;
      }
      Serial.println("INVIATO");
      Serial.println(send);

    }

    if ( ultimo_inviato >= 11 ) { // una volta che mi ha chiamato al 10 giro  resetto
      //delay (3000);
      //asm volatile ("jmp 0"); // resetto la scheda
      aggiornamento = 1; // per fare il refresh del display
      dcase = 5 ; // per scegliero lo screen del display
      n_pressione = 0;
      avviato = 0;
      send = 0;
      sinc = 0;
      lapcounter = -1;
      ultimo_inviato = 0;

    }

    //Serial.println(lapcounter);
  } // fine routine

}// chiusura loop
