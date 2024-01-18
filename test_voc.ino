#if false

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
}

void loop() {
  delay(500);

  int v = analogRead(6);
  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);
}

#endif