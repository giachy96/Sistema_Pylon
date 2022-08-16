#include "Arduino.h"
#include "LoRa_E22.h"
#include <AltSoftSerial.h>
AltSoftSerial AltSerial;
LoRa_E22 e22ttl(2, 3); // Arduino RX --> e22 TX - Arduino TX --> e22 RX

//Inizio configurazione
String Race = "3000";
String StartRace = "3001";
String Show = "2000";
String Startup = "1000";
String State = Show; //Startup state
String OldState;
String PressShowR = "2110";
String PressRaceR = "3112";
String PressCutR = "3111";
String end10lapR = "5514";
String PressShowV = "2120";
String PressRaceV = "3122";
String PressCutV = "3121";
String end10lapV = "5524";
String PressShowB = "2130";
String PressRaceB = "3132";
String PressCutB = "3131";
String end10lapB = "5534";
//Fine Configurazione

String TxData = "0";
String TxDatastr;
const byte numChars = 32;
char RecCh[numChars];
boolean newData = false;
int lucerosso = 4;
int luceverde = 5;
int luceblu = 6;
int  ntaglirosso;
int  ntagliblu;
int  ntagliverde;
int lampeggianterosso = 0;
int onrosso = 0;
int lampeggianteverde = 0;
int onverde = 0;
int lampeggianteblu = 0;
int onblu = 0;
int FlagState = 0;
unsigned long lampmillisrosso ;
unsigned long lampmillisverde ;
unsigned long lampmillisblu ;
String valrx [5];
void setup() {
  pinMode(luceverde, OUTPUT);
  pinMode(lucerosso, OUTPUT);
  pinMode(luceblu, OUTPUT);

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
    TxDatastr = "";
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 1, 10, RecCh);
    newData = false;
     FlagState = 1;
  }
  if (e22ttl.available() > 1) { // If there is something arrived from lora (Transmitter)
    ResponseContainer rc = e22ttl.receiveMessage();// Receive message
    TxData = rc.data; //Assign incoming data on TxData variable
    // Serial.println("Ricevo dal lora");
    Serial.println(TxData);

    if (TxData != "0" && (State == Show  || State == StartRace  )) { //Condition for sent to mega press
      if (AltSerial.available() > 0) { // If mega will sent state to sem_rx
        recvWithStartEndMarkers( RecCh );
        State = RecCh;
        FlagState = 1;
      }
      if (State == Show  || State == StartRace ) { //if nothing has change from previous if-statements
        String pack  = "<";
        pack.concat(TxData);
        pack.concat(">");
        AltSerial.print(pack); //Send Txdata from rx to Mega
        // Serial.println("Mando via il dato");
        Serial.println(pack);
      }
      TxDatastr = TxData;
      TxData = "0"; //Reset condition for set data
    }
  }


  if (TxDatastr.indexOf(Show) != -1 ) {
    decodecomma(TxDatastr , valrx );
      Serial.println(valrx[3]);



    if (valrx[1] == PressShowR) { // controllo se mi è arrivato lo show del rosso
      digitalWrite( lucerosso, HIGH);

    }
    if (valrx[2] == PressShowV) { // controllo se mi è arrivato lo show del verde
      digitalWrite( luceverde, HIGH);
    }
    if (valrx[3] == PressShowB) { // controllo se mi è arrivato lo show del blu
      digitalWrite( luceblu, HIGH);
      
    }
  }


  if (TxDatastr.indexOf(StartRace) != -1 ) {
    decodecomma(TxDatastr , valrx );
    if (valrx[2] == PressCutV) { // controllo se mi è arrivato il taglio del verde
      ntagliverde = ntagliverde + 1;
      if (ntagliverde == 1) {
        digitalWrite( luceverde, HIGH);
      }
      if (ntagliverde > 1) {
        lampeggianteverde = 1;
      }
    }
    if (valrx[1] == PressCutR) { // controllo se mi è arrivato il taglio del rosso

      ntaglirosso = ntaglirosso + 1;
      if (ntaglirosso == 1) {
        digitalWrite( lucerosso, HIGH);

      }
      if (ntaglirosso > 1) {
        lampeggianterosso = 1;
        Serial.println("ci entro?");
      }
      valrx[1] = "";
    }
    if (valrx[3] == PressCutB) { // controllo se mi è arrivato il taglio del blu
      ntagliblu = ntagliblu + 1;
      if (ntagliblu == 1) {
        digitalWrite( luceblu, HIGH);
      }
      if (ntagliblu > 1) {
        lampeggianteblu = 1;
      }
    }
    TxDatastr = "";
  }

  if (FlagState == 1) { //If state was change on previous code

    digitalWrite( luceblu, LOW);
    digitalWrite( lucerosso, LOW);
    digitalWrite( luceverde, LOW);
    ntaglirosso = 0;
    ntagliblu = 0;
    ntagliverde = 0;
    lampeggianterosso = 0;
    lampeggianteverde = 0;
    lampeggianteblu = 0;
    TxDatastr = "";
    FlagState = 0;
  }

  //codici per il lampeggio
  if (lampeggianterosso == 1) {
    if (onrosso == 0 && millis() - lampmillisrosso >= 500) {
      lampmillisrosso = millis();
      onrosso = 1;
      digitalWrite( lucerosso, HIGH);

    }
    if (onrosso == 1 && millis() - lampmillisrosso >= 500 ) {
      digitalWrite( lucerosso, LOW);
      onrosso = 0;
      lampmillisrosso = millis();

    }
  }

  if (lampeggianteverde == 1) {
    if (onverde == 0 && millis() - lampmillisverde >= 500) {
      lampmillisverde = millis();
      onverde = 1;
      digitalWrite( luceverde, HIGH);

    }
    if (onverde == 1 && millis() - lampmillisverde >= 500 ) {
      digitalWrite( luceverde, LOW);
      onverde = 0;
      lampmillisverde = millis();

    }
  }
  if (lampeggianteblu == 1) {
    if (onblu == 0 && millis() - lampmillisblu >= 500) {
      lampmillisblu = millis();
      onblu = 1;
      digitalWrite( luceblu, HIGH);

    }
    if (onblu == 1 && millis() - lampmillisblu >= 500 ) {
      digitalWrite( luceblu, LOW);
      onblu = 0;
      lampmillisblu = millis();

    }
  }
  // fine codici per il lampeggio
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
