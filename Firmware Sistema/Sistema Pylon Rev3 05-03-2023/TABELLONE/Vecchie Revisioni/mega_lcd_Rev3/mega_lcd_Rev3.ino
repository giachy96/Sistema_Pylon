#include "exceldecode.h"
#include "codedecode.h"
#include "dmdcases.h"
#include "LoRa_E22.h"
#include "countdown.h"
LoRa_E22 e22ttl(&Serial2);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX

const byte numChars = 200;
char receivedChars1[numChars];
boolean newData = false;
String msg;
int updatescreen = 0;
/*
  Pin Defination
  nOE - 11
  A - 6
  B - 7
  CLK - 52
  SCLK - 8
  Data - 51
*/

//Fire up the DMD library as dmd

unsigned long currentMillis;
unsigned long lastdiplayupdate;
unsigned long oldPress;
unsigned long intervalPress = 2000;
unsigned long intervalstandby = 5000;
unsigned long milliscountdown;
unsigned long sirenamillis;
unsigned long startsirena;
unsigned long  lastsend;
unsigned long intervalsend = 300;

int sec;
int sirena = 2;
int pulsanteAvanti = 11;
int pulsanteIndietro = 10;
bool flagcount;
bool showcroverde;
bool showcrorosso;
bool showcroblu;
String temporosso[5];
String tempoblu[5];
String tempoverde[5];
String values[5];
String arraytempirosso[12];
String arraytaglirosso[12];
String arraytempiverde[12];
String arraytagliverde[12];
String arraytempiblu[12];
String arraytagliblu[12];
String tempototrosso;
String tempototverde;
String tempototblu;
String codestringone;
int end10laprosso;
int end10lapverde;
int end10lapblu;
int doppiotaglioverde;
int doppiotagliorosso;
int doppiotaglioblu;
int timeoutblu = 0;
int timeoutverde = 0;
int timeoutrosso = 0;
extern String nome_rosso;
extern String cognome_rosso;
extern String nome_verde;
extern String cognome_verde;
extern String nome_blu;
extern String cognome_blu;
String Staterx;
String RxData;
String stringonerosso;
String stringoneblu;
String stringoneverde;
int sendcentraleverde = 0;
int sendcentralerosso = 0;
int sendcentraleblu = 0;
int count_send = 0;


byte Key = 0;
byte Add = 1;
byte Chan = 5;





void setup() {

  pinMode(sirena, OUTPUT);
  pinMode(pulsanteAvanti, INPUT_PULLUP);
  pinMode(pulsanteIndietro, INPUT_PULLUP);
  dmd.setBrightness(255);
  // dmd.selectFont(SystemFont5x7);
  dmd.selectFont(Arial_Black_16);
  dmd.begin();
  delay(500);
  Serial.begin(9600);
  delay(200);
  Serial1.begin(9600);  // SERIALE CON IL TEENSY
  delay(200);
  Serial2.begin(9600);  // seriale per il lora
  e22ttl.begin();       //Start e22ttl
}


void loop() {
  currentMillis = millis();
  recvWithStartEndMarkers(Serial1, receivedChars1);

  // se ho tutti i 3 stringoni li mando alla centrale con 200 ms di intervallo
  if ( sendcentraleverde == 1 &&  sendcentralerosso == 1 &&  sendcentraleblu == 1) {

    if ((currentMillis - lastsend) >= intervalsend) {

      switch (count_send) {
        case 0:
          e22ttl.sendFixedMessage(Key, Add, Chan, stringonerosso);
          count_send++;
          Serial.println("mando lo stringone rosso");
          lastsend = millis();
          break;
        case 1:
          e22ttl.sendFixedMessage(Key, Add, Chan, stringoneverde);
          count_send++;
          Serial.println("mando lo stringone verde");
          lastsend = millis();
          break;
        case 2:
          e22ttl.sendFixedMessage(Key, Add, Chan, stringoneblu);
          count_send++;
          Serial.println("mando lo stringone blu");
          lastsend = millis();
          break;
        case 3:
          count_send = 0;
          sendcentraleverde = 0;
          sendcentralerosso = 0;
          sendcentraleblu = 0;
          break;
        default:
          break;
      }
    }
  }


  //INIZO PARTE RICEZIONE DAL LORA
  if (e22ttl.available() > 1) {
    // read the String message
    ResponseContainer rc = e22ttl.receiveMessage();
    RxData = rc.data;
    Serial.print("Ricevo DATO dal lora: ");
    Serial.println(rc.data);
  }

  if (digitalRead(pulsanteIndietro) == LOW && currentMillis - oldPress >= intervalPress) {  // se pigio il pulsante indietro
    Serial1.println("<6000>"); // pima mando lo stop per fermare tutto
    delay(2000);
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "700");
    oldPress = millis();
  }
  if (digitalRead(pulsanteAvanti) == LOW && currentMillis - oldPress >= intervalPress) {  // se pigio il pulsante Avanti
    Serial1.println("<6000>"); // pima mando lo stop per fermare tutto
    delay(2000);
    ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "800");
    oldPress = millis();
  }
  if (RxData == "2000") {  // se ricevo lo show dalla centrale
    Staterx = RxData;
    Serial1.println("<2000>");
    //sirena
    startsirena = millis();
    digitalWrite(sirena, HIGH);
    RxData = "";
  }
  if (RxData == "3000") {  // se ricevo GO dalla centrale
    Staterx = RxData;
    Serial1.println("<3000>");
    //sirena
    startsirena = millis();
    digitalWrite(sirena, HIGH);
    RxData = "";
  }
  if (RxData == "6000") {  // se ricevo STOP dalla centrale
    Staterx = RxData;
    Serial1.println("<6000>");
    //sirena
    startsirena = millis();
    digitalWrite(sirena, HIGH);
    RxData = "";
  }
  if (RxData.indexOf("750") != -1 || RxData.indexOf("850") != -1) {  // se ricevo dalla centrale una striga CONTENTE AVANTI o INDIETRO
    Serial1.println("<850>");
    Staterx = RxData;
    end10lapblu = 0;
    end10lapverde = 0;
    end10laprosso = 0;
    doppiotaglioverde = 0;
    doppiotagliorosso = 0;
    doppiotaglioblu = 0;
    timeoutblu = 0;
    timeoutverde = 0;
    timeoutrosso = 0;
    showcroverde = false;
    showcrorosso = false;
    showcroblu = false;
    stringonerosso = "";
    stringoneblu = "";
    stringoneverde = "";
    sendcentraleverde = 0;
    sendcentralerosso = 0;
    sendcentraleblu = 0;
    count_send = 0;
    splitCommaSeparated(RxData);
    // Serial.println(nome_rosso);
    // Serial.println(cognome_rosso);
    // Serial.println(nome_verde);
    // Serial.println(cognome_verde);
    // Serial.println(nome_blu);
    // Serial.println(cognome_blu);
    draw(0, 0, 0);
    RxData = "";
  }

  // FINE PARTE RICEZIONE DAL LORA

  // inzio la lettura dalla seriale del teesny

  if (newData == true) {  // se ricevo un nuovo dato in seriale
    String Rxs;
    Rxs = receivedChars1;
    Serial.print("Ricevo dato dal Teensy: ");
    Serial.println(Rxs);
    //  Serial.println(Rxs);
    updatescreen = 0;

    if (Rxs.indexOf("STOP") != -1 && updatescreen == 0) {  // se riveco STOP dal TEENSY
      end10lapblu = 0;
      end10lapverde = 0;
      end10laprosso = 0;
      doppiotaglioverde = 0;
      doppiotagliorosso = 0;
      doppiotaglioblu = 0;
      timeoutblu = 0;
      timeoutverde = 0;
      timeoutrosso = 0;
      showcroverde = false;
      showcrorosso = false;
      showcroblu = false;
      stringonerosso = "";
      stringoneblu = "";
      stringoneverde = "";
      sendcentraleverde = 0;
      sendcentralerosso = 0;
      sendcentraleblu = 0;
      count_send = 0;
      draw(6, 6, 6);  // display STOP
      if (Staterx != "6000") {
        ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "6000");
      }
      flagcount = false;
      Staterx = "";
      updatescreen = 1;
    }


    if (Rxs.indexOf("GO") != -1 && updatescreen == 0) {  // se riveco GO dal TEENSY
      end10lapblu = 0;
      end10lapverde = 0;
      end10laprosso = 0;
      doppiotaglioverde = 0;
      doppiotagliorosso = 0;
      doppiotaglioblu = 0;
      timeoutblu = 0;
      timeoutverde = 0;
      timeoutrosso = 0;
      showcroverde = false;
      showcrorosso = false;
      showcroblu = false;
      flagcount = true;
      stringonerosso = "";
      stringoneblu = "";
      stringoneverde = "";
      sendcentraleverde = 0;
      sendcentralerosso = 0;
      sendcentraleblu = 0;
      count_send = 0;
      updatescreen = 1;
      if (Staterx != "3000") {
        ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "3000");
      }
      Staterx = "";
      memset(arraytempirosso, 0, sizeof(arraytempirosso));
      memset(arraytaglirosso, 0, sizeof(arraytaglirosso));
      memset(arraytempiverde, 0, sizeof(arraytempiverde));
      memset(arraytagliverde, 0, sizeof(arraytagliverde));
      memset(arraytempiblu, 0, sizeof(arraytempiblu));
      memset(arraytagliblu, 0, sizeof(arraytagliblu));
      memset(temporosso, 0, sizeof(temporosso));
      memset(tempoverde, 0, sizeof(tempoverde));
      memset(tempoblu, 0, sizeof(tempoblu));
    }
    if (Rxs.indexOf("SHOW") != -1 && updatescreen == 0) {  // se riveco SHOW dal TEENSY
      end10lapblu = 0;
      end10lapverde = 0;
      doppiotaglioverde = 0;
      doppiotagliorosso = 0;
      doppiotaglioblu = 0;
      end10laprosso = 0;
      timeoutblu = 0;
      timeoutverde = 0;
      timeoutrosso = 0;
      showcroverde = false;
      showcrorosso = false;
      showcroblu = false;
      memset(arraytempirosso, 0, sizeof(arraytempirosso));
      memset(arraytaglirosso, 0, sizeof(arraytaglirosso));
      memset(arraytempiverde, 0, sizeof(arraytempiverde));
      memset(arraytagliverde, 0, sizeof(arraytagliverde));
      memset(arraytempiblu, 0, sizeof(arraytempiblu));
      memset(arraytagliblu, 0, sizeof(arraytagliblu));
      memset(temporosso, 0, sizeof(temporosso));
      memset(tempoverde, 0, sizeof(tempoverde));
      memset(tempoblu, 0, sizeof(tempoblu));
      if (Staterx != "2000") {
        ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, "2000");
      }
      draw(1, 1, 1);  // display show
      stringonerosso = "";
      stringoneblu = "";
      stringoneverde = "";
      sendcentraleverde = 0;
      sendcentralerosso = 0;
      sendcentraleblu = 0;
      count_send = 0;
      updatescreen = 1;
      Staterx = "";
      flagcount = false;
    }

    if (Rxs.indexOf("2510") != -1 || Rxs.indexOf("2520") != -1 || Rxs.indexOf("2530") != -1) {  // codice  show dai telecomandi del cronometro
      values[0] = "0";
      decodecomma(Rxs, values);  // decodifico i valori

      if (values[0] == "2510") {
        temporosso[0] = values[0];
        showcrorosso = true;
      } else if (values[0] == "2530") {
        tempoblu[0] = values[0];
        showcroblu = true;
      } else if (values[0] == "2520") {
        tempoverde[0] = values[0];
        showcroverde = true;
      }
      draw(1, 1, 1);  // display show
    }


    if (Rxs.indexOf("4514") != -1 || Rxs.indexOf("4524") != -1 || Rxs.indexOf("4534") != -1) {  // controllo se ci sta il codice della ricezione tempo
      values[0] = "0";
      decodecomma(Rxs, values);  // decodifico i valori

      if (values[0] == "4514") {
        temporosso[0] = values[0];
        temporosso[1] = values[1];
        temporosso[2] = values[2];
      } else if (values[0] == "4534") {
        tempoblu[0] = values[0];
        tempoblu[1] = values[1];
        tempoblu[2] = values[2];
      } else if (values[0] == "4524") {
        tempoverde[0] = values[0];
        tempoverde[1] = values[1];
        tempoverde[2] = values[2];
      }
      if (flagcount == false) {
        draw(2, 2, 2);
      }
    }


    if (Rxs.indexOf("5514") != -1 || Rxs.indexOf("5524") != -1 || Rxs.indexOf("5534") != -1) {  // se ricevo dal teensy i codici di fine gara codice ricezione 10 giri
      codestringone = decodestringone(Rxs);
      if (codestringone.indexOf("5514") != -1) {
        end10laprosso = 1;
        tempototrosso = arraytempirosso[11];
        sendcentralerosso = 1;
        stringonerosso = Rxs;
      }
      if (codestringone.indexOf("5524") != -1) {
        end10lapverde = 1;
        tempototverde = arraytempiverde[11];
        sendcentraleverde = 1;
        stringoneverde = Rxs;
      }
      if (codestringone.indexOf("5534") != -1) {
        end10lapblu = 1;
        tempototblu = arraytempiblu[11];
        sendcentraleblu = 1;
        stringoneblu = Rxs;
      }
      displayend10lap(end10laprosso, end10lapverde, end10lapblu);
    }


    if (Rxs.indexOf("6534") != -1 || Rxs.indexOf("6524") != -1 || Rxs.indexOf("6514") != -1) {  // se ricevo dal teensy i codici di TIMEOUT
      if (Rxs.indexOf("6514") != -1) {
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        end10laprosso = 1;
        timeoutrosso = 1;
        sendcentralerosso = 1;
        stringonerosso = Rxs;
      }
      if (Rxs.indexOf("6524") != -1) {
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        end10lapverde = 1;
        timeoutverde = 1;
        sendcentraleverde = 1;
        stringoneverde = Rxs;
      }
      if (Rxs.indexOf("6534") != -1) {
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        end10lapblu = 1;
        timeoutblu = 1;
        sendcentraleblu = 1;
        stringoneblu = Rxs;
      }
    }



    if (Rxs.indexOf("4015") != -1 || Rxs.indexOf("4025") != -1 || Rxs.indexOf("4035") != -1) {  // se ricevo dal teensy il DOPPIO TAGLIO
      codestringone = decodestringone(Rxs);
      if (Rxs.indexOf("4015") != -1) {                                                          // se doppiotaglio rosso
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        doppiotagliorosso = 1;
        end10laprosso = 1;
        sendcentralerosso = 1;
        stringonerosso = Rxs;
      }
      if (Rxs.indexOf("4035") != -1) {  // se doppiotaglio blu
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        doppiotaglioblu = 1;
        end10lapblu = 1;
        sendcentraleblu = 1;
        stringoneblu = Rxs;
      }
      if (Rxs.indexOf("4025") != -1) {  // se doppiotaglio verde
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        doppiotaglioverde = 1;
        end10lapverde = 1;
        sendcentraleverde = 1;
        stringoneverde = Rxs;
      }
    }

    newData = false;
  }
  // fine lettura dalla seriale del teesny

  // GESTIONE DEL DISPLAY A FINE 10 GIRI

  displayend10lap(end10laprosso, end10lapverde, end10lapblu);


  // GESTIONE DEL CONTO ALLA ROVESCIA
  flagcount = countdown(flagcount);

  // GESTIONE DELLA SIRENA
  sirenamillis = millis();

  if (sirenamillis - startsirena >= 300) {
    digitalWrite(sirena, LOW);
  }


  //FINE GESTIONE SIRENA
}




void recvWithStartEndMarkers(HardwareSerial& port, char receivedChars[numChars]) {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (port.available() > 0 && newData == false) {
    rc = port.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
  port.flush();
}
