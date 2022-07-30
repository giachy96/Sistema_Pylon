extern int pinbatt; // This is a declaration
float vout;
float out_max = 14.5;
float out_min = 0.2 ;
float in_max = 4.91;
float in_min = 0.0;
extern float tempi_rosso[11];
extern float tempi_blu[11];
extern float tempi_verde[11];
extern int  v ;
extern int  r ;
extern int  b ;
// per la mappatura della tensione batteria in un range
float fmap(float x, float out_max , float out_min , float in_max , float in_min)
{
  float k1 = (out_max - out_min) / (in_max - in_min);
  float k2 = out_min - (in_min * k1);
  return x * k1 + k2;
}

// per la lettura della tensione della batteria
float  tensionebatt( int pin) {
  float tensione_float = analogRead(pin);
  tensione_float = (tensione_float / 1023) * 5;
  tensione_float = tensione_float - 0.015;
  tensione_float = fmap(tensione_float, out_max , out_min ,  in_max ,  in_min);
  return tensione_float;
}






void draw (int cases , LiquidCrystal_I2C lcd) {
  switch (cases) {
    case 0 :
      // Avviamento
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("FT Models");
      lcd.setCursor(0, 1);
      lcd.print("Base Cronometraggio");
      lcd.setCursor(3, 2);
      lcd.print("Avviamento");
      vout = tensionebatt( pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      break;
    case 1:
      // Sincronizzazione
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Sincronizzazione ");
      lcd.setCursor(0, 1);
      lcd.print("Pronti per la gara!");
      vout = tensionebatt( pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      // statements
      break;
    case 2:
      // ricezioni tempi
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("TEMPI       LAP N");
      lcd.setCursor(0, 1);
      lcd.print("P.Rosso ");
      lcd.setCursor(9, 1);
      lcd.print(tempi_rosso[r - 1], 2);
      lcd.setCursor(13, 1);
      lcd.print("s" );
      lcd.setCursor(18, 1);
      lcd.print(r );
      lcd.setCursor(0, 2);
      lcd.print("P.Verde ");
      lcd.setCursor(9, 2);
      lcd.print(tempi_verde[v - 1], 2);
      lcd.setCursor(13, 2);
      lcd.print("s" );
      lcd.setCursor(18, 2);
      lcd.print(v );
      lcd.setCursor(0, 3);
      lcd.print("P.Blu ");
      lcd.setCursor(9, 3);
      lcd.print(tempi_blu[b - 1], 2);
      lcd.setCursor(13, 3);
      lcd.print("s" );
      lcd.setCursor(18, 3);
      lcd.print(b );

      break;
    case 3:
      // STOP
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("STOP! ");
      lcd.setCursor(0, 1);
      lcd.print("Arresto della gara!");
      vout = tensionebatt( pinbatt);
      lcd.setCursor(3, 3);
      lcd.print("V.Batt.");
      lcd.print(vout);

      break;
    case 4:
      // Totali
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("TEMPI TOTALI ");
      lcd.setCursor(0, 1);
      lcd.print("P.Rosso ");
      lcd.setCursor(9, 1);
      lcd.print(tempi_rosso[10], 2);
      lcd.setCursor(14, 1);
      lcd.print("s");
      lcd.setCursor(0, 2);
      lcd.print("P.Verde ");
      lcd.setCursor(9, 2);
      lcd.print(tempi_verde[10], 2);
      lcd.setCursor(14, 2);
      lcd.print("s");
      lcd.setCursor(0, 3);
      lcd.print("P.Blu ");
      lcd.setCursor(9, 3);
      lcd.print(tempi_blu[10], 2);
      lcd.setCursor(14, 3);
      lcd.print("s");



      break;
    default:
      // statements
      break;
  }

}
