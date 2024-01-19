#if false

void setup() {
	Serial.begin(9600);
	while (!Serial) { /* do nothing here */ }
}

void loop() {
  delay(500);

  int v = analogRead(6);
  Serial.print("VOC voltage (V): ");
  Serial.println(3.3F / 1023.0F * (float)v);
}

#endif
