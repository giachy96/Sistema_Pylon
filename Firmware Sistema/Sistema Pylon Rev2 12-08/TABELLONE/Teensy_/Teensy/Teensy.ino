
#include "serialmethode.h"

char receivedChars1[numChars];
char receivedChars2[numChars];
char receivedChars3[numChars];
char receivedChars4[numChars];
char receivedChars5[numChars];
char receivedChars6[numChars];
char receivedChars7[numChars];
char receivedChars8[numChars];
boolean newData1 = false;
boolean newData2 = false;
boolean newData3 = false;
boolean newData4 = false;
boolean newData5 = false;
boolean newData6 = false;
boolean newData7 = false;
boolean newData8 = false;
unsigned long currentMillis ;
unsigned long oldMillis;
unsigned long pressbutton = 1000;

String cnc ;
String tempiverde[11];
String tempirosso[11];
String tempiblu[11];
String values [5];

int pStop = 38;
int pGo = 39;
int pShow = 40;



void setup() {
  pinMode(pStop , INPUT_PULLUP);
  pinMode(pGo , INPUT_PULLUP);
  pinMode(pShow , INPUT_PULLUP);
  Serial.begin(9600);
  Serial1.begin(9600); // cronoblu
  Serial2.begin(9600); // palo 2/3 blu
  Serial3.begin(9600); // palo1
  Serial4.begin(9600); // palo 2/3 verde
  Serial5.begin(9600); // palo 2/3 rosso
  Serial6.begin(9600); // centrale
  Serial7.begin(9600); // cronoverde
  Serial8.begin(9600); // cronorosso

}


// the loop routine runs over and over again forever:
void loop() {
  currentMillis = millis();
  recvSerial1();
  /// showSerial1();attenziona al new data
  recvSerial2();
  // showSerial2();
  recvSerial3();
  // showSerial3();
  recvSerial4();
  // showSerial4();
  recvSerial5();
  //  showSerial5();
  recvSerial6();
  // showSerial6();
  recvSerial7();
  //showSerial7();attenziona al new data
  recvSerial8();
  //showSerial8(); attenziona al new data

  if (currentMillis - oldMillis >= pressbutton) {
    if (digitalRead(pStop) == HIGH) {
      Serial6.print("<STOP>");
      Serial8.print("<6000>");
      Serial1.print("<6000>");
      Serial7.print("<6000>");
      Serial3.print("<6000>");
      Serial5.print("<6000>");
      Serial4.print("<6000>");
      Serial2.print("<6000>");
      oldMillis = millis();
    }
    if (digitalRead(pGo) == LOW ) {
      Serial6.print("<GO>");
      Serial8.print("<3000>");
      Serial1.print("<3000>");
      Serial7.print("<3000>");
      Serial3.print("<3000>");
      Serial5.print("<3000>");
      Serial4.print("<3000>");
      Serial2.print("<3000>");
      oldMillis = millis();
    }
    if (digitalRead(pShow) == LOW ) {
      Serial6.print("<SHOW>");
      Serial8.print("<2000>");
      Serial1.print("<2000>");
      Serial7.print("<2000>");
      Serial3.print("<2000>");
      Serial5.print("<2000>");
      Serial4.print("<2000>");
      Serial2.print("<2000>");      
      oldMillis = millis();
    }
  }

  if (newData8 == true) {
    String Rxs8;
    Rxs8 = receivedChars8;
    if (Rxs8.indexOf("4514") != -1 || Rxs8.indexOf("5514") != -1 || Rxs8.indexOf("2510") != -1) {
      String pack8 = "<";
      pack8.concat(receivedChars8);
      pack8.concat(">");
      Serial6.print(pack8);

      if (Rxs8.indexOf("4514") != -1 ) {
        decodecomma(Rxs8, values);
        tempirosso[values[1].toInt()] = values[2];
      }
      if (Rxs8.indexOf("5514") != -1 ) {
        decodecomma(Rxs8, values);
        tempirosso[11] = values[2];
        Serial3.print("<5514>");
      }

    }
    newData8 = false;
  }

  if (newData1 == true) {
    String Rxs1;
    Rxs1 = receivedChars1;
    if (Rxs1.indexOf("4534") != -1 || Rxs1.indexOf("5534") != -1 || Rxs1.indexOf("2530") != -1) {
      String pack1 = "<";
      pack1.concat(receivedChars1);
      pack1.concat(">");
      Serial6.print(pack1);
      Serial.println(pack1);

      if (Rxs1.indexOf("4534") != -1 ) {
        decodecomma(Rxs1, values);
        tempiblu[values[1].toInt()] = values[2];
      }
      if (Rxs1.indexOf("5534") != -1 ) {
        decodecomma(Rxs1, values);
        tempiblu[11] = values[2];
        Serial3.print("<5534>");
      }

    }
    newData1 = false;
  }

  if (newData7 == true) {
    String Rxs7;
    Rxs7 = receivedChars7;
    if (Rxs7.indexOf("4524") != -1 || Rxs7.indexOf("5524") != -1 || Rxs7.indexOf("2520") != -1) {
      String pack7 = "<";
      pack7.concat(receivedChars7);
      pack7.concat(">");
      Serial6.print(pack7);

      if (Rxs7.indexOf("4524") != -1 ) {
        decodecomma(Rxs7, values);
        tempiverde[values[1].toInt()] = values[2];
      }
      if (Rxs7.indexOf("5524") != -1 ) {
        decodecomma(Rxs7, values);
        tempiverde[11] = values[2];
        Serial3.print("<5524>");
      }

    }
    newData7 = false;
  }

  if (newData6 == true) {
    String Rxs6;
    Rxs6 = receivedChars6;
    if (Rxs6.indexOf("3513") != -1 ) { // avvio rosso
      String pack6_r = "<";
      pack6_r.concat(receivedChars6);
      pack6_r.concat(">");
      Serial8.print(pack6_r);
    }
    if (Rxs6.indexOf("3523") != -1 ) { // avvio verde
      String pack6_v = "<";
      pack6_v.concat(receivedChars6);
      pack6_v.concat(">");
      Serial7.print(pack6_v);
    }
    if (Rxs6.indexOf("3533") != -1 ) { // avvio blu
      String pack6_b = "<";
      pack6_b.concat(receivedChars6);
      pack6_b.concat(">");
      Serial1.print(pack6_b);
    }
    if (Rxs6.indexOf("3001") != -1 ) { // avvio tutti i tagli
      String pack6_t = "<";
      pack6_t.concat(receivedChars6);
      pack6_t.concat(">");
      Serial3.print(pack6_t); // palo1
      Serial5.print(pack6_t);//palo2-3 rosso
      Serial4.print(pack6_t);//palo2-3 rosso
      Serial2.print(pack6_t);//palo2-3 rosso
    }
    newData6 = false;
  }

}


void decodecomma (String str , String tempi[]) {
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

  //return tempi;
}
