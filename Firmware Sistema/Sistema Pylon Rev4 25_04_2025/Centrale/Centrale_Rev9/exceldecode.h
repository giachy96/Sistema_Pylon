
const int len = 100; // SE NON LI LEGGE BENE IL TESTO RIECVUTO AUMENTARE I CARETTERI
char my_str[len];
int pos = 0;
int incomingByte = 0; // for incoming serial data
extern  int buzzer;

int code_rx = 0;
String gara ;
int manche_rx = 0;
int round_rx = 0;
String nome_rosso;
String cognome_rosso;
String nome_verde;
String cognome_verde;
String nome_blu;
String cognome_blu;

bool newDataPC = false;


void processParam(char * param, int paramNum) {
  char key[] = "true"; //https://www.cplusplus.com/reference/cstring/strcmp/
  if (strcmp (key, param) == 0) {
    for (int i = 4; i < 14; i++) {
      digitalWrite(i, LOW);
    }
  }
  if (paramNum == 1) {
    code_rx = atoi(param); //codice dell'evento
  }
  if (paramNum == 2 && paramNum != NULL) {
    gara = param;
  }
  if (paramNum == 3 && paramNum != NULL) {
    manche_rx = atoi(param);
  }
  if (paramNum == 4 && paramNum != NULL) {
    round_rx = atoi(param);
  }
  if (paramNum == 5 && paramNum != NULL) {
    nome_rosso = param;
  }
  if (paramNum == 6 && paramNum != NULL) {
    cognome_rosso = param;
  }
  if (paramNum == 7 && paramNum != NULL) {
    nome_verde = param;
  }
  if (paramNum == 8 && paramNum != NULL) {
    cognome_verde = param;
  }
  if (paramNum == 9 && paramNum != NULL) {
    nome_blu = param;
  }
  if (paramNum == 10 && paramNum != NULL) {
    cognome_blu = param;
  }

}
/* behaves like strtok() except that it returns empty tokens also
*/
char* strtoke(char *str, const char *delim)
{
  static char *start = NULL; /* stores string str for consecutive calls */
  char *token = NULL; /* found token */
  /* assign new start in case */
  if (str) start = str;
  /* check whether text to parse left */
  if (!start) return NULL;
  /* remember current start as found token */
  token = start;
  /* find next occurrence of delim */
  start = strpbrk(start, delim);
  /* replace delim with terminator and move start to follower */
  if (start) *start++ = '\0';
  /* done */
  return token;
}


void splitCommaSeparated() {
  //splits a comma separated char array into separated char arrays for each comma separated value
  char * param;
  int paramNum = 1;



  //param = strtok(my_str,",\n");   // strtok explanation at : https://www.cplusplus.com/reference/cstring/strtok/
  param = strtoke(my_str, ",\n");  // strtoke explanation at : https://stackoverflow.com/questions/42315585/split-string-into-tokens-in-c-when-there-are-2-delimiters-in-a-row

  while (param != NULL)
  {
//    Serial.print("param");
//    Serial.print(paramNum);
//    Serial.print(": ");
//    Serial.println(param);
    processParam(param, paramNum);
    //finally get the next param from the comma separated my_str if there is one.
    //param = strtok(NULL, ",\n");
    param = strtoke(NULL, ",\n");
    paramNum++;
  }

  newDataPC = true;

}
