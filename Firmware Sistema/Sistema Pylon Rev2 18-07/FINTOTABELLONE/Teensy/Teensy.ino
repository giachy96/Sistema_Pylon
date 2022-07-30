
String dataS;
unsigned long Timesend = 0;
unsigned long Currentmillis = 0;
unsigned long Delaysend = 1000;
int Stoppress ;

 int inByte = 200;

void setup() {
  pinMode(38, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(40, INPUT_PULLUP);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  Serial4.begin(9600);
  Serial5.begin(9600);
  Serial6.begin(9600);
  Serial7.begin(9600);
  Serial8.begin(9600);

}


// the loop routine runs over and over again forever:
void loop() {
  //  Currentmillis = millis();
  //  if (digitalRead(38) == HIGH & Stoppress == 0) {

 // dataS = "STOP@";
  //Serial3.println(dataS); //Write the serial data

    inByte = 300;
    Serial3.print(inByte, DEC);
  //    Serial.println(dataS); //Write the serial data
  //    Timesend = millis();
  //    Stoppress = 1;
  //  }
  delay(5000);
  //dataS = "OK@";
 // Serial3.println(dataS); //Write the serial data
    inByte = 200;
    Serial3.print(inByte, DEC);
  delay(5000);

  //  if ((Currentmillis - Timesend) > Delaysend ) { //If statements who control that state was change and
  //
  //    Stoppress = 0;
  //    dataS = "GO";
  //    Serial3.println(dataS);

//}

}
