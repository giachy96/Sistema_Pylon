//#include "timestructure.h"
extern int lapcounter ;
extern int pinbatt ;
//extern float tempo_flt[12];
extern float* poitem ;
float tensione_float = 0;
char vout[8]; // Buffer big enough for 7-character float

float readvoltage (int pin) {
  float tensione;
  tensione = analogRead(pin);
  tensione = (tensione/ 1023) * 4.35;
  return tensione;
}

void draw(int cases , U8GLIB_SH1106_128X64 u8g) {
  switch (cases) {

    case 0:
      // Standby
      tensione_float = readvoltage(pinbatt);
      dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!

      //tensione_batt = char(tensione_float );// using a float and the decimal places
      u8g.setFont(u8g_font_9x15);
      u8g.setFontRefHeightExtendedText();
      u8g.setDefaultForegroundColor();
      u8g.setFontPosTop();
      u8g.drawFrame(0, 0, 125, 64);
      u8g.drawStr(0, 8 , " Voltaggio");
      u8g.drawStr(0, 20 , " Batteria");
      u8g.drawStr(60, 40 , " V");
      u8g.drawStr(10, 40, vout );
      //Serial.println(vout);

      break;
    case 1:
      // Sinc

      u8g.setFont(u8g_font_9x15);
      u8g.setFontRefHeightExtendedText();
      u8g.setDefaultForegroundColor();
      u8g.setFontPosTop();
      u8g.drawStr(0, 5 , " Sincronizzato");
      u8g.drawStr(0, 25 , " Pronto per");
      u8g.drawStr(0, 40 , " la gara");
      break;
    case 2:
      // Tempo


      u8g.setFont(u8g_font_9x15);
      u8g.setFontRefHeightExtendedText();
      u8g.setDefaultForegroundColor();
      u8g.setFontPosTop();
      u8g.drawStr(5, 5 , "LAP N:");
      char buff[9];
      sprintf (buff, "%d", lapcounter);
      u8g.drawStr(65, 5 , buff);
      u8g.drawStr(0, 25 , " Tempo ");
      char buff2[7];
      dtostrf(*(poitem + lapcounter - 1), 6, 2, buff2);
      u8g.drawStr( 0, 42 , buff2);
      u8g.drawStr(50, 42 , " secondi ");
      break;

    case 3:
      // Fine 10 Giri

      u8g.setFont(u8g_font_9x15);
      u8g.setFontRefHeightExtendedText();
      u8g.setDefaultForegroundColor();
      u8g.setFontPosTop();
      u8g.drawStr(5, 5 , "LAP N:10");
      u8g.drawStr(0, 25 , "Tempo ");
      dtostrf(*(poitem + 9), 6, 2, buff2);
      u8g.drawStr( 50, 25 , buff2);
      u8g.drawStr(110, 25 , "s");
      u8g.drawStr(0, 45 , "Tot ");
      dtostrf(*(poitem + 10), 6, 2, buff);
      u8g.drawStr(50, 45 , buff);
      u8g.drawStr(110, 45 , "s");
      break;

    case 4:
      // STOP

      u8g.setFont(u8g_font_9x15);
      u8g.setFontRefHeightExtendedText();
      u8g.setDefaultForegroundColor();
      u8g.setFontPosTop();
      u8g.drawStr(0, 5 , " STOP Gara!");
      u8g.drawStr(0, 25 , " Reset della ");
      u8g.drawStr(0, 40 , " manche ");
      break;
   case 5:
      // attesa nuova
     tensione_float = readvoltage(pinbatt);
      dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!

      u8g.setFont(u8g_font_9x15);
      u8g.setFontRefHeightExtendedText();
      u8g.setDefaultForegroundColor();
      u8g.setFontPosTop();
      u8g.drawStr(0, 5 , " Fine Manche");
      u8g.drawStr(0, 25 , " In Attesa.... ");
      u8g.drawStr(60, 40 , " V");
      u8g.drawStr(10, 40, vout );
      break;
    default:
      // statements
      break;
  }
}
