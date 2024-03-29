extern String arraytempirosso[12];
extern String arraytaglirosso[12];
extern String arraytempiverde[12];
extern String arraytagliverde[12];
extern String arraytempiblu[12];
extern String arraytagliblu[12];
int ntaglitotrosso = 0;
int ntaglitotverde = 0;
int ntaglitotblu = 0;


void decodecomma (String str , String tempi[]) {
  //https://www.delftstack.com/it/howto/arduino/arduino-strtok/
  
  int lungh_str = str.length();
  char buff[lungh_str + 1];
  str.toCharArray(buff, lungh_str + 1);
  //Serial.println(parseData(buff));
  int i = 0;
  //Serial.println("Example of splitting a string into tokens: ");
  // Serial.print("The input string is: '");
  // Serial.print(buff);
  // Serial.println("'");

  char* p = strtok(buff, ","); //2nd argument is a char[] of delimiters
  while (p != NULL) { //not equal to NULL
    //Serial.println(p);
    tempi[i] = p;
    p = strtok(NULL, ",");  //expects NULL for string on subsequent calls
    i++;
  }

//return tempi;
}

String decodestringone (String strx) {


  String arraytmp [12];
  String code ;
  decodecomma (strx , arraytmp);
  //caso rosso
  if (arraytmp[0] == "5514" || arraytmp[0] == "4015") {
    ntaglitotrosso = 0;
    for (int i = 1 ; i < 12; i++) {
      String tmp = arraytmp[i];
      if (arraytmp[i].indexOf("-P1") != -1) {
        arraytaglirosso[i - 1].concat("P1");
        ntaglitotrosso++;
        tmp.remove(tmp.indexOf("-P1"), 3);
        arraytempirosso[i] = tmp;
      }
      if (arraytmp[i].indexOf("-P2") != -1) {
        arraytaglirosso[i - 1].concat("P2");
        ntaglitotrosso++;
        tmp.remove(tmp.indexOf("-P2"), 3);
        arraytempirosso[i] = tmp;
      }
      if (arraytmp[i].indexOf("-P3") != -1) {
        arraytaglirosso[i - 1].concat("P3");
        ntaglitotrosso++;
        tmp.remove(tmp.indexOf("-P3"), 3);
        arraytempirosso[i] = tmp;
      }

      arraytempirosso[i] = tmp;
    }
    code = "5514";
  }

  //caso verde
  if (arraytmp[0] == "5524" || arraytmp[0] == "4025") {
    ntaglitotverde = 0;
    for (int i = 1 ; i < 12; i++) {
      String tmp = arraytmp[i];
      if (arraytmp[i].indexOf("-P1") != -1) {
        arraytagliverde[i - 1].concat("P1");
        ntaglitotverde++;
        tmp.remove(tmp.indexOf("-P1"), 3);
        arraytempiverde[i] = tmp;
      }
      if (arraytmp[i].indexOf("-P2") != -1) {
        arraytagliverde[i - 1].concat("P2");
        ntaglitotverde++;
        tmp.remove(tmp.indexOf("-P2"), 3);
        arraytempiverde[i] = tmp;
      }
      if (arraytmp[i].indexOf("-P3") != -1) {
        arraytagliverde[i - 1].concat("P3");
        ntaglitotverde++;
        tmp.remove(tmp.indexOf("-P3"), 3);
        arraytempiverde[i] = tmp;
      }

      arraytempiverde[i] = tmp;
    }
    code = "5524";
  }
  //caso blu
  if (arraytmp[0] == "5534" || arraytmp[0] == "4035") {
    ntaglitotblu = 0;
    for (int i = 1 ; i < 12; i++) {
      String tmp = arraytmp[i];
      if (arraytmp[i].indexOf("-P1") != -1) {
        arraytagliblu[i - 1].concat("P1");
        ntaglitotblu++;
        tmp.remove(tmp.indexOf("-P1"), 3);
        arraytempiblu[i] = tmp;
      }
      if (arraytmp[i].indexOf("-P2") != -1) {
        arraytagliblu[i - 1].concat("P2");
        ntaglitotblu++;
        tmp.remove(tmp.indexOf("-P2"), 3);
        arraytempiblu[i] = tmp;
      }
      if (arraytmp[i].indexOf("-P3") != -1) {
        arraytagliblu[i - 1].concat("P3");
        ntaglitotblu++;
        tmp.remove(tmp.indexOf("-P3"), 3);
        arraytempiblu[i] = tmp;
      }

      arraytempiblu[i] = tmp;
    }
    code = "5534";
  }
  //debug caso rosso
  //  Serial.println("Tempi rosso ");
  //  for (int l = 0 ; l < 12 ; l++) {
  //    Serial.print(arraytempirosso[l]);
  //    Serial.print(" ");
  //  }
  //  Serial.println("Tagli Rosso");
  //  for (int l = 0 ; l < 10 ; l++) {
  //    if ( arraytaglirosso[l] == "") {
  //      Serial.print("/");
  //    }
  //    else {
  //      Serial.print(arraytaglirosso[l]);
  //    }
  //    Serial.print(" ");
  //  }
  //  Serial.println("");

  return code;

}
