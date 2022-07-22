#include "Arduino.h"
#include "LoRa_E22.h"

 
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
const unsigned int MAX_MESSAGE_LENGTH = 12;
char message[MAX_MESSAGE_LENGTH];
unsigned int message_pos = 0;
void setup() {
 
  Serial.begin(9600);
  delay(500); 
  Serial.println("Hi, I'm going to receive a message!");
 
  // Startup all pins and UART
  e22ttl.begin();
 
 
}
 
void loop() {
 
    // If something available
  if (e22ttl.available()>1) {
      // read the String message
    ResponseContainer rc = e22ttl.receiveMessage();
    // Is something goes wrong print error
    if (rc.status.code!=1){
        rc.status.getResponseDescription();
    }else{ 
         Serial.println(rc.data);             
        
    }
  }

 //Controllo se ci sta qualcosa nella seriale 
 if (Serial.available() > 0) {
   //Creo una variabile dove immagazzinare il dato


   //Leggo il prossimo byte disponibile nel buffer della seriale
   char inByte = Serial.read();

   //Messagio in arrivo  (controllo ch eil carattere sia terminato ) e guardo se il messaggio supera la diensione
   if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) )
   {
     //e aggiungo il byte in arrivo ai precedenti per completare il messaggio 
     message[message_pos] = inByte;
     message_pos++;
   }
   //ho ricevuto l'intero messaggio 
   else
   {
     //aggiungo un carettere nullo alla stringa
     message[message_pos] = '\0';

     //Stampo il messaggio 
     Serial.println(message);
     ResponseStatus rs = e22ttl.sendFixedMessage(0, 0, 7, message);


     //resetto per il prossimo messaggio 
     message_pos = 0;
   }
 }


 

 }
