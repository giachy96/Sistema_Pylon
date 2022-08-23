extern bool flagcount;
extern bool showcroverde;
extern bool showcrorosso;
extern bool showcroblu;
extern String temporosso[5];
extern String tempoblu[5];
extern String tempoverde[5];
extern String nome_rosso;
extern String cognome_rosso;
extern String nome_verde;
extern String cognome_verde;
extern String nome_blu;
extern String cognome_blu;

unsigned long oldbounce;
int bounce = 0;

void checkboxcrono(SPIDMD dmd, int xs, int ys, int xe, int ye, boolean flg) {

  if (flg == true) {
    dmd.drawFilledBox(xs, ys, xe, ye);
  } else {
    dmd.drawBox(xs, ys, xe, ye);
  }
}





void drawsubRosso(int cases, SPIDMD dmd) {
  switch (cases) {

    case 0:
      // NOME rosso
      String nomerossotmp = nome_rosso;
      nomerossotmp.remove(1);
      nomerossotmp.concat(".");
      dmd.drawString(0, 33, nomerossotmp);
      dmd.drawString(10, 33, cognome_rosso);

      break;
    case 1:
      // Show
      dmd.drawString(0, 33, "SHOW");
      checkboxcrono(dmd, 52, 35, 60, 43, showcrorosso);
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
      dmd.drawString(3, 33, "DUE TAGLI! ");
      break;


    case 6:
      dmd.drawString(8, 33, "STOP");
      break;
  }
}

void drawsubVerde(int cases, SPIDMD dmd) {
  switch (cases) {

    case 0:
      // NOME verde
      String nomeverdetmp = nome_verde;
      nomeverdetmp.remove(1);
      nomeverdetmp.concat(".");
      dmd.drawString(0, 17, nomeverdetmp);
      dmd.drawString(10, 17, cognome_verde);
      break;
    case 1:
      // Show
      dmd.drawString(0, 17, "SHOW");
      checkboxcrono(dmd, 52, 19, 60, 27, showcroverde);
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
      dmd.drawString(3, 17, "DUE TAGLI! ");
      break;



    case 6:  // STOP
      dmd.drawString(8, 17, "STOP");
      break;
  }
}
void drawsubBlu(int cases, SPIDMD dmd) {
  switch (cases) {

    case 0:
      // NOME blu
      String nomeblutmp = nome_blu;
      nomeblutmp.remove(1);
      nomeblutmp.concat(".");
      dmd.drawString(0, 1, nomeblutmp);
      dmd.drawString(10, 1, cognome_blu);
      break;
    case 1:
      // Show
      dmd.drawString(0, 1, "SHOW");
      checkboxcrono(dmd, 52, 3, 60, 11, showcroblu);
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
      dmd.drawString(3, 1, "DUE TAGLI! ");
      break;

    case 6:  // Stop
      dmd.drawString(8, 1, "STOP");
      break;
  }
}

void draw(int casoRosso, int casoVerde, int casoBlu, SPIDMD dmd) {
  dmd.clearScreen();
  if (casoRosso == 0 || casoVerde == 0 || casoBlu == 0) {
    dmd.selectFont(Arial_Black_16);
  } else {
    dmd.selectFont(Arial_Black_16);
  }
  drawsubRosso(casoRosso, dmd);
  drawsubVerde(casoVerde, dmd);
  drawsubBlu(casoBlu, dmd);
}

void displayend10lap(int end10laprosso, int  end10lapverde, int end10lapblu, SPIDMD dmd ) {
  if (end10laprosso == 1 || end10lapverde == 1 || end10lapblu == 1) {  // se ricevo dal teensy i codici di fine gara
    int r;
    int g;
    int b;

    if (millis() - oldbounce >= 1000) {
      if (bounce == 0) {
        if (end10laprosso == 1) { r = 3; }
        if (end10laprosso == 0) { r = 2; }
        if (end10lapverde == 1) { g = 3; }
        if (end10lapverde == 0) { g = 2; }
        if (end10lapblu == 1) { b = 3; }
        if (end10lapblu == 0) { b = 2; }
        bounce = 1;
      } else {
        if (end10laprosso == 1) { r = 4; }
        if (end10laprosso == 0) { r = 2; }
        if (end10lapverde == 1) { g = 4; }
        if (end10lapverde == 0) { g = 2; }
        if (end10lapblu == 1) { b = 4; }
        if (end10lapblu == 0) { b = 2; }
        bounce = 0;
      }

      draw(r, g, b, dmd);
      oldbounce = millis();
    }
  }
}