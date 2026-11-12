// lcdcases.h - Rev10
// Lettura tensione batteria e schermate dell'LCD 20x4.

extern int pinbatt;
float out_max = 16.9;
float out_min = 1;
float in_max = 4.85;
float in_min = 0.0;
extern int round_rx;
extern int manche_rx;
extern String gara;

// per la mappatura della tensione batteria in un range
float fmap(float x, float out_max, float out_min, float in_max, float in_min) {
  float k1 = (out_max - out_min) / (in_max - in_min);
  float k2 = out_min - (in_min * k1);
  return x * k1 + k2;
}

// per la lettura della tensione della batteria (media di 8 letture contro il rumore)
float tensionebatt(int pin) {
  long somma = 0;
  for (byte k = 0; k < 8; k++) {
    somma += analogRead(pin);
  }
  float tensione_float = ((float)somma / 8.0) / 1023.0 * 5.0;
  return fmap(tensione_float, out_max, out_min, in_max, in_min);
}

// riga batteria: larghezza fissa, così un valore più corto non lascia cifre vecchie
void stampaBatteria(LiquidCrystal_I2C &lcd, float v) {
  lcd.setCursor(3, 3);
  lcd.print("V.Batt.");
  lcd.print(v, 2);
  lcd.print("  ");
}

void draw(int cases, LiquidCrystal_I2C &lcd) {
  switch (cases) {
    case 0:
      // Avviamento
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("FT Models");
      lcd.setCursor(0, 1);
      lcd.print("    Centrale");
      lcd.setCursor(3, 2);
      lcd.print("Avviamento...");
      break;

    case 1:
      // Invio nomi - avanti
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("INVIO NOMI - AVANTI");
      lcd.setCursor(4, 1);
      lcd.print("Guardare il ");
      lcd.setCursor(5, 2);
      lcd.print("tabellone");
      break;

    case 11:
      // Invio nomi - indietro
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("INVIO NOMI-INDIETRO");
      lcd.setCursor(4, 1);
      lcd.print("Guardare il ");
      lcd.setCursor(5, 2);
      lcd.print("tabellone");
      break;

    case 2:
      // Show
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("SHOW ");
      lcd.setCursor(0, 1);
      lcd.print("Controllare giudici!");
      break;

    case 3:
      // Start gara
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("START GARA ");
      lcd.print(gara.substring(0, 7));  // oltre 7 caratteri andrebbe a capo sulla riga sbagliata
      lcd.setCursor(0, 1);
      lcd.print("MANCHE ");
      lcd.print(manche_rx);
      lcd.setCursor(0, 2);
      lcd.print("ROUND ");
      lcd.print(round_rx);
      break;

    case 4:
      // Fine gara
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("FINE GARA");
      lcd.setCursor(0, 1);
      lcd.print("INVIO DEI DATI AL PC");
      break;

    case 5:
      // STOP
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("STOP! ");
      lcd.setCursor(0, 1);
      lcd.print("Arresto della gara!");
      break;

    default:
      return;
  }
  stampaBatteria(lcd, tensionebatt(pinbatt));
}
