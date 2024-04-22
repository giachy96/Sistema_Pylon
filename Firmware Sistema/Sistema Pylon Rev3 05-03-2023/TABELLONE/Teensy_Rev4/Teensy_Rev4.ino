//testato 27/12/2022
#include "serialmethode.h"
#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>


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
unsigned long currentMillis;
unsigned long oldMillis;
unsigned long pressbutton = 1200;


String cnc;
String tempiverde[12];
String tempirosso[12];
String tempiblu[12];
int end10lap_rosso = 0;
int end10lap_verde = 0;
int end10lap_blu = 0;


String values[5];

int ngiri_blu = 0;
int ngiri_rosso = 0;
int ngiri_verde = 0;
int ntagliP1_rosso = 0;
int ntaglitot_rosso = 0;
int arrayP1_rosso[11];
int ntagliP2_rosso = 0;
int arrayP2_rosso[11];
int ntagliP3_rosso = 0;
int arrayP3_rosso[11];
int ntagliP1_blu = 0;
int ntaglitot_blu = 0;
int arrayP1_blu[11];
int ntagliP2_blu = 0;
int arrayP2_blu[11];
int ntagliP3_blu = 0;
int arrayP3_blu[11];
int ntagliP1_verde = 0;
int ntaglitot_verde = 0;
int arrayP1_verde[11];
int ntagliP2_verde = 0;
int arrayP2_verde[11];
int ntagliP3_verde = 0;
int arrayP3_verde[11];

// variabili per il dalay a giro
int delaygiro_rosso = 0;
unsigned long timedelaygiro_rosso ;
int delay10giri_rosso = 0;
unsigned long timedelay10giri_rosso ;
int delaygiro_verde = 0;
unsigned long timedelaygiro_verde ;
int delay10giri_verde = 0;
unsigned long timedelay10giri_verde ;
int delaygiro_blu = 0;
unsigned long timedelaygiro_blu ;
int delay10giri_blu = 0;
unsigned long timedelay10giri_blu ;



String stringoneverde  ;
String stringoneblu  ;
String stringonerosso  ;
unsigned long stringone_delay ;


int pStop = 38;
int pGo = 39;
int pShow = 40;
int codicecentrale = 0;

const int chipSelect = BUILTIN_SDCARD;
String nome_file = "Backup_";


void setup() {
  pinMode(pStop, INPUT_PULLUP);
  pinMode(pGo, INPUT_PULLUP);
  pinMode(pShow, INPUT_PULLUP);
  Serial.begin(9600);
  Serial1.begin(9600);  // cronoblu
  Serial2.begin(9600);  // palo 2/3 blu
  Serial3.begin(9600);  // palo1
  Serial4.begin(9600);  // palo 2/3 verde
  Serial5.begin(9600);  // palo 2/3 rosso
  Serial6.begin(9600);  // centrale
  Serial7.begin(9600);  // cronoverde
  Serial8.begin(9600);  // cronorosso

  // for the log of the manche
  SD.begin(chipSelect);
  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  nome_file.concat(day());
  nome_file.concat("_");
  nome_file.concat(month());
  nome_file.concat("_");
  nome_file.concat(year());
  nome_file.concat(".txt");


}


// the loop routine runs over and over again forever:
void loop() {
  currentMillis = millis();
  recvSerial1();
  recvSerial2();
  recvSerial3();
  recvSerial4();
  recvSerial5();
  recvSerial6();
  recvSerial7();
  recvSerial8();


  if (currentMillis - oldMillis >= pressbutton) {
  //  if (digitalRead(pStop) == HIGH || codicecentrale == 6000) {
   if (digitalRead(pStop) == LOW || codicecentrale == 6000) {
      Serial.println("<STOP>");
      Serial6.print("<STOP>");
      Serial8.print("<6000>");
      Serial1.print("<6000>");
      Serial7.print("<6000>");
      Serial3.print("<6000>");
      Serial5.print("<6000>");
      Serial4.print("<6000>");
      Serial2.print("<6000>");
      oldMillis = millis();
      codicecentrale = 0;
    }
    if (digitalRead(pGo) == LOW || codicecentrale == 3000) {
      Serial.println("<GO>");
      Serial6.print("<GO>");
      Serial8.print("<3000>");
      Serial1.print("<3000>");
      Serial7.print("<3000>");
      Serial3.print("<3000>");
      Serial5.print("<3000>");
      Serial4.print("<3000>");
      Serial2.print("<3000>");
      oldMillis = millis();
      ntagliP1_rosso = 0;
      memset(arrayP1_rosso, 0, sizeof(arrayP1_rosso));
      ntagliP2_rosso = 0;
      memset(arrayP2_rosso, 0, sizeof(arrayP2_rosso));
      ntagliP3_rosso = 0;
      memset(arrayP3_rosso, 0, sizeof(arrayP3_rosso));
      ntagliP1_blu = 0;
      memset(arrayP1_blu, 0, sizeof(arrayP1_blu));
      ntagliP2_blu = 0;
      memset(arrayP2_blu, 0, sizeof(arrayP2_blu));
      ntagliP3_blu = 0;
      memset(arrayP3_blu, 0, sizeof(arrayP3_blu));
      ntagliP1_verde = 0;
      memset(arrayP1_verde, 0, sizeof(arrayP1_verde));
      ntagliP2_verde = 0;
      memset(arrayP2_verde, 0, sizeof(arrayP2_verde));
      ntagliP3_verde = 0;
      memset(arrayP3_verde, 0, sizeof(arrayP3_verde));
      for ( int k = 0;  k < 12 ; k++) {
        tempiverde[k] = "";
        tempiblu[k] = "";
        tempirosso[k] = "";
      }
      ngiri_blu = 0;
      ngiri_rosso = 0;
      ngiri_verde = 0;
      ntaglitot_blu = 0;
      ntaglitot_verde = 0;
      ntaglitot_rosso = 0;
      codicecentrale = 0;
      stringoneverde = ""  ;
      stringoneblu = "" ;
      stringonerosso = "" ;
    }
    if (digitalRead(pShow) == LOW || codicecentrale == 2000) {
      Serial.println("<SHOW>");
      Serial6.print("<SHOW>");
      Serial8.print("<2000>");
      Serial1.print("<2000>");
      Serial7.print("<2000>");
      Serial3.print("<2000>");
      Serial5.print("<2000>");
      Serial4.print("<2000>");
      Serial2.print("<2000>");
      oldMillis = millis();
      ntagliP1_rosso = 0;
      memset(arrayP1_rosso, 0, sizeof(arrayP1_rosso));
      ntagliP2_rosso = 0;
      memset(arrayP2_rosso, 0, sizeof(arrayP2_rosso));
      ntagliP3_rosso = 0;
      memset(arrayP3_rosso, 0, sizeof(arrayP3_rosso));
      ntagliP1_blu = 0;
      memset(arrayP1_blu, 0, sizeof(arrayP1_blu));
      ntagliP2_blu = 0;
      memset(arrayP2_blu, 0, sizeof(arrayP2_blu));
      ntagliP3_blu = 0;
      memset(arrayP3_blu, 0, sizeof(arrayP3_blu));
      ntagliP1_verde = 0;
      memset(arrayP1_verde, 0, sizeof(arrayP1_verde));
      ntagliP2_verde = 0;
      memset(arrayP2_verde, 0, sizeof(arrayP2_verde));
      ntagliP3_verde = 0;
      memset(arrayP3_verde, 0, sizeof(arrayP3_verde));
      for ( int k = 0;  k < 12 ; k++) {
        tempiverde[k] = "";
        tempiblu[k] = "";
        tempirosso[k] = "";
      }
      ngiri_blu = 0;
      ngiri_rosso = 0;
      ngiri_verde = 0;
      ntaglitot_blu = 0;
      ntaglitot_verde = 0;
      ntaglitot_rosso = 0;
      codicecentrale = 0;
      stringoneverde = ""  ;
      stringoneblu = "" ;
      stringonerosso = "" ;
    }
  }

  //Dati dal CronoROSSO
  if (newData8 == true) {
    String Rxs8;
    Rxs8 = receivedChars8;
    Serial.println( Rxs8);
    if (Rxs8.indexOf("4514") != -1 || Rxs8.indexOf("5514") != -1 || Rxs8.indexOf("2510") != -1 || Rxs8.indexOf("6514") != -1) {
      String pack8 = "<";
      pack8.concat(receivedChars8);
      pack8.concat(">");

      if (Rxs8.indexOf("2510") != -1) {  // show rosso
        Serial6.print(pack8);
      }

      if (Rxs8.indexOf("4514") != -1) {  // nuovo giro rosso
        Serial6.print(pack8);
        decodecomma(Rxs8, values);
        tempirosso[values[1].toInt()] = values[2];
        delaygiro_rosso = 1;
        timedelaygiro_rosso = millis();
      }
      if (Rxs8.indexOf("5514") != -1) {  // fine 10 giri rosso
        decodecomma(Rxs8, values);
        tempirosso[11] = values[2];
        delay10giri_rosso = 1;
        timedelay10giri_rosso = millis();
      }
      if (Rxs8.indexOf("6514") != -1) {  // timeout rosso
        ntaglitot_rosso = 10;
      }
    }
    newData8 = false;
  }

  if (delaygiro_rosso == 1) {
    if (millis() - timedelaygiro_rosso > 300) {
      ngiri_rosso++;
      ntagliP1_rosso = 0;
      ntagliP2_rosso = 0;
      ntagliP3_rosso = 0;
      delaygiro_rosso = 0;
      Serial.println("ngiri_rosso");
      Serial.println(ngiri_rosso);
    }
  }
  if (delay10giri_rosso == 1) {
    if (millis() - timedelay10giri_rosso > 600) {
      Serial3.print("<5514>");
      Serial5.print("<5514>");
      stringonerosso = "<5514,";
      end10lap_rosso = 1;
      Serial.println("10giri_rosso");
      delay10giri_rosso = 0;
    }
  }

  //Dati dal CronoBLU
  if (newData1 == true) {
    String Rxs1;
    Rxs1 = receivedChars1;
    Serial.println( Rxs1);
    if (Rxs1.indexOf("4534") != -1 || Rxs1.indexOf("5534") != -1 || Rxs1.indexOf("2530") != -1 || Rxs1.indexOf("6534") != -1) {
      String pack1 = "<";
      pack1.concat(receivedChars1);
      pack1.concat(">");

      if (Rxs1.indexOf("2530") != -1) {  // show blu
        Serial6.print(pack1);
      }

      if (Rxs1.indexOf("4534") != -1) {  // nuovo giro blu
        Serial6.print(pack1);
        decodecomma(Rxs1, values);
        tempiblu[values[1].toInt()] = values[2];
        delaygiro_blu = 1;
        timedelaygiro_blu = millis();

      }
      if (Rxs1.indexOf("5534") != -1) {  // fine 10 giri blu
        decodecomma(Rxs1, values);
        tempiblu[11] = values[2];
        delay10giri_blu = 1;
        timedelay10giri_blu = millis();

      }
      if (Rxs1.indexOf("6534") != -1) {  // timeout blu
        ntaglitot_blu = 10;
      }
    }
    newData1 = false;
  }
  if (delaygiro_blu == 1) {
    if (millis() - timedelaygiro_blu > 300) {
      ngiri_blu++;
      ntagliP1_blu = 0;
      ntagliP2_blu = 0;
      ntagliP3_blu = 0;
      delaygiro_blu = 0;
      Serial.println("ngiri_blu");
      Serial.println(ngiri_blu);
    }
  }
  if (delay10giri_blu == 1) {
    if (millis() - timedelay10giri_blu > 600) {
      Serial3.print("<5534>");
      Serial2.print("<5534>");
      stringoneblu = "<5534,";
      end10lap_blu = 1;
      Serial.println("10giri_blu");
      delay10giri_blu = 0;
    }
  }


  //Dati dal CronoVERDE
  if (newData7 == true) {
    String Rxs7;
    Rxs7 = receivedChars7;
    Serial.println( Rxs7);
    if (Rxs7.indexOf("4524") != -1 || Rxs7.indexOf("5524") != -1 || Rxs7.indexOf("2520") != -1 || Rxs7.indexOf("6524") != -1) {
      String pack7 = "<";
      pack7.concat(receivedChars7);
      pack7.concat(">");

      if (Rxs7.indexOf("2520") != -1) {  // show verde
        Serial6.print(pack7);
      }

      if (Rxs7.indexOf("4524") != -1) {  // nuovo giro verde
        Serial6.print(pack7);
        decodecomma(Rxs7, values);
        tempiverde[values[1].toInt()] = values[2];
        delaygiro_verde = 1;
        timedelaygiro_verde = millis();

      }
      if (Rxs7.indexOf("5524") != -1) {  //fine 10 giri verde
        decodecomma(Rxs7, values);
        tempiverde[11] = values[2];
        delay10giri_verde = 1;
        timedelay10giri_verde = millis();

      }
      if (Rxs7.indexOf("6524") != -1) {  // timeout verde
        ntaglitot_verde = 10;
      }
    }
    newData7 = false;
  }

  if (delaygiro_verde == 1) {
    if (millis() - timedelaygiro_verde > 300) {
      ngiri_verde++;
      ntagliP1_verde = 0;
      ntagliP2_verde = 0;
      ntagliP3_verde = 0;
      Serial.println("ngiri_verde");
      Serial.println(ngiri_verde);
      delaygiro_verde = 0;
    }
  }
  if (delay10giri_verde == 1) {
    if (millis() - timedelay10giri_verde > 600) {
      Serial3.print("<5524>");
      Serial4.print("<5524>");
      stringoneverde = "<5524,";
      end10lap_verde = 1;
      Serial.println("10giri_verde");
      delay10giri_verde = 0;
    }
  }

  //Dati dalla CENTRALE
  if (newData6 == true) {
    String Rxs6;
    Rxs6 = receivedChars6;

    if (Rxs6.indexOf("850") != -1) {  // ho ricevuto avanti/indietro dalla centrale
      //per resettare le luci dei pali
      Serial2.println("<850>");
      Serial3.println("<850>");
      Serial4.println("<850>");
      Serial5.println("<850>");
      scrivisuSD (Rxs6);
    }

    if (Rxs6.indexOf("2000") != -1) {  // ho ricevuto lo show dalla centrale
      codicecentrale = 2000;
    }
    if (Rxs6.indexOf("3000") != -1) {  // ho ricevuto GO dalla centrale
      codicecentrale = 3000;
    }
    if (Rxs6.indexOf("6000") != -1) {  // ho ricevuto lo show dalla centrale
      codicecentrale = 6000;
    }
    if (Rxs6.indexOf("3513") != -1) {  // avvio rosso
      String pack6_r = "<";
      pack6_r.concat(receivedChars6);
      pack6_r.concat(">");
      Serial8.print(pack6_r);
    }
    if (Rxs6.indexOf("3523") != -1) {  // avvio verde
      String pack6_v = "<";
      pack6_v.concat(receivedChars6);
      pack6_v.concat(">");
      Serial7.print(pack6_v);
    }
    if (Rxs6.indexOf("3533") != -1) {  // avvio blu
      String pack6_b = "<";
      pack6_b.concat(receivedChars6);
      pack6_b.concat(">");
      Serial1.print(pack6_b);
    }
    if (Rxs6.indexOf("3001") != -1) {  // avvio tutti i tagli
      String pack6_t = "<";
      pack6_t.concat(receivedChars6);
      pack6_t.concat(">");
      Serial3.print(pack6_t);  // palo1
      Serial5.print(pack6_t);  //palo2-3 rosso
      Serial4.print(pack6_t);  //palo2-3 rosso
      Serial2.print(pack6_t);  //palo2-3 rosso
    }
    if (Rxs6.indexOf("500") != -1) {  // ho ricevuto STANDBY
      Serial2.println("<500>");
      Serial3.println("<500>");
      Serial4.println("<500>");
      Serial5.println("<500>");
    }
    newData6 = false;
  }

  //Dati dalla Palo1
  if (newData3 == true) {
    String Rxs3;
    Rxs3 = receivedChars3;
    if (Rxs3.indexOf("3111") != -1) {  // tagli palo 1 rosso
      Serial.println("Taglio P1 Rosso");
      ntagliP1_rosso++;
      ntaglitot_rosso++;
      arrayP1_rosso[ngiri_rosso - 1] = ntagliP1_rosso;
    }
    if (Rxs3.indexOf("3121") != -1) {  // tagli palo 1 verde
      ntagliP1_verde++;
      ntaglitot_verde++;
      arrayP1_verde[ngiri_verde - 1] = ntagliP1_verde;
    }
    if (Rxs3.indexOf("3131") != -1) {  // tagli palo 1 blu
      ntagliP1_blu++;
      ntaglitot_blu++;
      arrayP1_blu[ngiri_blu - 1] = ntagliP1_blu;
    }
    newData3 = false;
  }


  //Dati dalla Palo2/3rosso
  if (newData5 == true) {
    String Rxs5;
    Rxs5 = receivedChars5;
    if (Rxs5.indexOf("3211") != -1) {  // tagli palo 2 rosso
      Serial.println("Taglio P2 Rosso");
      ntagliP2_rosso++;
      ntaglitot_rosso++;
      arrayP2_rosso[ngiri_rosso - 1] = ntagliP2_rosso;
    }
    if (Rxs5.indexOf("3311") != -1) {  // tagli palo 3 rosso
      Serial.println("Taglio P3 Rosso");
      ntagliP3_rosso++;
      ntaglitot_rosso++;
      arrayP3_rosso[ngiri_rosso - 1] = ntagliP3_rosso;
    }
    newData5 = false;
  }

  //Dati dalla Palo2/3verde
  if (newData4 == true) {
    String Rxs4;
    Rxs4 = receivedChars4;
    if (Rxs4.indexOf("3221") != -1) {  // tagli palo 2 verde
      Serial.println("Taglio P2 Verde");
      ntagliP2_verde++;
      ntaglitot_verde++;
      arrayP2_verde[ngiri_verde - 1] = ntagliP2_verde;
    }
    if (Rxs4.indexOf("3321") != -1) {  // tagli palo 3 verde
      Serial.println("Taglio P3 verde");
      ntagliP3_verde++;
      ntaglitot_verde++;
      arrayP3_verde[ngiri_verde - 1] = ntagliP3_verde;
    }
    newData4 = false;
  }

  //Dati dalla Palo2/3blu
  if (newData2 == true) {
    String Rxs2;
    Rxs2 = receivedChars2;
    if (Rxs2.indexOf("3231") != -1) {  // tagli palo 2 blu
      Serial.println("Taglio P2 Blu");
      ntagliP2_blu++;
      ntaglitot_blu++;
      arrayP2_blu[ngiri_blu - 1] = ntagliP2_blu;
    }
    if (Rxs2.indexOf("3331") != -1) {  // tagli palo 3 blu
      Serial.println("Taglio P3 blu");
      ntagliP3_blu++;
      ntaglitot_blu++;
      arrayP3_blu[ngiri_blu - 1] = ntagliP3_blu;
    }
    newData2 = false;
  }



  if (ntaglitot_rosso > 1 && ntaglitot_rosso < 10 ) {  // ha fatto due tagli il rosso
    //Serial6.println("<4015>");
    Serial3.println("<4015>");
    Serial5.println("<4015>");
    Serial8.println("<4015>");
    ntaglitot_rosso = 0;
    end10lap_rosso = 1;
    stringonerosso = "<4015,";
  }
  if (ntaglitot_verde > 1 && ntaglitot_verde < 10) {  // ha fatto due tagli il verde
    // Serial6.println("<4025>");
    Serial3.println("<4025>");
    Serial7.println("<4025>");
    Serial4.println("<4025>");
    ntaglitot_verde = 0;
    end10lap_verde = 1;
    stringoneverde = "<4025,";
  }

  if (ntaglitot_blu > 1 && ntaglitot_blu < 10) {  // ha fatto due tagli il blu
    // Serial6.println("<4035>");
    Serial3.println("<4035>");
    Serial1.println("<4035>");
    Serial2.println("<4035>");
    ntaglitot_blu = 0;
    end10lap_blu = 1;
    stringoneblu = "<4035,";
  }
  if ( ntaglitot_blu == 10) {  // ATTENZIONE PER PULIZA SI è riustao il ntagli blu per il TIMEOUT!
    // Serial6.println("<6534>");  // timeout blu
    Serial3.println("<6534>");
    Serial2.println("<6534>");
    ntaglitot_blu = 0;
    end10lap_blu = 1;
    stringoneblu = "<6534,";
  }
  if ( ntaglitot_verde == 10) {  //timeout verde
    // Serial6.println("<6534>");
    Serial3.println("<6524>");
    Serial4.println("<6524>");
    ntaglitot_verde = 0;
    end10lap_verde = 1;
    stringoneverde = "<6524,";
  }
  if ( ntaglitot_rosso == 10) {  //timeout rosso
    // Serial6.println("<6534>");
    Serial3.println("<6514>");
    Serial5.println("<6514>");
    ntaglitot_rosso = 0;
    end10lap_rosso = 1;
    stringonerosso = "<6514,";
  }


  // codice che crea le stringhe finali per i colori
  if (end10lap_rosso == 1 &&  ( millis() - stringone_delay  >= 200)) {  // creo la stringa finale per il rosso

    for (int i = 1; i <= 10; i++) {
      stringonerosso.concat(tempirosso[i]);

      if (arrayP1_rosso[i - 1] != 0) {
        stringonerosso.concat("-P1");
      }
      if (arrayP2_rosso[i - 1] != 0) {
        stringonerosso.concat("-P2");
      }
      if (arrayP3_rosso[i - 1] != 0) {
        stringonerosso.concat("-P3");
      }
      stringonerosso.concat(",");
    }
    stringonerosso.concat(tempirosso[11]);
    stringonerosso.concat(">");
    Serial6.println(stringonerosso);
    stringone_delay  = millis();
    Serial.println("Mando lo stringone rosso alla centrale");
    Serial.println("Scrivo su SD lo stringone rosso");
    scrivisuSD (stringonerosso);
    Serial.println(stringonerosso);
    for (int k = 0; k <= 10; k++) {
      Serial.print(arrayP2_rosso[k]);
    }
    Serial.println(" ");
    end10lap_rosso = 0;

  }

  if (end10lap_verde == 1 &&  ( millis() - stringone_delay  >= 200) ) {  // creo la stringa finale per il verde


    for (int i = 1; i <= 10; i++) {
      stringoneverde.concat(tempiverde[i]);

      if (arrayP1_verde[i - 1] != 0) {
        stringoneverde.concat("-P1");
      }
      if (arrayP2_verde[i - 1] != 0) {
        stringoneverde.concat("-P2");
      }
      if (arrayP3_verde[i - 1] != 0) {
        stringoneverde.concat("-P3");
      }
      stringoneverde.concat(",");
    }
    stringoneverde.concat(tempiverde[11]);
    stringoneverde.concat(">");
    Serial6.println(stringoneverde);
    stringone_delay  = millis();
    Serial.println("Mando lo stringone verde alla centrale");
    Serial.println("Scrivo su SD lo stringone verde");
    scrivisuSD (stringoneverde);
    Serial.println(stringoneverde);
    for (int k = 0; k <= 10; k++) {
      Serial.print(arrayP2_verde[k]);
    }
    Serial.println(" ");
    end10lap_verde = 0;

  }


  if (end10lap_blu == 1 &&  ( millis() - stringone_delay  >= 200)) {  // creo la stringa finale per il blu


    for (int i = 1; i <= 10; i++) {
      stringoneblu.concat(tempiblu[i]);

      if (arrayP1_blu[i - 1] != 0) {
        stringoneblu.concat("-P1");
      }
      if (arrayP2_blu[i - 1] != 0) {
        stringoneblu.concat("-P2");
      }
      if (arrayP3_blu[i - 1] != 0) {
        stringoneblu.concat("-P3");
      }
      stringoneblu.concat(",");
    }
    stringoneblu.concat(tempiblu[11]);
    stringoneblu.concat(">");
    Serial6.println(stringoneblu);
    stringone_delay  = millis();
    Serial.println("Mando lo stringone blu alla centrale");
    Serial.println("Scrivo su SD lo stringone blu");
    scrivisuSD (stringoneblu);
    Serial.println(stringoneblu);
    end10lap_blu = 0;
  }


  // fine codice delle stringhe finali
} // FINE LOOP


void decodecomma(String str, String tempi[]) {
  int lungh_str = str.length();
  char buff[lungh_str + 1];
  str.toCharArray(buff, lungh_str + 1);
  //Serial.println(parseData(buff));
  int i = 0;
  //Serial.println("Example of splitting a string into tokens: ");
  // Serial.print("The input string is: '");
  // Serial.print(buff);
  // Serial.println("'");

  char* p = strtok(buff, ",");  //2nd argument is a char[] of delimiters
  while (p != NULL ) {       //not equal to NULL
    //Serial.println(p);
    tempi[i] = p;

    p = strtok(NULL, ",");  //expects NULL for string on subsequent calls
    i++;
  }

  //return tempi;
}

void scrivisuSD( String string_to_backup ) {

  // make a string for assembling the data to log:
  String dataString = hour();
  dataString += ":";
  dataString += minute();
  dataString += ":";
  dataString += second();
  dataString += "   ";
  dataString += string_to_backup;

  // open the file.

  int str_len = nome_file.length() + 1;
  char nome_file_char[str_len];
  nome_file.toCharArray(nome_file_char, str_len);
  File dataFile = SD.open(nome_file_char, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println("Ho scritto su SD");
  } else {
    // if the file isn't open, pop up an error:
    Serial.println("error opening file.txt");
  }

}


time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    return pctime;
    if ( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
}
