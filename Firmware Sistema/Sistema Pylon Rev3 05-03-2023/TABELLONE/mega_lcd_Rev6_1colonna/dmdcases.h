
#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include "nomi_font14.h"
#include <fonts/bmz10x14.h>
#include <fonts/Droid_Sans_12.h>
#include <fonts/Arial_Black_16.h>




#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 3
SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 3, 6, 7, 8);  // DMD controls the entire display

extern bool flagcount;
extern bool showcroverde;
extern bool showcrorosso;
extern bool showcroblu;
extern String temporosso[5];
extern String tempoblu[5];
extern String tempoverde[5];
extern int doppiotaglioverde;
extern int doppiotagliorosso;
extern int doppiotaglioblu;
extern int ntaglitotrosso;
extern int ntaglitotverde ;
extern int ntaglitotblu ;
extern String tempototrosso;
extern String arraytaglirosso[12];
extern String tempototverde;
extern String arraytagliverde[12];
extern String tempototblu;
extern String arraytagliblu[12];
extern int timeoutblu ;
extern int timeoutverde ;
extern int timeoutrosso ;

unsigned long oldbounce;
int bounce = 0;


void checkboxcrono( int xs, int ys, int xe, int ye, boolean flg) {

  if (flg == true) {
    dmd.drawFilledBox(xs, ys, xe, ye);
  } else {
    dmd.drawBox(xs, ys, xe, ye);
  }
}





void drawsubRosso(int cases) {
  String nomerossotmp;
  String cognnomerossotmp_1;
  String cognnomerossotmp_2;
  switch (cases) {

    case 0:
      // NOME rosso
      nomerossotmp = nome_rosso;
      nomerossotmp.remove(1);
      nomerossotmp.concat(".");
      cognnomerossotmp_1 = cognome_rosso;
      cognnomerossotmp_1.remove(4);
      cognnomerossotmp_2 = cognome_rosso;
      cognnomerossotmp_2.remove(0, 4);
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 32, nomerossotmp );
      dmd.drawString(9, 32, cognnomerossotmp_1 );
      dmd.drawString(0, 40, cognnomerossotmp_2);

      break;
    case 1:
      // Show

      dmd.selectFont(nomi_font14);
      dmd.drawString(0, 35, "SW");
      checkboxcrono( 22, 36, 30, 44, showcrorosso);
      break;

    case 2:  // tempo giri
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 32, "LP:" );
      dmd.drawString(20, 32, temporosso[1] );
      dmd.drawString(0, 40, temporosso[2]);
      break;

    case 3:  // totale tempo

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 35,  tempototrosso);
      break;

    case 4:  // numero tagli
      if (ntaglitotrosso <= 0) {
        dmd.selectFont(Droid_Sans_12);
        dmd.drawString(0, 35, "0");
        dmd.drawString(8, 35, "CUT");
      } else if (ntaglitotrosso == 1) {
        int trovato;
        for (int i = 0; i < 10; i++) {
          if (arraytaglirosso[i].indexOf("P2") != -1 || arraytaglirosso[i].indexOf("P1") != -1 || arraytaglirosso[i].indexOf("P3") != -1 ) {
            trovato = i;
          }
        }
        char cut1[2];
        itoa(trovato + 1, cut1, 10); //(integer, yourBuffer, base)
        dmd.selectFont(Droid_Sans_12);
        dmd.drawString(0, 35, cut1 );
        dmd.drawString(10, 35, arraytaglirosso[trovato]);
      } else { // tagli > 1
        dmd.selectFont(SystemFont5x7);
        int index_row = 0;
        for (int i = 0; i < 10; i++) {
          if (arraytaglirosso[i].indexOf("P1") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 32 + index_row, lapbuf );
            dmd.drawString(13, 32 + index_row, ".P1" );
            index_row = index_row + 8;

          }
          if (arraytaglirosso[i].indexOf("P2") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 32 + index_row, lapbuf );
            dmd.drawString(13, 32 + index_row, ".P2" );
            index_row = index_row + 8;
          }
          if (arraytaglirosso[i].indexOf("P3") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 32 + index_row , lapbuf );
            dmd.drawString(13, 32 + index_row, ".P3" );
            index_row = index_row + 8;

          }
        }
      }
      break;
    case 5:  // doppio taglio

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 36, "200");
      break;


    case 6:

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 36, "STOP");
      break;

    case 7:
      // Gara
      dmd.selectFont(nomi_font14);
      dmd.drawString(4, 35, gara);
      break;
  }
}

void drawsubVerde(int cases) {
  String nomeverdetmp;
  String cognnomeverdetmp_1;
  String cognnomeverdetmp_2;
  switch (cases) {

    case 0:

      // NOME verde
      nomeverdetmp = nome_verde;
      nomeverdetmp.remove(1);
      nomeverdetmp.concat(".");
      cognnomeverdetmp_1 = cognome_verde;
      cognnomeverdetmp_1.remove(4);
      cognnomeverdetmp_2 = cognome_verde;
      cognnomeverdetmp_2.remove(0, 4);
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 16, nomeverdetmp );
      dmd.drawString(9, 16, cognnomeverdetmp_1);
      dmd.drawString(0, 24, cognnomeverdetmp_2);


      break;
    case 1:
      // Show

      dmd.selectFont(nomi_font14);
      dmd.drawString(0, 19, "SW");
      checkboxcrono( 22, 20, 30, 28, showcroverde);
      break;

    case 2:  // tempo giri

      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 16, "LP:" );
      dmd.drawString(20, 16, tempoverde[1] );
      dmd.drawString(0, 24, tempoverde[2]);
      break;


    case 3:  // totale tempo

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 19,   tempototverde);
      break;

    case 4:  // numero tagli
      if (ntaglitotverde <= 0) {
        dmd.selectFont(Droid_Sans_12);
        dmd.drawString(0, 19, "0");
        dmd.drawString(8, 19, "CUT");
      } else if (ntaglitotverde == 1) {
        int trovato;
        for (int i = 0; i < 10; i++) {
          if (arraytagliverde[i].indexOf("P2") != -1 || arraytagliverde[i].indexOf("P1") != -1 || arraytagliverde[i].indexOf("P3") != -1 ) {
            trovato = i;
          }
        }
        char cut1[2];
        itoa(trovato + 1, cut1, 10); //(integer, yourBuffer, base)
        dmd.selectFont(Droid_Sans_12);
        dmd.drawString(0, 19, cut1 );
        dmd.drawString(10, 19, arraytagliverde[trovato]);
      } else { // tagli > 1
        dmd.selectFont(SystemFont5x7);
        int index_row = 0;
        for (int i = 0; i < 10; i++) {
          if (arraytagliverde[i].indexOf("P1") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 16 + index_row, lapbuf );
            dmd.drawString(13, 16 + index_row, ".P1" );
            index_row = index_row + 8;

          }
          if (arraytagliverde[i].indexOf("P2") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 16 + index_row, lapbuf );
            dmd.drawString(13, 16 + index_row, ".P2" );
            index_row = index_row + 8;
          }
          if (arraytagliverde[i].indexOf("P3") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 16 + index_row , lapbuf );
            dmd.drawString(13, 16 + index_row, ".P3" );
            index_row = index_row + 8;

          }
        }
      }

      break;

    case 5:  // doppio taglio

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 20, "200");
      break;



    case 6:  // STOP

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 20, "STOP");
      break;

    case 7:
      // Manche
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(6, 16, "Set" );
      dmd.drawString(10, 24, String(manche_rx));
      break;
  }
}
void drawsubBlu(int cases) {
  String nomeblutmp;
  String cognnomeblutmp_1;
  String cognnomeblutmp_2;
  switch (cases) {

    case 0:
      // NOME blu
      nomeblutmp = nome_blu;
      nomeblutmp.remove(1);
      nomeblutmp.concat(".");
      cognnomeblutmp_1 = cognome_blu;
      cognnomeblutmp_1.remove(4);
      cognnomeblutmp_2 = cognome_blu;
      cognnomeblutmp_2.remove(0, 4);
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 0, nomeblutmp );
      dmd.drawString(9, 0, cognnomeblutmp_1);
      dmd.drawString(0, 8, cognnomeblutmp_2);
      break;
    case 1:
      // Show

      dmd.selectFont(nomi_font14);
      dmd.drawString(0, 3, "SW");
      checkboxcrono(22, 4, 30, 12, showcroblu);
      break;

    case 2:  // tempo giri

      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 0, "LP:" );
      dmd.drawString(20, 0, tempoblu[1] );
      dmd.drawString(0, 8, tempoblu[2]);

      break;


    case 3:  // tempo totale

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 3,  tempototblu);
      break;

    case 4:  // numero tagli
      if (ntaglitotblu <= 0) {
        dmd.selectFont(Droid_Sans_12);
        dmd.drawString(0, 3, "0");
        dmd.drawString(8, 3, "CUT");
      } else if (ntaglitotblu == 1) {
        int trovato;
        for (int i = 0; i < 10; i++) {
          if (arraytagliblu[i].indexOf("P2") != -1 || arraytagliblu[i].indexOf("P1") != -1 || arraytagliblu[i].indexOf("P3") != -1 ) {
            trovato = i;
          }
        }
        char cut1[2];
        itoa(trovato + 1, cut1, 10); //(integer, yourBuffer, base)
        dmd.selectFont(Droid_Sans_12);
        dmd.drawString(0, 3, cut1 );
        dmd.drawString(10, 3, arraytagliblu[trovato]);
      } else { // tagli > 1
        dmd.selectFont(SystemFont5x7);
        int index_row = 0;
        for (int i = 0; i < 10; i++) {
          if (arraytagliblu[i].indexOf("P1") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 0 + index_row, lapbuf );
            dmd.drawString(13, 0 + index_row, ".P1" );
            index_row = index_row + 8;

          }
          if (arraytagliblu[i].indexOf("P2") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 0 + index_row, lapbuf );
            dmd.drawString(13, 0 + index_row, ".P2" );
            index_row = index_row + 8;
          }
          if (arraytagliblu[i].indexOf("P3") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            dmd.drawString(1, 0 + index_row , lapbuf );
            dmd.drawString(13, 0 + index_row, ".P3" );
            index_row = index_row + 8;

          }
        }
      }

      break;

    case 5:  // doppio taglio

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 1, "200");
      break;

    case 6:  // Stop

      dmd.selectFont(Droid_Sans_12);
      dmd.drawString(0, 4, "STOP");
      break;

    case 7:
      // Round
      dmd.selectFont(SystemFont5x7);
      dmd.drawString(0, 0, "Round");
      dmd.drawString(10, 8, String(round_rx));
      break;
  }
}

void draw(int casoRosso, int casoVerde, int casoBlu) {
  int cRosso ;
  int cVerde ;
  int cBlu ;
  cRosso = casoRosso;
  cVerde = casoVerde;
  cBlu = casoBlu;
  dmd.clearScreen();

  if (doppiotaglioverde == 1 && casoVerde != 4  ) {
    cVerde = 5;
  }
  if (doppiotagliorosso == 1 && casoRosso != 4 ) {
    cRosso = 5;
  }
  if (doppiotaglioblu == 1 && casoBlu != 4 ) {
    cBlu = 5;
  }
  if (timeoutverde == 1  ) {
    cVerde = 5;
  }
  if (timeoutrosso == 1 ) {
    cRosso = 5;
  }
  if (timeoutblu == 1 ) {
    cBlu = 5;
  }

  //  if (casoRosso == 0 && casoVerde == 0 &&   casoBlu == 0 ) {
  //    drawsubRosso(7);
  //    drawsubVerde(7);
  //    drawsubBlu(7);
  //    delay(4000);
  //    dmd.clearScreen();
  //  }



  drawsubRosso(cRosso);
  drawsubVerde(cVerde);
  drawsubBlu(cBlu);
}

void displayend10lap(int end10laprosso, int  end10lapverde, int end10lapblu ) {
  if (end10laprosso == 1 || end10lapverde == 1 || end10lapblu == 1) {  // se ricevo dal teensy i codici di fine gara
    int r;
    int g;
    int b;

    if (millis() - oldbounce >= 1000) {
      if (bounce == 0) {
        if (end10laprosso == 1) {
          r = 3;
        }
        if (end10laprosso == 0) {
          r = 2;
        }
        if (end10lapverde == 1) {
          g = 3;
        }
        if (end10lapverde == 0) {
          g = 2;
        }
        if (end10lapblu == 1) {
          b = 3;
        }
        if (end10lapblu == 0) {
          b = 2;
        }
        bounce = 1;
      } else {
        if (end10laprosso == 1) {
          r = 4;
        }
        if (end10laprosso == 0) {
          r = 2;
        }
        if (end10lapverde == 1) {
          g = 4;
        }
        if (end10lapverde == 0) {
          g = 2;
        }
        if (end10lapblu == 1) {
          b = 4;
        }
        if (end10lapblu == 0) {
          b = 2;
        }
        bounce = 0;
      }

      draw(r, g, b);
      oldbounce = millis();
    }
  }
}
