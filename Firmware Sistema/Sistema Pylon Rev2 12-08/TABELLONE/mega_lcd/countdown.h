extern unsigned long currentMillis;
extern unsigned long milliscountdown;
extern int sec;
extern int sirenaflag ;


boolean countdown(boolean flag) {

  if (flag == true) {


    if (currentMillis - milliscountdown >= 1000) {
      dmd.selectFont(Arial_Black_16);
      if (sec > 5 && sec != 32  && sec != 12  ) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        sirenaflag = 0;
        sec = sec - 1;
      }  else if (sec == 32) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 12) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 5) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 4) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 3) {

        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);
        dmd.clearScreen();
        dmd.drawString(24, 1, cstr);
        dmd.drawString(24, 17, cstr);
        dmd.drawString(24, 33, cstr);
        milliscountdown = millis();

        sirenaflag = 1;

        sec = sec - 1;
      } else if (sec == 2) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        Serial1.println("<3513>"); //parte il rosso cronometro
        Serial1.println("<3001>"); //partono tutti i telecomandi dei tagli
        sirenaflag = 1;
        sec = sec - 1;
      }
      else if (sec == 1) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 17, "GO");
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        Serial1.println("<3523>"); // parte il verde
        sirenaflag = 1;
        sec = sec - 1;
      }
      else if (sec == 0) {
        char cstr[10];

        int n = sec - 2;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 1, "GO");
        dmd.drawString(24, 17, "GO");
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        Serial.println(sec);
        Serial.println(flag);
        Serial1.println("<3533>"); // parte il blu
        sirenaflag = 1;
        sec = sec - 1;
      }
      else if (sec == -1) {
        char cstr[10];

        int n = sec - 3;
        itoa(n, cstr, 10);

        dmd.clearScreen();
        dmd.drawString(24, 1, "GO");
        dmd.drawString(24, 17, "GO");
        dmd.drawString(24, 33, "GO");
        milliscountdown = millis();
        sirenaflag = 0;
        flag = false;
        sec = 62;
        Serial.println(sec);
        Serial.println(flag);
      }

    }
  } else {

    milliscountdown = millis();
    flag = false;
    sec = 62;


  }
  return flag;
}
