// exceldecode.h - Rev10
// Ricezione e decodifica delle righe inviate dal PC (Excel) sulla seriale USB.
// Formato riga: CODICE,GARA,MANCHE,ROUND,NOME_R,COGN_R,NOME_V,COGN_V,NOME_B,COGN_B\n

const int len = 160;  // lunghezza massima riga (nomi lunghi compresi); righe più lunghe vengono scartate
char my_str[len];
int pos = 0;
bool overflowRiga = false;

int code_rx = 0;
String gara;
int manche_rx = 0;
int round_rx = 0;
String nome_rosso;
String cognome_rosso;
String nome_verde;
String cognome_verde;
String nome_blu;
String cognome_blu;

bool newDataPC = false;


void processParam(char *param, int paramNum) {
  switch (paramNum) {
    case 1:  code_rx = atoi(param); break;  // codice dell'evento
    case 2:  gara = param; break;
    case 3:  manche_rx = atoi(param); break;
    case 4:  round_rx = atoi(param); break;
    case 5:  nome_rosso = param; break;
    case 6:  cognome_rosso = param; break;
    case 7:  nome_verde = param; break;
    case 8:  cognome_verde = param; break;
    case 9:  nome_blu = param; break;
    case 10: cognome_blu = param; break;
  }
}

/* come strtok() ma restituisce anche i token vuoti */
char *strtoke(char *str, const char *delim) {
  static char *start = NULL; /* memorizza la stringa per le chiamate successive */
  char *token = NULL;
  if (str) start = str;
  if (!start) return NULL;
  token = start;
  start = strpbrk(start, delim);
  if (start) *start++ = '\0';
  return token;
}


void splitCommaSeparated() {
  // divide my_str (terminata con '\0') nei campi separati da virgola
  char *param;
  int paramNum = 1;

  param = strtoke(my_str, ",");
  while (param != NULL) {
    processParam(param, paramNum);
    param = strtoke(NULL, ",");
    paramNum++;
  }
  newDataPC = true;
}

// scarta tutto quello che c'è in seriale e riparte da una riga pulita
void flushSerialePC() {
  while (Serial.available() > 0) Serial.read();
  pos = 0;
  overflowRiga = false;
}

// Legge dalla seriale tutti i byte disponibili, ma si ferma dopo aver completato
// UNA riga (così ogni riga viene inoltrata via LoRa prima di leggere la successiva).
void leggiSeriale() {
  while (Serial.available() > 0) {
    int c = Serial.read();
    if (c < 0) break;
    if (c == '\n') {
      bool valida = (!overflowRiga && pos > 0);
      if (valida) {
        my_str[pos] = '\0';
        splitCommaSeparated();
      }
      pos = 0;
      overflowRiga = false;
      if (valida) break;
    } else if (c != '\r') {
      if (pos < len - 1) {
        my_str[pos++] = (char)c;
      } else {
        overflowRiga = true;  // riga troppo lunga: la scarto fino al prossimo a capo
      }
    }
  }
}
