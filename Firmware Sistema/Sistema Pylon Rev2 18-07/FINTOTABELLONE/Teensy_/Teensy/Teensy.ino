
String msg;


void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
}


// the loop routine runs over and over again forever:
void loop() {
  msg = "STOP";
  Serial3.print(msg);
  delay(5000);
  msg = "GO";
  Serial3.print(msg);
  delay(5000);
}
