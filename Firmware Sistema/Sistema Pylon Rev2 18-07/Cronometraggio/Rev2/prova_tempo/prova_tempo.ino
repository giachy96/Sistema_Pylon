
unsigned long mil;
unsigned long delta;
unsigned long lastpress;

void setup() {
  pinMode(12, INPUT_PULLUP);
  Serial.begin(9600);

}

void loop() {
  mil = millis();



  if (digitalRead(12) == LOW ) {

    delta = mil - lastpress ;
    Serial.println(delta);
    lastpress = millis();
  }
}
