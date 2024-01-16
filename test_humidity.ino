#if false

#include "humidity.h"

// Since there's only the one sensor for this test, there's no reason to mess around with sercoms.
// Just assign it to the default I2C port.
HumiditySensor hum(&Wire);

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* Wait for Serial monitor to connect */ }
  Serial.println("Connecting to sensor...");
  hum.initialize();
}

void loop() {
  // Exponential backoff: each time it waits longer than the last.
  // That way, if a wire is loose or missing, it doesn't flood the console with messages.
  static unsigned int backoff = 100;

  if (hum.getStatus() != HumiditySensor::ACTIVE) {
    Serial.print("Not connected. Trying again in ");
    Serial.print(backoff);
    Serial.println("ms...");
    delay(backoff);
    backoff *= 2;
    hum.initialize();
  } else {
    float humid, temp;
    if (hum.getValues(&humid, &temp)) {
      Serial.print("Humidity: ");
      Serial.print(humid);
      Serial.print("%RH, temperature: ");
      Serial.print(temp);
      Serial.println('C');
      // Optional delay to slow down the console
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
