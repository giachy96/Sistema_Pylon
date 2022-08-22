extern int pinbatt;  // This is a declaration
float vout;
float out_max = 14.5;
float out_min = 0.2;
float in_max = 4.91;
float in_min = 0.0;

// per la mappatura della tensione batteria in un range
float fmap(float x, float out_max, float out_min, float in_max, float in_min) {
  float k1 = (out_max - out_min) / (in_max - in_min);
  float k2 = out_min - (in_min * k1);
  return x * k1 + k2;
}

// per la lettura della tensione della batteria
float tensionebatt(int pin) {
  float tensione_float = analogRead(pin);
  tensione_float = (tensione_float / 1023) * 5;
  tensione_float = tensione_float - 0.015;
  tensione_float = fmap(tensione_float, out_max, out_min, in_max, in_min);
  return tensione_float;
}






void draw(int cases, LiquidCrystal_I2C lcd) {
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
      vout = tensionebatt(pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      break;

    case 1:
      // Show
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("INVIO I NOMI AL TABELLONE ");
      lcd.setCursor(0, 1);
      lcd.print("Guardare il tabellone");
      vout = tensionebatt(pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      // statements
      break;

    case 2:
      // Show
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("SHOW ");
      lcd.setCursor(0, 1);
      lcd.print("Controllare giudici!");
      vout = tensionebatt(pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      // statements
      break;

    case 3:
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("START GARA ");
      lcd.setCursor(0, 1);
      lcd.print("MANCHE   ROUND");
      vout = tensionebatt(pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      // statements
      break;


    case 4:

      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("FINE GARA INVIO DEI DATI AL PC ");
      //lcd.setCursor(0, 1);
      // lcd.print("Arresto della gara!");
      vout = tensionebatt(pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

    case 5:
      // STOP
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("STOP! ");
      lcd.setCursor(0, 1);
      lcd.print("Arresto della gara!");
      vout = tensionebatt(pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      break;
  }
}
