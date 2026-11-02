// printercases.h - Rev10
// Stampa dei risultati sulla stampante termica (Serial3, comandi ESC/POS).
// Il testo stampato è identico alla Rev9; cambia solo la struttura del codice
// (un'unica funzione per pilota invece di tre blocchi copiati).

extern String arraytempirosso[12];
extern String arraytaglirosso[12];
extern String arraytempiverde[12];
extern String arraytagliverde[12];
extern String arraytempiblu[12];
extern String arraytagliblu[12];
extern String punteggiorosso;
extern String punteggioverde;
extern String punteggioblu;

void stampaGrandeOn()  { Serial3.write(0x1D); Serial3.write(0x21); Serial3.write(0x01); }
void stampaGrandeOff() { Serial3.write(0x1D); Serial3.write(0x21); Serial3.write(0x00); }
void stampaBoldOn()    { Serial3.write(0x1B); Serial3.write(0x45); Serial3.write(0x01); }
void stampaBoldOff()   { Serial3.write(0x1B); Serial3.write(0x45); Serial3.write(0x00); }

// Stampa il blocco di un pilota, dall'intestazione fino al punteggio (senza a capo finale).
// Si aspetta il carattere "grande" già attivo; lo lascia attivo.
void stampaPilota(const char *colore, const String &gara, int manche_rx, int round_rx,
                  const String &nome, const String &cognome,
                  String tempi[], String tagli[], const String &punteggio) {
  Serial3.print("RACE: ");
  Serial3.println(gara);
  Serial3.print("MANCHE N: ");
  if (manche_rx != 0) {
    Serial3.print(manche_rx);
  }
  Serial3.print("  ");
  Serial3.print("ROUND N: ");
  if (round_rx != 0) {
    Serial3.print(round_rx);
  }
  Serial3.println("");
  Serial3.print("Pilot ");
  Serial3.print(colore);
  Serial3.println(" ");
  Serial3.print("NAME:");
  Serial3.print(nome);
  Serial3.print(" ");
  Serial3.print(cognome);
  Serial3.println(" ");
  stampaGrandeOff();
  stampaBoldOn();
  for (int y = 1; y < 11; y++) {
    Serial3.print("LAP N: ");
    Serial3.print(y);
    Serial3.print(" T: ");
    Serial3.print(tempi[y]);
    Serial3.print("s     ");
    Serial3.print("[");
    Serial3.print(tagli[y - 1]);
    Serial3.println("]");
  }
  stampaBoldOff();
  stampaGrandeOn();
  Serial3.print("Total Time: ");
  Serial3.print(tempi[11]);
  Serial3.print("s");
  Serial3.println(" ");
  Serial3.print("SCORE: ");
  Serial3.print(punteggio);
}

void stampatotali(const String &gara, int manche_rx, int round_rx,
                  const String &nome_rosso, const String &cognome_rosso,
                  const String &nome_verde, const String &cognome_verde,
                  const String &nome_blu, const String &cognome_blu) {
  stampaGrandeOn();

  stampaPilota("RED", gara, manche_rx, round_rx, nome_rosso, cognome_rosso,
               arraytempirosso, arraytaglirosso, punteggiorosso);
  Serial3.println(" ");
  Serial3.println(" ");

  stampaPilota("GREEN", gara, manche_rx, round_rx, nome_verde, cognome_verde,
               arraytempiverde, arraytagliverde, punteggioverde);
  Serial3.println(" ");
  Serial3.println(" ");

  stampaPilota("BLUE", gara, manche_rx, round_rx, nome_blu, cognome_blu,
               arraytempiblu, arraytagliblu, punteggioblu);
  Serial3.println(" ");
  stampaGrandeOff();

  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println(" ");
}
