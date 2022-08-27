
#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include "nomi_font14.h"
#include <fonts/Arial_Black_16.h>

#define DISPLAYS_ACROSS 2
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
extern String arraytaglirosso[10];
extern String tempototverde;
extern String arraytagliverde[10];
extern String tempototblu;
extern String arraytagliblu[10];
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
  switch (cases) {

    case 0:
      // NOME rosso
      nomerossotmp = nome_rosso;
      nomerossotmp.remove(1);
      nomerossotmp.concat(".");
      dmd.selectFont(nomi_font14);
      dmd.drawString(0, 33, nomerossotmp);
      dmd.drawString(10, 33, cognome_rosso);

      break;
    case 1:
      // Show

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(0, 33, "SHOW");
      checkboxcrono( 52, 35, 60, 43, showcrorosso);
      break;

    case 2:  // tempo giri

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(0, 33, temporosso[1]);
      dmd.drawString(18, 33, temporosso[2]);
      break;

    case 3:  // totale tempo

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(10, 33,  tempototrosso);
      break;

    case 4:  // numero tagli
      if (ntaglitotrosso <= 0) {
        dmd.selectFont(Arial_Black_16);
        dmd.drawString(2, 33, "0 CUT ");
      } else if (ntaglitotrosso == 1) {
        int trovato;
        for (int i = 0; i < 10; i++) {
          if (arraytaglirosso[i].indexOf("P3") != -1 || arraytaglirosso[i].indexOf("P1") != -1 || arraytaglirosso[i].indexOf("P3") != -1 ) {
            trovato = i;
          }
        }
        char cut1[2];
        itoa(trovato + 1, cut1, 10); //(integer, yourBuffer, base)
        dmd.selectFont(Arial_Black_16);
        dmd.drawString(2, 33, cut1 );
        dmd.drawString(15, 33, arraytaglirosso[trovato]);
      } else { // tagli > 1
        String cut2 = "";
        for (int i = 0; i < 10; i++) {
          if (arraytaglirosso[i].indexOf("P1") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P1");
            cut2.concat(" ");

          }
          if (arraytaglirosso[i].indexOf("P2") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P2");
            cut2.concat(" ");
          }
          if (arraytaglirosso[i].indexOf("P3") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P3");
            cut2.concat(" ");
          }
        }
        dmd.selectFont(nomi_font14);
        dmd.drawString(5, 35, cut2 );
      }

      break;
    case 5:  // doppio taglio

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(18, 33, "200");
      break;


    case 6:

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(8, 33, "STOP");
      break;
  }
}

void drawsubVerde(int cases) {
  String nomeverdetmp;
  switch (cases) {

    case 0:
      // NOME verde
      nomeverdetmp = nome_verde;
      nomeverdetmp.remove(1);
      nomeverdetmp.concat(".");
      dmd.selectFont(nomi_font14);
      dmd.drawString(0, 17, nomeverdetmp);
      dmd.drawString(10, 17, cognome_verde);
      break;
    case 1:
      // Show

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(0, 17, "SHOW");
      checkboxcrono( 52, 19, 60, 27, showcroverde);
      break;

    case 2:  // tempo giri

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(0, 17, tempoverde[1]);
      dmd.drawString(18, 17, tempoverde[2]);
      break;


    case 3:  // totale tempo

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(10, 17,   tempototverde);
      break;

    case 4:  // numero tagli
      if (ntaglitotverde <= 0) {
        dmd.selectFont(Arial_Black_16);
        dmd.drawString(2, 17, "0 CUT ");
      } else if (ntaglitotverde == 1) {
        int trovato;
        for (int i = 0; i < 10; i++) {
          if (arraytagliverde[i].indexOf("P3") != -1 || arraytagliverde[i].indexOf("P1") != -1 || arraytagliverde[i].indexOf("P3") != -1 ) {
            trovato = i;
          }
        }
        char cut1[2];
        itoa(trovato + 1, cut1, 10); //(integer, yourBuffer, base)
        dmd.selectFont(Arial_Black_16);
        dmd.drawString(2, 17, cut1 );
        dmd.drawString(15, 17, arraytagliverde[trovato]);
      } else {
        String cut2 = "";
        for (int i = 0; i < 10; i++) {
          if (arraytagliverde[i].indexOf("P1") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P1");
            cut2.concat(" ");
          }
          if (arraytagliverde[i].indexOf("P2") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P2");
            cut2.concat(" ");
          }
          if (arraytagliverde[i].indexOf("P3") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P3");
            cut2.concat(" ");
          }
        }
        dmd.selectFont(nomi_font14);
        dmd.drawString(5, 19, cut2 );
      }


      break;

    case 5:  // doppio taglio

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(18, 17, "200");
      break;



    case 6:  // STOP

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(8, 17, "STOP");
      break;
  }
}
void drawsubBlu(int cases) {
  String nomeblutmp;
  switch (cases) {

    case 0:
      // NOME blu
      nomeblutmp = nome_blu;
      nomeblutmp.remove(1);
      nomeblutmp.concat(".");
      dmd.selectFont(nomi_font14);
      dmd.drawString(0, 1, nomeblutmp);
      dmd.drawString(10, 1, cognome_blu);
      break;
    case 1:
      // Show

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(0, 1, "SHOW");
      checkboxcrono(52, 3, 60, 11, showcroblu);
      break;

    case 2:  // tempo giri

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(0, 1, tempoblu[1]);
      dmd.drawString(18, 1, tempoblu[2]);
      break;


    case 3:  // tempo totale

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(10, 1,  tempototblu);
      break;

    case 4:  // numero tagli
      if (ntaglitotblu <= 0) {
        dmd.selectFont(Arial_Black_16);
        dmd.drawString(2, 1, "0 CUT ");
      } else if (ntaglitotblu == 1) {
        int trovato;
        for (int i = 0; i < 10; i++) {
          if (arraytagliblu[i].indexOf("P3") != -1 || arraytagliblu[i].indexOf("P1") != -1 || arraytagliblu[i].indexOf("P3") != -1 ) {
            trovato = i;
          }
        }
        char cut1[2];
        itoa(trovato + 1, cut1, 10); //(integer, yourBuffer, base)
        dmd.selectFont(Arial_Black_16);
        dmd.drawString(2, 1, cut1 );
        dmd.drawString(15, 1, arraytagliblu[trovato]);
      } else {
        String cut2 = "";
        for (int i = 0; i < 10; i++) {
          if (arraytagliblu[i].indexOf("P1") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P1");
            cut2.concat(" ");
          }
          if (arraytagliblu[i].indexOf("P2") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P2");
            cut2.concat(" ");
          }
          if (arraytagliblu[i].indexOf("P3") != -1  ) {
            char lapbuf[2];
            itoa(i + 1, lapbuf, 10); //(integer, yourBuffer, base)
            cut2.concat(lapbuf);
            cut2.concat(".");
            cut2.concat("P3");
            cut2.concat(" ");
          }
        }
        dmd.selectFont(nomi_font14);
        dmd.drawString(5, 3, cut2 );
      }

      break;

    case 5:  // doppio taglio

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(18, 1, "200");
      break;

    case 6:  // Stop

      dmd.selectFont(Arial_Black_16);
      dmd.drawString(8, 1, "STOP");
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
