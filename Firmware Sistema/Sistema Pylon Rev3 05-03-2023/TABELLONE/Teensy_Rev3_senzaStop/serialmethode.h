extern boolean newData1;
extern boolean newData2;
extern boolean newData3;
extern boolean newData4;
extern boolean newData5;
extern boolean newData6;
extern boolean newData7;
extern boolean newData8;
const byte numChars = 100;
extern  char receivedChars1[numChars];
extern char receivedChars2[numChars];
extern char receivedChars3[numChars];
extern char receivedChars4[numChars];
extern char receivedChars5[numChars];
extern char receivedChars6[numChars];
extern char receivedChars7[numChars];
extern char receivedChars8[numChars];

void  recvSerial1() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial1.available() > 0 && newData1 == false) {
    //while (port.available() > 0 ) {
    rc = Serial1.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars1[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars1[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData1 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial1() {
  if (newData1 == true) {
    Serial.println(receivedChars1);
    newData1 = false;
  }
}



void  recvSerial2() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial2.available() > 0 && newData2 == false) {
    //while (port.available() > 0 ) {
    rc = Serial2.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars2[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars2[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData2 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial2() {
  if (newData2 == true) {
    Serial.println(receivedChars2);
    newData2 = false;
  }
}
void  recvSerial3() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial3.available() > 0 && newData3 == false) {
    //while (port.available() > 0 ) {
    rc = Serial3.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars3[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars3[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData3 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial3() {
  if (newData3 == true) {
    Serial.println(receivedChars3);
    newData3 = false;
  }
}
void  recvSerial4() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial4.available() > 0 && newData4 == false) {
    //while (port.available() > 0 ) {
    rc = Serial4.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars4[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars4[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData4 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial4() {
  if (newData4 == true) {
    Serial.println(receivedChars4);
    newData4 = false;
  }
}


void  recvSerial5() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial5.available() > 0 && newData5 == false) {
    //while (port.available() > 0 ) {
    rc = Serial5.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars5[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars5[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData5 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial5() {
  if (newData5 == true) {
    Serial.println(receivedChars5);
    newData5 = false;
  }
}
void  recvSerial6() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial6.available() > 0 && newData6 == false) {
    //while (port.available() > 0 ) {
    rc = Serial6.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars6[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars6[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData6 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial6() {
  if (newData6 == true) {
    Serial.println(receivedChars6);
    newData6 = false;
  }
}
void  recvSerial7() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial7.available() > 0 && newData7 == false) {
    //while (port.available() > 0 ) {
    rc = Serial7.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars7[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars7[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData7 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial7() {
  if (newData7 == true) {
    Serial.println(receivedChars7);
    newData7 = false;
  }
}

void  recvSerial8() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  while (Serial8.available() > 0 && newData8 == false) {
    //while (port.available() > 0 ) {
    rc = Serial8.read();

    if (recvInProgress == true) {


      if (rc != endMarker) {
        receivedChars8[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars8[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData8 = true;
      }

    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

}

void showSerial8() {
  if (newData8 == true) {
    Serial.println(receivedChars8);
    newData8 = false;
  }
}
