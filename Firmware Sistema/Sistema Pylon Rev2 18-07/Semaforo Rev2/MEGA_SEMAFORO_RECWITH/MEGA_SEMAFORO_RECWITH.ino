  #include "LoRa_E22.h"
  LoRa_E22 e22ttl(10, 11); // Arduino RX --> e22 TX - Arduino TX --> e22 RX
  
  unsigned long Timeserial1=0;
  unsigned long Timeserial2=0;
  unsigned long Timeserial3=0;
  unsigned long Timesend=0;
  unsigned long Currentmilliss=0;
  unsigned long Currentmillisx=0;
  unsigned long DelayLight=2000;
  unsigned long Delayserialx=0;
  unsigned long Delaysend=200;
  String Dateserial1="0";
  String Dateserial2="0";
  String Dateserial3="0";
  String Dateserial1p="0";
  String Dateserial2p="0";
  String Dateserial3p="0";
  String Dateserial;
  int RelayRed=4;
  int RelayGreen=5;
  int RelayBlue=6;
  const byte numChars=32;
  char RecCh1[numChars];
  char RecCh2[numChars];
  char RecCh3[numChars];
  String Race="<300>";
  String Show="<200>";
  String Startup="<100>";
  String State=Show; //Startup state
  String OldState=Show;
  boolean newData1=false;
  boolean newData2=false;
  boolean newData3=false;
  
void setup() {
  Serial1.begin(9600); // Serial ColorRx
  delay(300);
  Serial2.begin(9600);// Serial ColorRx
  delay(300);
  Serial3.begin(9600);// Serial ColorRx
  delay(300);
  Serial.begin(9600);// Serial Pc interface
  delay(300);
  e22ttl.begin(); //Start e22ttl
  pinMode(RelayRed, OUTPUT); //Output pin to Relay Red
  pinMode(RelayBlue, OUTPUT); //Output pin to Relay Blue
  pinMode(RelayGreen, OUTPUT); //Output pin to Relay Green
  Serial.println("prova");
}
void loop() {
Currentmillisx=millis(); // Assign millis value to Currentmillisx for if statements
 if (State==Show || State==Race) { //Delay time for receive serial data, if necessary...
  RecStr1();
  if (newData1==true){ //(Serial1.available()>0){
    Serial.println("C'è qualcosa in seriale");
    String Dateserial1(RecCh1); //Assign incoming data from Serial1 to Dateserial1
    Serial.println(Dateserial1);
    digitalWrite(RelayRed,HIGH); //Turn on stoplight
    Timeserial1=millis(); // Assign millis value for turning off relay
    newData1=false;
    }
  }
 if ((Currentmillisx-Timeserial1) > DelayLight) { // if red light is on for more than Delaylight
   digitalWrite(RelayRed,LOW); //Turn off stoplight
   }

 Currentmillisx=millis();
 if (State==Show || State==Race) {
   RecStr2();
  if (newData2==true){//(Serial2.available()>0){
    Serial.println("C'è qualcosa in seriale");
    String Dateserial2(RecCh2); //Assign incoming data from Serial1 to Dateserial2
    digitalWrite(RelayGreen,HIGH);
    Timeserial2=millis();
    newData2=false;
    }
  }
  if ((Currentmillisx-Timeserial2) > DelayLight) {
    digitalWrite(RelayGreen,LOW);
    }

 Currentmillisx=millis();
 if (State==Show || State==Race) {
   RecStr3();
  if (newData3==true){//(Serial3.available()>0){
    Serial.println("C'è qualcosa in seriale");
    String Dateserial3(RecCh3); //Assign incoming data from Serial3 to Dateserial3
    digitalWrite(RelayBlue,HIGH);
    Timeserial3=millis();
    newData3=false;
    }
  }
  if ((Currentmillisx-Timeserial3) > DelayLight) {
    digitalWrite(RelayBlue,LOW);
    }

 Currentmilliss=millis(); // millis assignments for send delay control
  if ((Currentmilliss-Timesend) >= Delaysend && (State==Show || State==Race)) {  //If the last receive is older than Delaysend, and state is race or show
   if (Dateserial1 != "0" || Dateserial2 != "0" || Dateserial3 !="0")  { //If some button was pressed
    Dateserial=(Dateserial1); // Compose message for send with Lora
    Dateserial.concat(",");
    Dateserial.concat(Dateserial2);
    Dateserial.concat(",");
    Dateserial.concat(Dateserial3);
    ResponseStatus rs = e22ttl.sendFixedMessage(0, 1, 2, Dateserial); // Send fixedmessage Dateseria that contain all update of Dateserial1/2/3
    Serial.println(Dateserial); // DEBUG
    Dateserial1="0"; //Reset condition for received press button
    Dateserial2="0";
    Dateserial3="0";
    }
    }
  if (e22ttl.available()>1) { //If there's something incoming from lora
  ResponseContainer rc = e22ttl.receiveMessage(); //Receive message
    if (rc.status.code!=1){// Is something goes wrong
        rc.status.getResponseDescription(); //Get error response
    }
    else{ 
     State=rc.data; // if there isn't any error assign the incoming data to State
   }
    Timesend=millis(); // millis assigment for last data receive 
    }
  if (Serial.available()>0){ // DEBUG
  Serial.println("Ricezione Dato");
    State=Serial.readString(); // DEBUG
    Timesend=millis(); // DEBUG
    Serial.println(State);
  }
    

  if (OldState != State){ //If state was change on previous code
  Serial.println("Cambio stato");
  RecStr1();
  if (newData1==true){//(Serial1.available()>0){ //check if some press was incoming from receiver 1
    String Dateserial1p(RecCh1); //Assign incoming data from Serial1 to Dateserial1//Assign useless press on Dateserial1
    newData1=false;
    }
  Serial1.print(State); //Send to receiver 1 State
  RecStr2();
  if (newData2==true){//(Serial2.available()>0){
    String Dateserial2(RecCh2); //Assign incoming data from Serial1 to Dateserial1
    newData2=false;
    }
  Serial2.print(State);
  RecStr3();
  if (newData3==true){//(Serial3.available()>0){
    String Dateserial3p(RecCh3); //Assign incoming data from Serial1 to Dateserial1
    newData3==false;
    }
    Serial3.print(State);
  OldState=State; //Reset condition for send State to the rx
  }
}

void RecStr1() {
    static boolean recvInProgress1 = false;
    static byte ndx1 = 0;
    char startMarker1 = '<';
    char endMarker1 = '>';
    char rc1;
    boolean newData1=false;
 
    while (Serial1.available() > 0 && newData1 == false) {
        rc1 = Serial1.read();

        if (recvInProgress1 == true) {
            if (rc1 != endMarker1) {
                RecCh1[ndx1] = rc1;
                ndx1++;
                if (ndx1 >= numChars) {
                    ndx1 = numChars - 1;
                }
            }
            else {
                RecCh1[ndx1] = '\0'; // terminate the string
                recvInProgress1 = false;
                ndx1 = 0;
                newData1 = true;
            }
        }

        else if (rc1 == startMarker1) {
            recvInProgress1 = true;
        }
    }
    Serial1.flush();
}

void RecStr2() {
    static boolean recvInProgress2 = false;
    static byte ndx2 = 0;
    char startMarker2 = '<';
    char endMarker2 = '>';
    char rc2;
  boolean newData2=false;
    while (Serial2.available() > 0 && newData2 == false) {
        rc2 = Serial2.read();

        if (recvInProgress2 == true) {
            if (rc2 != endMarker2) {
                RecCh2[ndx2] = rc2;
                ndx2++;
                if (ndx2 >= numChars) {
                    ndx2 = numChars - 1;
                }
            }
            else {
                RecCh2[ndx2] = '\0'; // terminate the string
                recvInProgress2 = false;
                ndx2 = 0;
                newData2 = true;
            }
        }

        else if (rc2 == startMarker2) {
            recvInProgress2 = true;
        }
    }
    Serial2.flush();
}

void RecStr3() {
  const byte numChars3=32;
  boolean newData3=false;
    static boolean recvInProgress3 = false;
    static byte ndx3 = 0;
    char startMarker3 = '<';
    char endMarker3 = '>';
    char rc3;

    while (Serial3.available() > 0 && newData3 == false) {
        rc3 = Serial3.read();

        if (recvInProgress3 == true) {
            if (rc3 != endMarker3) {
                RecCh3[ndx3] = rc3;
                ndx3++;
                if (ndx3 >= numChars) {
                    ndx3 = numChars - 1;
                }
            }
            else {
                RecCh3[ndx3] = '\0'; // terminate the string
                recvInProgress3 = false;
                ndx3 = 0;
                newData3 = true;
            }
        }

        else if (rc3 == startMarker3) {
            recvInProgress3 = true;
        }
    }
    Serial3.flush();
}
