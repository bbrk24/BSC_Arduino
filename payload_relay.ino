#if false

#define PAYLOAD_THRESHOLD 14.5 // height of one floor in Baldwin staircase in units of feet

#include "Altimeter.h"

Altimeter m_altimeter;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
  m_altimeter.initialize();
}

void loop() {
  if (m_altimeter.getStatus() != Altimeter::ACTIVE) {
    Serial.print("Not connected. Trying again in 1 second....");
    delay(1000);
    m_altimeter.initialize();
  } else {
    float altitude_output = m_altimeter.getAltitude();

    if (altitude_output > PAYLOAD_THRESHOLD) {
      // if threshold is exceeded. then turn on relay
      pinMode(8, OUTPUT);
      digitalWrite(8, LOW); //Sets up pin 8 to be the signal to eject capsules
    }
  }
}