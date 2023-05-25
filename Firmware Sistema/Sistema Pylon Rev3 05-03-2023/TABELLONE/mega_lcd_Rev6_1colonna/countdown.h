extern unsigned long currentMillis;
extern unsigned long milliscountdown;
extern int sec;
extern unsigned long startsirena;
extern int sirena;
boolean countdown(boolean flag) {

  if (flag == true) {


    if (currentMillis - milliscountdown >= 1000) {
      dmd.selectFont(Arial_Black_16);
      if (sec > 5 && sec != 32  && sec != 12  ) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(6, 1, cstr);
        dmd.drawString(6, 17, cstr);
        dmd.drawString(6, 33, cstr);
        milliscountdown = millis();
        sec = sec - 1;
      }  else if (sec == 32) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(6, 1, cstr);
        dmd.drawString(6, 17, cstr);
        dmd.drawString(6, 33, cstr);
        milliscountdown = millis();
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);

        sec = sec - 1;
      } else if (sec == 12) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(6, 1, cstr);
        dmd.drawString(6, 17, cstr);
        dmd.drawString(6, 33, cstr);
        milliscountdown = millis();

        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);

        sec = sec - 1;
      } else if (sec == 5) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(6, 1, cstr);
        dmd.drawString(6, 17, cstr);
        dmd.drawString(6, 33, cstr);
        milliscountdown = millis();

        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);

        sec = sec - 1;
      } else if (sec == 4) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(6, 1, cstr);
        dmd.drawString(6, 17, cstr);
        dmd.drawString(6, 33, cstr);
        milliscountdown = millis();

        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);

        sec = sec - 1;
      } else if (sec == 3) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(6, 1, cstr);
        dmd.drawString(6, 17, cstr);
        dmd.drawString(6, 33, cstr);
        milliscountdown = millis();

        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);

        sec = sec - 1;
      } else if (sec == 2) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(6, 33, "GO");
        milliscountdown = millis();
        Serial1.println("<3513>"); //parte il rosso cronometro
        Serial1.println("<3001>"); //partono tutti i telecomandi dei tagli
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        sec = sec - 1;
      }
      else if (sec == 1) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(6, 17, "GO");
        dmd.drawString(6, 33, "GO");
        milliscountdown = millis();
        Serial1.println("<3523>"); // parte il verde
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        sec = sec - 1;
      }
      else if (sec == 0) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(6, 1, "GO");
        dmd.drawString(6, 17, "GO");
        dmd.drawString(6, 33, "GO");
        milliscountdown = millis();
        Serial1.println("<3533>"); // parte il blu
        //sirena
        startsirena = millis();
        digitalWrite(sirena, HIGH);
        sec = sec - 1;
      }
      else if (sec == -1) {
        char cstr[10];

        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.selectFont(SystemFont5x7);
        //        dmd.drawString(0, 33, temporosso[1]);
        //        dmd.drawString(18, 33, temporosso[2]);
        //        dmd.drawString(0, 17, tempoverde[1]);
        //        dmd.drawString(18, 17, tempoverde[2]);
        //        dmd.drawString(0, 1, tempoblu[1]);
        //        dmd.drawString(18, 1, tempoblu[2]);
        //rosso
        dmd.drawString(0, 32, "LP:" );
        dmd.drawString(20, 32, "0" );
        dmd.drawString(0, 40, "0.000" );
        //verde
        dmd.drawString(0, 16, "LP:" );
        dmd.drawString(20, 16, "0" );
        dmd.drawString(0, 24, "0.000" );
        //blu
        dmd.drawString(0, 0, "LP:" );
        dmd.drawString(20, 0, "0" );
        dmd.drawString(0, 8, "0.000" );
        milliscountdown = millis();
        flag = false;
        sec = 62;
      }

    }
  } else {

    milliscountdown = millis();
    flag = false;
    sec = 62;


  }
  return flag;
}
