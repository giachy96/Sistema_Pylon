
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
      dmd.drawString(0, 33, nomerossotmp);
      dmd.drawString(10, 33, cognome_rosso);

      break;
    case 1:
      // Show
      dmd.drawString(0, 33, "SHOW");
      checkboxcrono( 52, 35, 60, 43, showcrorosso);
      break;

    case 2:  // tempo giri
      dmd.drawString(0, 33, temporosso[1]);
      dmd.drawString(18, 33, temporosso[2]);

      break;

    // case 3:  // tempo 10 giro
    //   dmd.drawString(0, 33, temporosso[1]);
    //   dmd.drawString(18, 33, temporosso[2]);
    //   break;

    // case 4:  // totale tempo
    //   dmd.drawString(10, 33, temporosso[3]);

    //   break;

    case 3:  // totale tempo
      dmd.drawString(10, 33, temporosso[3]);
      break;

    case 4:  // numero tagli
      dmd.drawString(2, 33, "Tagli");

      break;
    case 5:  // doppio taglio
      dmd.drawString(18, 33, "200");
      break;


    case 6:
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
      dmd.drawString(0, 17, nomeverdetmp);
      dmd.drawString(10, 17, cognome_verde);
      break;
    case 1:
      // Show
      dmd.drawString(0, 17, "SHOW");
      checkboxcrono( 52, 19, 60, 27, showcroverde);
      break;

    case 2:  // tempo giri
      dmd.drawString(0, 17, tempoverde[1]);
      dmd.drawString(18, 17, tempoverde[2]);
      break;


    // case 3:  // tempo 10 giri
    //   dmd.drawString(0, 17, tempoverde[1]);
    //   dmd.drawString(18, 17, tempoverde[2]);
    //   break;

    // case 4:  // tempo totale
    //   dmd.drawString(10, 17, tempoverde[3]);

    //   break;

    case 3:  // totale tempo
      dmd.drawString(10, 17, tempoverde[3]);
      break;

    case 4:  // numero tagli
      dmd.drawString(10, 17, "Tagli");

      break;

    case 5:  // doppio taglio
      dmd.drawString(18, 17, "200");
      break;



    case 6:  // STOP
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
      dmd.drawString(0, 1, nomeblutmp);
      dmd.drawString(10, 1, cognome_blu);
      break;
    case 1:
      // Show
      dmd.drawString(0, 1, "SHOW");
      checkboxcrono(52, 3, 60, 11, showcroblu);
      break;

    case 2:  // tempo giri
      dmd.drawString(0, 1, tempoblu[1]);
      dmd.drawString(18, 1, tempoblu[2]);
      break;


    // case 3:  // tempo giro 10

    //   dmd.drawString(0, 1, tempoblu[1]);
    //   dmd.drawString(18, 1, tempoblu[2]);

    //   break;

    // case 4:  // tempo totale
    //   dmd.drawString(10, 1, tempoblu[3]);


    //   break;
    case 3:  // tempo totale
      dmd.drawString(10, 1, tempoblu[3]);
      break;

    case 4:  // numero tagli
      dmd.drawString(2, 1, "Tagli");
      break;

    case 5:  // doppio taglio
      dmd.drawString(18, 1, "200");
      break;

    case 6:  // Stop
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

  if (casoRosso == 0 || casoVerde == 0 || casoBlu == 0) {
    dmd.selectFont(nomi_font14);
  } else {
    dmd.selectFont(Arial_Black_16);
  }

  if (doppiotaglioverde == 1) {
    cVerde = 5;
  }
  if (doppiotagliorosso == 1) {
    cRosso = 5;
  }
  if (doppiotaglioblu == 1) {
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
