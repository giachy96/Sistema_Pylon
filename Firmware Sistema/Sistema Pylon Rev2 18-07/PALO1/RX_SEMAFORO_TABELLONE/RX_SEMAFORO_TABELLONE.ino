#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial AltSerial;

LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
String Race = "<300>";
String Show = "<200>";
String Startup = "<100>";
String State = Show; //Startup state
String TxData = "0";
const byte numChars = 32;
char RecCh[numChars];
boolean newData = false;
int lucerosso = 4;
int luceverde = 5;
int luceblu = 6;
int  ntaglirosso;
int  ntagliblu;
int  ntagliverde;
unsigned long lampmillisrosso ;
unsigned long lampmillisverde ;
unsigned long lampmillisblu ;
String valrx [4];
void setup() {
  pinMode(luceverde, OUTPUT);
  pinMode(lucerosso, OUTPUT);
  pinMode(luceverde, OUTPUT);

  Serial.begin(9600);
  delay(500);
  AltSerial.begin(9600);
  delay(500);
  e22ttl.begin();

}
void loop() {

  recvWithStartEndMarkers( RecCh );
  State = RecCh;

  if (newData == true) {
    digitalWrite( luceblu, LOW);
    digitalWrite( lucerosso, LOW);
    digitalWrite( luceverde, LOW);
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 1, 2, RecCh);
    newData = false;
  }
  if (e22ttl.available() > 1) { // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    TxData = rc.data; //Assign incoming data on TxData variable
    // Serial.println("Ricevo dal lora");
    Serial.println(TxData);

    if (TxData != "0" && (State == "200"  || State == "300" )) { //Condition for sent to mega press
      if (AltSerial.available() > 0) { // If mega will sent state to sem_rx
        recvWithStartEndMarkers( RecCh );
        State = RecCh;
      }
      if (State == "200"  || State == "300" ) { //if nothing has change from previous if-statements
        String pack  = "<";
        pack.concat(TxData);
        pack.concat(">");
        AltSerial.print(pack); //Send Txdata from rx to Mega
        // Serial.println("Mando via il dato");
        Serial.print(pack);
      }
      TxData = "0"; //Reset condition for set data
    }
  }

  if (TxData.indexOf("200") != -1 ) {
    decodecomma(TxData , valrx );
    ntaglirosso = 0;
    ntagliblu = 0;
    ntagliverde = 0;

    if (valrx[1] == "212") { // controllo se mi è arrivato lo show del verde
      digitalWrite( luceverde, HIGH);
    }
    if (valrx[2] == "222") { // controllo se mi è arrivato lo show del rosso
      digitalWrite( lucerosso, HIGH);
    }
    if (valrx[3] == "232") { // controllo se mi è arrivato lo show del blu
      digitalWrite( luceblu, HIGH);
    }
  }


  if (TxData.indexOf("300") != -1 ) {
    if (valrx[2] == "312") { // controllo se mi è arrivato il taglio del verde
      ntagliverde = ntagliverde + 1;
      if (ntagliverde == 1) {
        digitalWrite( luceverde, HIGH);
      }
      if (ntagliverde >= 1) {
        lampeggiante(luceverde, 1);
      }
    }
    if (valrx[1] == "322") { // controllo se mi è arrivato il taglio del rosso
      ntaglirosso = ntaglirosso + 1;
      if (ntaglirosso == 1) {
        digitalWrite( lucerosso, HIGH);
      }
      if (ntaglirosso >= 1) {
        lampeggiante(lucerosso, 2);
      }
    }
    if (valrx[3] == "332") { // controllo se mi è arrivato il taglio del blu
      ntagliblu = ntagliblu + 1;
      if (ntagliblu == 1) {
        digitalWrite( luceblu, HIGH);
      }
      if (ntagliblu >= 1) {
        lampeggiante(luceblu, 3);
      }
    }




  }


}


void lampeggiante (int pin , int colore ) {
  if (colore = 1) {
    if (millis() - lampmillisverde >= 1000) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    lampmillisverde = millis();
  } else if (colore = 2) {
    if (millis() - lampmillisrosso >= 1000) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    lampmillisrosso = millis();
  } else if (colore = 3) {
    if (millis() - lampmillisblu >= 1000) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    lampmillisblu = millis();
  }

}


int decodecomma (String str , String tempi[]) {
  int lungh_str = str.length();
  char buff[lungh_str + 1];
  str.toCharArray(buff, lungh_str + 1);
  //Serial.println(parseData(buff));
  int i = 0;
  char* p;
  //Serial.println("Example of splitting a string into tokens: ");
  // Serial.print("The input string is: '");
  // Serial.print(buff);
  // Serial.println("'");

  p = strtok(buff, "{,}"); //2nd argument is a char[] of delimiters
  while (p != '\0') { //not equal to NULL
    //Serial.println(p);
    tempi[i] = p;

    p = strtok('\0', "{,}");  //expects NULL for string on subsequent calls
    i++;
  }

  return tempi;
}



void  recvWithStartEndMarkers( char receivedChars[numChars] ) {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (AltSerial.available() > 0 && newData == false) {
    rc = AltSerial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
  AltSerial.flush();

}

void showNewData(char receivedChars[numChars]) {
  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}
