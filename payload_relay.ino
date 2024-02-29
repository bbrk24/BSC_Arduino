#if false

#include "Altimeter.h"

const float PAYLOAD_THRESHOLD = 14.5; // the height of one floor in Baldwin Hall staircase (ft)
const float COUNT_THRESHOLD = 200; // number of measurements to take

Altimeter m_altimeter;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
  m_altimeter.initialize();
  pinMode(8, OUTPUT); // sets pin D8 to be in output mode
}

void loop() {
  if (m_altimeter.getStatus() != Altimeter::ACTIVE) {
    Serial.print("Not connected. Trying again in 1 second....");
    delay(1000);
    m_altimeter.initialize();
  } else {
    float altitudeOutput = m_altimeter.getAltitude();
    static int numAltimeterReadings = 0;

    if (altitudeOutput > PAYLOAD_THRESHOLD) {
      numAltimeterReadings += 1;
      
      if (numAltimeterReadings > COUNT_THRESHOLD) {
        digitalWrite(8, HIGH); // activates the relay pin
      }
    }
  }
}

#endif
