
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
      Serial8.print("<500>");
      Serial1.print("<500>");
      Serial7.print("<500>");
      Serial3.print("<500>");
      oldMillis = millis();
    }
    if (digitalRead(pGo) == LOW ) {
      Serial6.print("<GO>");
      Serial8.print("<300>");
      Serial1.print("<300>");
      Serial7.print("<300>");
      Serial3.print("<300>");
      oldMillis = millis();
    }
    if (digitalRead(pShow) == LOW ) {
      Serial6.print("<SHOW>");
      Serial8.print("<200>");
      Serial1.print("<200>");
      Serial7.print("<200>");
      Serial3.print("<200>");
      oldMillis = millis();
    }
  }

  if (newData8 == true) {
    String Rxs8;
    Rxs8 = receivedChars8;
    if (Rxs8.indexOf("423") != -1 || Rxs8.indexOf("523") != -1 || Rxs8.indexOf("220") != -1) {
      String pack = "<";
      pack.concat(receivedChars8);
      pack.concat(">");
      Serial6.print(pack);

      if (Rxs8.indexOf("423") != -1 ) {
        decodecomma(Rxs8, values);
        tempirosso[values[1].toInt()] = values[2];
      }
       if (Rxs8.indexOf("523") != -1 ) {
        decodecomma(Rxs8, values);
        tempirosso[11] = values[2];
      }

    }
    newData8 = false;
  }

  if (newData1 == true) {
    String Rxs1;
    Rxs1 = receivedChars1;
    if (Rxs1.indexOf("433") != -1 || Rxs1.indexOf("533") != -1 || Rxs1.indexOf("230") != -1) {
      String pack = "<";
      pack.concat(receivedChars1);
      pack.concat(">");
      Serial6.print(pack);
      Serial.println(pack);

       if (Rxs1.indexOf("433") != -1 ) {
        decodecomma(Rxs1, values);
        tempiblu[values[1].toInt()] = values[2];
      }
       if (Rxs1.indexOf("533") != -1 ) {
        decodecomma(Rxs1, values);
        tempiblu[11] = values[2];
      }

    }
    newData1 = false;
  }

  if (newData7 == true) {
    String Rxs7;
    Rxs7 = receivedChars7;
    if (Rxs7.indexOf("413") != -1 || Rxs7.indexOf("513") != -1 || Rxs7.indexOf("210") != -1) {
      String pack = "<";
      pack.concat(receivedChars7);
      pack.concat(">");
      Serial6.print(pack);

       if (Rxs7.indexOf("413") != -1 ) {
        decodecomma(Rxs7, values);
        tempiverde[values[1].toInt()] = values[2];
      }
       if (Rxs7.indexOf("513") != -1 ) {
        decodecomma(Rxs7, values);
        tempiverde[11] = values[2];
      }

    }
    newData7 = false;
  }

    if (newData6 == true) {
    String Rxs6;
    Rxs6 = receivedChars6;
    if (Rxs6.indexOf("329") != -1 ) { // avvio rosso
      String pack = "<";
      pack.concat(receivedChars6);
      pack.concat(">");
      Serial8.print(pack);
    }
       if (Rxs6.indexOf("319") != -1 ) { // avvio verde
      String pack = "<";
      pack.concat(receivedChars6);
      pack.concat(">");
      Serial7.print(pack);
    }
    if (Rxs6.indexOf("339") != -1 ) { // avvio blu
      String pack = "<";
      pack.concat(receivedChars6);
      pack.concat(">");
      Serial1.print(pack);
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