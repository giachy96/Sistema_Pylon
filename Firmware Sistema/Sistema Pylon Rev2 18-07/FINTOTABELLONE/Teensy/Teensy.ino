




void setup() {
  pinMode(38, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(40, INPUT_PULLUP);
  Serial.begin(9600);
  Serial3.begin(9600);

}

// the loop routine runs over and over again forever:
void loop() {

  if (digitalRead(38) == LOW) {
    Serial3.print("STOP");
  }

Serial3.println("OK");
delay(1000);
}
