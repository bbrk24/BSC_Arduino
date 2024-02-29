#if false

#include "Altimeter.h"

const float PAYLOAD_THRESHOLD = 14.5; // the height of one floor in Baldwin Hall staircase (ft)
const float COUNT_THRESHOLD 200; // I THINK this is how we rate limit the frequency?

Altimeter m_altimeter;

int numAltimeterReadings = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
  m_altimeter.initialize();
  pinMode(8, OUTPUT);
}

void loop() {
  if (m_altimeter.getStatus() != Altimeter::ACTIVE) {
    Serial.print("Not connected. Trying again in 1 second....");
    delay(1000);
    m_altimeter.initialize();
  } else {
    float altitudeOutput = m_altimeter.getAltitude();

    if (altitudeOutput > PAYLOAD_THRESHOLD && numAltimeterReadings > COUNT_THRESHOLD) {
      digitalWrite(8, LOW); //Sets up pin 8 to be the signal to eject capsules
    }

    numAltimeterReadings += 1;
  }
}
