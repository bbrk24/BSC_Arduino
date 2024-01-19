#if false

#include "altimeter.h"

Altimeter alt;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
  alt.initialize();
}

void loop() {
  // Exponential backoff: each time it waits longer than the last.
  // That way, if a wire is loose or missing, it doesn't flood the console with messages.
  static unsigned int backoff = 100;

  if (alt.getStatus() != Altimeter::ACTIVE) {
    Serial.print("Not connected. Trying again in ");
    Serial.print(backoff);
    Serial.println("ms...");
    delay(backoff);
    backoff *= 2;
    alt.initialize();
  } else {
    float altitude = alt.getAltituide();
    const float SEA_LEVEL_PRESSURE = 1013.25F; // sea level pressure in hPa
    setSeaLevel(SEA_LEVEL_PRESSURE);

    if (altitude > m_seaLevel) {
      Serial.print("Altitude: ");
      Serial.print(altitude);
      Serial.println("m");
      delay(102);
    } else {
      Serial.print("No values available to be read. Trying again in ");
      Serial.print(backoff);
      Serial.println("ms...");
      delay(backoff);
      backoff *= 2;
    }
  }
}

#endif
