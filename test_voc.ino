#if true

void setup() {
	Serial.begin(9600);
  // this uses the AREF pin to use 3.3 V instead of 5.0 V
  analogReference(EXTERNAL);
	while (!Serial) { /* do nothing here */ }
}

void loop() {
  delay(500);

  // had to change to pin 4 because of using Arduino Micro
  int v = analogRead(4);
  Serial.print("VOC voltage (V): ");
  Serial.println(3.3F / 1023.0F * (float)v);
}

#endif
