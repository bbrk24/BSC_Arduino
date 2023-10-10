void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(1, PinMode::INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
  int v = analogRead(1);
  Serial.println(v);
}
