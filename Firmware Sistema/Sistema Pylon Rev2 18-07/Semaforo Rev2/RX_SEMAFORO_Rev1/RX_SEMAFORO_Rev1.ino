#include "Arduino.h"
#include "LoRa_E22.h"
#include <SoftwareSerial.h> //MT - include softwareserial library
 
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
#define SOFTRX 10 // MT - define rx pin 10
#define SOFTTX 11 // MT - define tx pin 11
SoftwareSerial SwSerial(SOFTRX,SOFTTX);

void setup() {
     pinMode(SOFTTX, OUTPUT); // MT - Softrx - pin mapping output
   pinMode(SOFTRX, INPUT); // MT - Softrx - pin mapping input
  Serial.begin(9600);
  delay(500); 
  SwSerial.begin(9600);
  delay(500);
 
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
             SwSerial.println(rc.data);
    }
  }
 }
