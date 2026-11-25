extern String arraytempirosso[12];
extern String arraytaglirosso[12];
extern String arraytempiverde[12];
extern String arraytagliverde[12];
extern String arraytempiblu[12];
extern String arraytagliblu[12];
int ntaglitotrosso = 0;
int ntaglitotverde = 0;
int ntaglitotblu = 0;

// Rev12: primo campo del pacchetto (tutto fino alla prima virgola).
// I codici si confrontano SOLO con questo campo, mai con indexOf sull'intera stringa,
// altrimenti numeri dentro al nome gara o ai tempi (es. "2000", "6000", "850") fanno scattare comandi a caso.
String headOf(const String &s) {
  int p = s.indexOf(',');
  if (p < 0) return s;
  return s.substring(0, p);
}

// Rev12: spezza "a,b,,c" in campi (i campi vuoti vengono mantenuti, cosi' i valori non scalano di posto)
// e non scrive mai oltre la dimensione dell'array. I campi non ricevuti vengono azzerati.
template <size_t N>
int decodecomma(const String &str, String (&tempi)[N]) {
  int n = 0;
  int start = 0;
  while (n < (int)N) {
    int p = str.indexOf(',', start);
    if (p < 0) {
      tempi[n++] = str.substring(start);
      break;
    }
    tempi[n++] = str.substring(start, p);
    start = p + 1;
  }
  for (int k = n; k < (int)N; k++) tempi[k] = "";
  return n;
}

// Rev12: svuota un array di String senza memset (memset su String perde memoria dinamica e lascia puntatori sporchi)
template <size_t N>
void azzeraStringhe(String (&arr)[N]) {
  for (size_t k = 0; k < N; k++) arr[k] = "";
}

// Rev12: elabora un singolo colore. arraytmp[0] e' il codice, 1..11 = tempi (con eventuale -P1/-P2/-P3), 11 = totale.
static void decodeColore(String arraytmp[], String arraytagli[], String arraytempi[], int &ntagliTot) {
  ntagliTot = 0;
  for (int i = 1; i < 12; i++) {
    String tmp = arraytmp[i];
    arraytagli[i - 1] = "";  // Rev12: riparto da zero, cosi' una seconda ricezione non accumula "P1P1"
    int p;
    p = tmp.indexOf("-P1");
    if (p != -1) {
      arraytagli[i - 1].concat("P1");
      ntagliTot++;
      tmp.remove(p, 3);
    }
    p = tmp.indexOf("-P2");
    if (p != -1) {
      arraytagli[i - 1].concat("P2");
      ntagliTot++;
      tmp.remove(p, 3);
    }
    p = tmp.indexOf("-P3");
    if (p != -1) {
      arraytagli[i - 1].concat("P3");
      ntagliTot++;
      tmp.remove(p, 3);
    }
    arraytempi[i] = tmp;
  }
}

String decodestringone (String strx) {

  String arraytmp [12];
  String code ;
  decodecomma (strx , arraytmp);
  //caso rosso
  if (arraytmp[0] == "5514" || arraytmp[0] == "4015") {
    decodeColore(arraytmp, arraytaglirosso, arraytempirosso, ntaglitotrosso);
    code = "5514";
  }
  //caso verde
  if (arraytmp[0] == "5524" || arraytmp[0] == "4025") {
    decodeColore(arraytmp, arraytagliverde, arraytempiverde, ntaglitotverde);
    code = "5524";
  }
  //caso blu
  if (arraytmp[0] == "5534" || arraytmp[0] == "4035") {
    decodeColore(arraytmp, arraytagliblu, arraytempiblu, ntaglitotblu);
    code = "5534";
  }
  return code;
}
