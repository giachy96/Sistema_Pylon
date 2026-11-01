// commacode.h - Rev10
// Decodifica dei pacchetti LoRa ricevuti dalle schede pilota.
// Formato pacchetto: CODICE,T1,T2,...,T10,TOTALE   (T può avere il suffisso -P1/-P2/-P3)

extern String arraytempirosso[12];
extern String arraytaglirosso[12];
extern String arraytempiverde[12];
extern String arraytagliverde[12];
extern String arraytempiblu[12];
extern String arraytagliblu[12];

#define NUM_CAMPI_PACCHETTO 12

// Divide "str" sulle virgole. A differenza di strtok i campi vuoti restano al loro
// posto (un giro mancante non sposta più i tempi successivi).
// Scrive al massimo "maxCampi" campi in tempi[] e ritorna quanti ne ha scritti:
// un pacchetto con troppi campi viene troncato invece di scrivere fuori dall'array.
int decodecomma(const String &str, String tempi[], int maxCampi) {
  int n = 0;
  int start = 0;
  int lungh = str.length();
  while (n < maxCampi && start <= lungh) {
    int fine = str.indexOf(',', start);
    if (fine < 0) fine = lungh;
    tempi[n++] = str.substring(start, fine);
    start = fine + 1;
  }
  return n;
}

// Scompone il pacchetto in tempi[] (indici 1..11) e tagli[] (indici 0..10).
// I tagli vengono riscritti da zero a ogni pacchetto: un pacchetto ripetuto
// (retry LoRa) non produce più "P1P1".
void decodestringone(const String &strx, String tempi[], String tagli[]) {
  static const char *const marcatori[3] = {"-P1", "-P2", "-P3"};
  static const char *const nomiTaglio[3] = {"P1", "P2", "P3"};

  String arraytmp[NUM_CAMPI_PACCHETTO];
  decodecomma(strx, arraytmp, NUM_CAMPI_PACCHETTO);

  for (int i = 1; i < NUM_CAMPI_PACCHETTO; i++) {
    String tmp = arraytmp[i];
    tagli[i - 1] = "";
    for (int p = 0; p < 3; p++) {
      int pos = tmp.indexOf(marcatori[p]);
      if (pos != -1) {
        tagli[i - 1].concat(nomiTaglio[p]);
        tmp.remove(pos, 3);
      }
    }
    tempi[i] = tmp;
  }
}
