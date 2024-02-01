#if true

#define BUFFER_SIZE 15
#define CAPSULE 1

#include "altimeter.h"
#include "Buffer.h"
#include "SD_Card.h"

Altimeter alt;
Buffer buf;
SDCard card;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
  alt.initialize();
  card.initialize();

  // Loop for 20 minutes
  while (millis() < 20 * 60 * 1000) {
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
      const float SEA_LEVEL_PRESSURE = 1013.25F; // sea level pressure in hPa
      // alt.setSeaLevel(SEA_LEVEL_PRESSURE);
      float altitude = alt.getAltitude();
      buf.addPoint(altitude);
      Serial.print("Recent max: ");
      Serial.print(buf.maximum());
      Serial.print(", min: ");
      Serial.println(buf.minimum());

      // checks if we're above our origin launch altitude
      if (altitude > -2) {
        Serial.print("Altitude: ");
        Serial.print(altitude);
        Serial.println(" ft");
        delay(1000);
      } else {
        Serial.print("Rocket has not gone up into the air. Trying again in ");
        Serial.print(backoff);
        Serial.println("ms...");
        delay(backoff);
        // backoff *= 2;
      }

      GPS::Coordinates fakeCoords = {0};
      IMU::vector3 fakeAccel = {0};
      IMU::vector3 gyro = {0};
      card.writeToCSV(fakeCoords, fakeAccel, altitude, gyro);
    }
  }

  card.closeFile();
  Serial.println("Done!");
}

void loop() {}

#endif
