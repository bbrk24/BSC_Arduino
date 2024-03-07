#if false

#define CAPSULE 2

#include "humidity.h"
#include "SD_Card.h"

HumiditySensor hum(&Wire);
SDCard card;

void setup() {
  // The temperature sensor has a time constant of 2s
  const double TIME_CONSTANT = 2.0;
  // The temperature range of the sensor is -40 to +125, for a range of 165
  const double TEMP_RANGE = 165.0;
  // The precision of the sensor is +/- 0.2
  const double PRECISION = 0.2;
  // Plug that into the formula, then go 20% longer to be safe
  // Also multiply by a million to convert to us
  const unsigned long RUNTIME = 1000000 * 1.2 * (-TIME_CONSTANT * log(PRECISION / TEMP_RANGE));
  // Time between measurements, in us
  const unsigned long DELAY = 5000;

  // connect to the sensors
  while (hum.getStatus() != HumiditySensor::ACTIVE) {
    hum.initialize();
  }
  while (card.getStatus() != SDCard::ACTIVE) {
    card.initialize();
  }

  GPS::Coordinates fakeCoords{0};
  IMU::vector3 accel{0};
  IMU::vector3 gyro{0};

  unsigned long startTime = micros();
  unsigned long currTime;
  do {
    float temperature;
    // No need to read humidity
    while (!hum.getValues(nullptr, &temperature)) {
      // wait for the values to be read successfully
    }

    currTime = micros();
    fakeCoords.timestamp.milliseconds = (currTime / 1000) % 1000;
    fakeCoords.timestamp.seconds = currTime / 1000000;
    // not going to go long enough to worry about minutes

    card.writeToCSV(fakeCoords, accel, 0, 0, 0, temperature, gyro);

    // add some delay
    delayMicroseconds(DELAY - (currTime % DELAY));
  } while (currTime - startTime <= RUNTIME);

  // we're done
  card.closeFile();
}

void loop() {}

#endif