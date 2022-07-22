#include "Arduino.h"
#include "LoRa_E22.h"

 
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
int relay = 9;


void setup() {
   pinMode(relay, OUTPUT);
  Serial.begin(9600);
  delay(500);
 
  Serial.println("Hi, I'm going to receive a message!");
 
  // Startup all pins and UART
  e22ttl.begin();
 
  // Send message
 // ResponseStatus rs = e22ttl.sendFixedMessage(0,1,5,"Hello, world?");
  // Check If there is some problem of succesfully send
 // Serial.println(rs.getResponseDescription());
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
        // Print the data received
           if (rc.data == "OK")  {
            Serial.println(" Masaggio Ricevuto ");
             digitalWrite(relay , HIGH );
             delay(1000);
            digitalWrite(relay, LOW );

           }      
        
    }
  }


 

 }
