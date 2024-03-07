#if false

#define CAPSULE 2

#include "humidity.h"
#include "SD_Card.h"

// Since there's only the one sensor for this test, there's no reason to mess around with sercoms.
// Just assign it to the default I2C port.
HumiditySensor hum(&Wire);

SDCard card;

void setup() {
  pinMode(6, PinMode::OUTPUT);
  digitalWrite(6, HIGH);

  // wait a full day
  while (millis() < 24 * 60 * 60 * 1000) {
    // blink the LED *slowly*
    delay(1000);
    digitalWrite(6, LOW);
    delay(29000);
    digitalWrite(6, HIGH);
  }

  while (hum.getStatus() != HumiditySensor::ACTIVE) {
    hum.initialize();
  }
  while (card.getStatus() != SDCard::ACTIVE) {
    card.initialize();
  }

  GPS::Coordinates fakeCoords{0};
  IMU::vector3 accel{0};
  IMU::vector3 gyro{0};

  float humidity, temperature;

  // get multiple readings
  for (int i = 0; i < 50; ++i) {
    // wait until 5ms has passed
    delayMicroseconds(5000 - (micros() % 5000));

    // get the values until they succeed
    while (!hum.getValues(&humidity, &temperature)) {}

    card.writeToCSV(fakeCoords, accel, 0, 0, humidity, temperature, gyro);
  }

  card.closeFile();
  digitalWrite(6, LOW);
}

void loop() {
}

#endif
