//#include "timestructure.h"
extern int lapcounter ;
extern int pinbatt ;
//extern float tempo_flt[12];
extern float* poitem ;
float tensione_float = 0;
char vout[8]; // Buffer big enough for 7-character float

extern int ntagli;

float readvoltage (int pin) {
  float tensione;
  tensione = analogRead(pin);
  tensione = (tensione / 1023) * 4.35;
  return tensione;
}

void draw(int cases , U8X8_SH1106_128X64_NONAME_HW_I2C u8x8) {
  switch (cases) {

    case 0:
      // Standby
      u8x8.clear();
      tensione_float = readvoltage(pinbatt);
      dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!

      //tensione_batt = char(tensione_float );// using a float and the decimal places
      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      //      u8x8.drawFrame(0, 0, 125, 64);
      u8x8.drawString(0, 8 , " Voltaggio");
      u8x8.drawString(0, 10 , " Batteria");
      u8x8.drawString(10, 29, " V");
      u8x8.drawString(4, 29, vout );
      //Serial.println(vout);

      break;
    case 1:
      // SHOW
      u8x8.clear();
      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(0, 0 , " SHOW");
      u8x8.drawString(0, 2 , " Riconoscere il");
      u8x8.drawString(0, 4 , " proprio pilota");
      break;
    case 2:
      // START
      u8x8.clear();
      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(0, 0 , " START RACE");
      u8x8.drawString(0, 2, " Guardare il");
      u8x8.drawString(0, 4 , " proprio pilota");
      break;
    case 3:
      // Start Gara

      u8x8.clear();
      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(1, 1 , "LAP N:");
      char buff[9];
      sprintf (buff, "%d", lapcounter);
      u8x8.drawString(7, 1 , buff);
      u8x8.drawString(0, 3, " Tempo ");
      char buff2[7];
      dtostrf(*(poitem + lapcounter), 6, 3, buff2);
      u8x8.drawString( 6, 3 , buff2);
      u8x8.drawString(3, 5 , " secondi ");
      break;

    case 4:
      // Fine 10 Giri
      u8x8.clear();
      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(1, 0 , "LAP N:10");
      dtostrf(*(poitem + 10), 6, 3, buff2);
      u8x8.drawString( 3, 2 , buff2);
      u8x8.drawString(9, 2 , "s");
      u8x8.drawString(1, 4 , "Totale ");
      dtostrf(*(poitem + 11), 6, 3, buff);
      u8x8.drawString(3, 6 , buff);
      u8x8.drawString(9, 6 , "s");
      break;

    case 5:
      // STOP
      u8x8.clear();
      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(1, 0 , " STOP Gara!");
      u8x8.drawString(1, 3 , " Reset della ");
      u8x8.drawString(1,  5, " manche ");
      break;
    case 6:
      // attesa nuova
      u8x8.clear();
      tensione_float = readvoltage(pinbatt);
      dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!

      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(0, 5 , " Fine Manche");
      u8x8.drawString(0, 25 , " In Attesa.... ");
      u8x8.drawString(60, 40 , " V");
      u8x8.drawString(10, 40, vout );
      break;
          case 7:
      // attesa nuova
      u8x8.clear();
      tensione_float = readvoltage(pinbatt);
      dtostrf(tensione_float, 6, 2, vout); // Leave room for too large numbers!

      u8x8.setFont(u8x8_font_8x13B_1x2_r);
      u8x8.drawString(0, 5 , " Tagli");
      u8x8.drawString(0, 25 , " N: ");
      u8x8.drawString(60, 40 , " V");
      u8x8.drawString(10, 40, ntagli );
      break;
    default:
      // statements
      break;
  }
}
