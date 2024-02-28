#if false

#ifndef PAYLOAD_THRESHOLD
#define PAYLOAD_THRESHOLD 14.5 // height of one floor in Baldwin staircase in units of feet
#endif

#ifndef COUNT_THRESHOLD
#define COUNT_THRESHOLD 200
#endif

#include "Altimeter.h"

Altimeter m_altimeter;

int numAltimeterReadings = 0;

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
    float altitudeOutput = m_altimeter.getAltitude();

    numAltimeterReadings += 1;

    if (altitudeOutput > PAYLOAD_THRESHOLD && numAltimeterReadings > COUNT_THRESHOLD) {
      // if threshold is exceeded. then turn on relay
      pinMode(8, OUTPUT);
      digitalWrite(8, LOW); //Sets up pin 8 to be the signal to eject capsules
    }
  }
}