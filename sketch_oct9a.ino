#include <Adafruit_MPL3115A2.h>
#include <Adafruit_BNO08x.h>

static Adafruit_MPL3115A2 baro;
static bool baro_begun = false;

static Adafruit_BNO08x bno08x;
// The altimeter is already using the default pins, so wire this somewhere else
TwoWire imuI2C(/*sda:*/A6, /*scl:*/A7);
static bool bno08x_begun = false;

void printAcceleration(float x, float y, float z);

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* wait for serial port to connect */ }

  baro_begun = baro.begin();
  if (baro_begun) {
    // idk, copied from the example
    baro.setSeaPressure(1013.26);
  } else {
    Serial.println("Failed to set up altimeter.");
  }

  bno08x_begun = bno08x.begin_I2C(BNO08x_I2CADDR_DEFAULT, &imuI2C);
  if (bno08x_begun) {
    // idk man
    bno08x.enableReport(SH2_ACCELEROMETER) || bno08x.enableReport(SH2_LINEAR_ACCELERATION) || bno08x.enableReport(SH2_RAW_ACCELEROMETER);
    bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED) || bno08x.enableReport(SH2_RAW_GYROSCOPE);
  } else {
    Serial.println("Failed to set up IMU.");
  }

  // Pin A1: analog input from VOC sensor
  pinMode(A1, PinMode::INPUT);
}

void loop() {
  delay(500);
  int v = analogRead(1);

  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);

  if (baro_begun) {
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

  if (bno08x_begun) {
    sh2_SensorValue_t value;
    if (bno08x.getSensorEvent(&value)) {
      switch (value.sensorId) {
      case SH2_ACCELEROMETER: {
        Serial.print("Accelerometer: ");
        printAcceleration(value.un.accelerometer.x, value.un.accelerometer.y, value.un.accelerometer.z);
        Serial.print('\n');
        break;
      }
      case SH2_LINEAR_ACCELERATION:
        Serial.print("Linear acceleration:");
        Serial.print("Accelerometer: ");
        printAcceleration(value.un.linearAcceleration.x, value.un.linearAcceleration.y, value.un.linearAcceleration.z);
        Serial.print('\n');
        break;
      case SH2_RAW_ACCELEROMETER:
        Serial.print("Accelerometer (raw): (");
        printAcceleration(value.un.rawAccelerometer.x, value.un.rawAccelerometer.y, value.un.rawAccelerometer.z);
        Serial.print(" @ ");
        Serial.println(value.un.rawAccelerometer.timestamp);
        break;
      
      case SH2_GYROSCOPE_CALIBRATED:
        Serial.print("Gyroscope: (");
        Serial.print(value.un.gyroscope.x);
        Serial.print(", ");
        Serial.print(value.un.gyroscope.y);
        Serial.print(", ");
        Serial.print(value.un.gyroscope.z);
        Serial.println(')');
        break;
      case SH2_RAW_GYROSCOPE:
        Serial.print("Gyroscope (raw): (");
        Serial.print(value.un.rawGyroscope.x);
        Serial.print(", ");
        Serial.print(value.un.rawGyroscope.y);
        Serial.print(", ");
        Serial.print(value.un.rawGyroscope.z);
        Serial.print(") @ ");
        Serial.print(value.un.rawGyroscope.timestamp);
        Serial.print("; T = ");
        Serial.println(value.un.rawGyroscope.temperature);
        break;
      
      default:
        Serial.print("Unexpected sensor ID ");
        Serial.println(value.sensorId);
      }
    }
  }
}

void printAcceleration(float x, float y, float z) {
  Serial.print('(');
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.print(z);
  Serial.print(") - magnitude ");

  float magnitude = sqrtf(x*x + y*y + z*z);
  Serial.print(magnitude);
}
