/*
   LoRa E22
   Write on serial to transfer a message to other device
   https://www.mischianti.org

   E22        ----- Arduino UNO
   M0         ----- GND
   M1         ----- GND
   TX         ----- PIN 2 (PullUP)
   RX         ----- PIN 3 (PullUP & Voltage divider)
   AUX        ----- Not connected
   VCC        ----- 3.3v/5v
   GND        ----- GND

*/
#include "U8x8lib.h"
#include "Arduino.h"
#include "LoRa_E22.h"
#include "lcdcasesp2r.h"
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
int taglio = 9;
int buzzer = 5;
int pinbatt = A0;
int bootup = 1;
int dcase = 0;
int altupdatelcd = 0;
char t[5] = { '3', ',', '3', '3',  '\0' };
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
unsigned long previousMillis = 0;        // will store last time voltage was updated
unsigned long currentMillis = 0;
unsigned long CurrentPress=0;
unsigned long Lastpress=0;
unsigned long Delaypress=2500;
unsigned long DelaySend=200;
unsigned long TimeSend=0;
unsigned long Freq=500;
unsigned long Phase=0;
unsigned long WindowTransm=150;
unsigned long Sync=0;
int Transm=0;
int Press=0;
int Pressr=0;
int changeState=0;
String Race="300";
String Show="200";
String End="600";
String Startup="100";
String State=Startup; //Startup state
unsigned long interval = 10000;// constants won't change:

void setup() {
  pinMode(taglio , INPUT_PULLUP);
  pinMode (buzzer , OUTPUT);
  pinMode (pinbatt , INPUT);
  pinMode (taglio , INPUT_PULLUP);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();  // Startup all pins and UART
  u8x8.begin();
  u8x8.setPowerSave(0);
  Serial.println("start");
  Serial.println(State);
}

void loop() {

 currentMillis = millis();

  while (bootup < 2) {
    //previousMillis = currentMillis;
    Serial.println("Bootup Case");
    draw(0, u8x8);
    bootup++;
  }

  while (bootup >= 2) {
    float voltage = readvoltage(pinbatt);
    if (voltage < 2.9) {
      digitalWrite(buzzer , HIGH);
    }
    currentMillis = millis();
    if ((((currentMillis - previousMillis)>=interval) || changeState==1) && State==Startup) { // Casistica per display, se non siamo in Show, Race o End, metti la schermata 0 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate  è attivo
      previousMillis = currentMillis;
      Serial.println("Case0");
      draw(0, u8x8);
      changeState=0;
    }
    currentMillis = millis();
    if ((((currentMillis - previousMillis)>=interval) || changeState==1) && State==Show) { // Casistica per display, se siamo in Show metti la schermata 1 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate è attivo
      previousMillis = currentMillis;
      Serial.println("Case1");
      draw(1, u8x8);
      changeState=0;
    }
    currentMillis = millis();
    if (changeState==1 && State==Race) { // Casistica per display, se siamo in Race metti la schermata 2 e aggiorna solo quando il flag changestate è attivo
      previousMillis = currentMillis;
      Serial.println("Case2");
      draw(2, u8x8);
      changeState=0;
    }
    currentMillis = millis();
    if ((((currentMillis - previousMillis)>=interval) || changeState==1) && State==End) { // Casistica per display, se siamo in End metti la schermata 6 e aggiorna ogni 5 sec, o aggiorna quando il flag changestate è attivo
      previousMillis = currentMillis;
      draw(6, u8x8);
      Serial.println("Case6");
      changeState=0;
    }
    if (e22ttl.available()>1){ 
      Serial.println("ricevo qualcosa");
      ResponseContainer rc = e22ttl.receiveMessage();// Receive message
      if (rc.status.code!=1){ // If there is some problem
        rc.status.getResponseDescription(); //Get report
        }
      else { //If there isn't any problem we're going to receive press 
        State=rc.data; //Assign incoming data on State variable
        changeState=1; //attivo il flag changeState=1, lo uso sopra per il primo aggiornamento display
        TimeSend=millis(); 
      Sync=(millis()+Phase+2*Freq); //prendo il tempo per la sincronia di invio, millis(tempo corrente) inserisco la fase (0 per il primo, Freq/2 per per il secondo) e mi metto in sicurezza aggiungendo due volte la frequenza
      }
    } 
      CurrentPress=millis();
      if (digitalRead(taglio) == LOW && (CurrentPress-Lastpress)>=Delaypress && State==Show) { // Se sono in show, è passato il tempo di sicurezza e il bottone è premuto
      Lastpress=millis(); // memorizzo il tempo della pressione
      tone(buzzer, 1000, 200); //invio il tono
      Press=1; //metto il flag su Press
    }  
      CurrentPress=millis();
      if (digitalRead(taglio) == LOW && (CurrentPress-Lastpress)>=Delaypress && State==Race) { // Se sono in race, è passato il tempo di sicurezza e il bottone è premuto
      Lastpress=millis();// memorizzo il tempo della pressione
      tone(buzzer, 1000, 200);//invio il tono
      Pressr=1;//metto il flag su Pressr
    }  
    currentMillis=millis();
if (State==Show || State==Race){ // Ogni volta che ricevo un nuovo stato memorizzo un millis che mi serve come sincronismo (Sync) , elaboro la conferma
  if (currentMillis<Sync){ // se ancora non è arrivato il momento di trasmettere assegno il flag Transm a zero
  Transm=0;
  }
  else if (currentMillis>=Sync && (currentMillis-Sync)<= WindowTransm){ // Se è arrivato il momento di trasmettere, ovvero ho appena superato il tempo Sync e sono nella finestra temporale da Sync a Sync+150ms
    Transm=1; // Inserisco il flag di trasmissione
    Sync=Sync+Freq; // incremento il valore di Sync aggiungendo la frequenza
    }
  }
  else if (currentMillis>=Sync && (currentMillis-Sync)> WindowTransm){ // Se è arrivato il momento di trasmettere ma non siamo più nella finestra temporale di sicurezza ovvero Sync/Sync+150
    Transm=0; // non permettere di trasmettere
    while (currentMillis>Sync) { //Incrementa il valore di Sync con la frequenza finchè Sync non è di nuovo maggiore di currentmillis (sicurezza forse inutile se il codice impunta previene la trasmissione sovrapposta)
    Sync=Sync+Freq;
    }
  }
    currentMillis=millis();
    if (Press==1 && Transm==1 && State==Show && (currentMillis-TimeSend)>DelaySend){ //se sono in Show, se ho il flag della pressione di show e se ho consenso alla trasmissione con Transm
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 9,"<211a>"); //mando il messaggio
    Press=0; //azzero il flag pressione di show
    }
    else if (Press==1 && State !=Show){ //se ho il flag di pressione ma nel frattempo è cambiato lo stato togli il flag della pressione
      Press=0;
    }
    if (Pressr==1 && Transm==1 && State==Race && (currentMillis-TimeSend)>DelaySend){//se sono in Race, se ho il flag della pressione di Race e se ho consenso alla trasmissione con Transm
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 9,"<311a>");//mando il messaggio
    Pressr=0;//azzero il flag pressione di Race
    }
    else if (Pressr==1 && State !=Race){//se ho il flag di pressione ma nel frattempo è cambiato lo stato togli il flag della pressione
      Pressr=0;
    }
}
}
