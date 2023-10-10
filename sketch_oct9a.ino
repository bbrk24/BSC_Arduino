
#include <Adafruit_MPL3115A2.h>

static Adafruit_MPL3115A2 baro;
static bool begun = false;

void setup() {
  Serial.begin(9600);

  begun = baro.begin();
  if (begun) {
    // idk, copied from the example
    baro.setSeaPressure(1013.26);
  } else {
    Serial.println("Failed to set up baro sensor.");
  }

  // Pin A1: analog input from VOC sensor
  pinMode(1, PinMode::INPUT);
}

void loop() {
  delay(500);
  int v = analogRead(1);

  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);

  if (begun) {
    float altitude = baro.getAltitude();
    Serial.print("Altitude (meters): ");
    Serial.println(altitude);

    float pressure = baro.getPressure();
    Serial.print("Pressure (hPa): ");
    Serial.println(pressure);
    
    float temperature = baro.getTemperature();
    Serial.print("Temperature: ");
    Serial.println(temperature);
  }
}
