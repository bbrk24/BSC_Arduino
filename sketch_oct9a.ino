#include <Adafruit_MPL3115A2.h>
#include <Adafruit_BNO08x.h>

static Adafruit_MPL3115A2 baro;
static bool baro_begun = false, baro_reasonable = false;

static Adafruit_BNO08x bno08x;
// The altimeter is already using the default pins, so wire this somewhere else
TwoWire imuI2C(/*sda:*/A6, /*scl:*/A7);
static bool bno08x_begun = false, bno08x_reasonable = false;

// Pass nullptr for either one to indicate you don't need it
bool getIMUValues(sh2_Accelerometer_t* accel, sh2_Gyroscope_t* gyro) {
  sh2_SensorValue_t value;
  if (!bno08x.getSensorEvent(&value)) {
    // Neither one is ready
    return false;
  }

  bool accel_found = (accel == nullptr), gyro_found = (gyro == nullptr);

  while (true) {
    switch (value.sensorId) {
    case SH2_ACCELEROMETER:
      accel_found = true;
      if (accel != nullptr) {
        *accel = value.un.accelerometer;
      }
      break;
    case SH2_GYROSCOPE_CALIBRATED:
      gyro_found = true;
      if (gyro != nullptr) {
        *gyro = value.un.gyroscope;
      }
      break;
    default:
      Serial.print("Unexpected sensor ID ");
      Serial.println(value.sensorId);
    }

    // If both are found, we're done
    if (accel_found && gyro_found) {
      return true;
    }
    // Otherwise, wait for the next sensor reading
    while (!bno08x.getSensorEvent(&value)) { /* This loop intentionally left blank */ }
  }
}

float getMagnitude(const sh2_Accelerometer_t& accel) {
  return sqrtf(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
}

void initializeAltimeter() {
  if (!baro_begun) {
    baro_begun = baro.begin();
  }
  if (baro_begun && !baro_reasonable) {
    float basePressure = baro.getPressure();
    baro_reasonable = basePressure >= 929.0F && basePressure <= 1041.0F;
    if (baro_reasonable) {
      baro.setSeaPressure(basePressure);
    }
  }
}

void initializeIMU() {
  if (!bno08x_begun) {
    bno08x_begun = bno08x.begin_I2C(BNO08x_I2CADDR_DEFAULT, &imuI2C);
    if (bno08x_begun) {
      bno08x.enableReport(SH2_ACCELEROMETER);
      bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED);
      // Give it time to get measurements
      // The datasheet claims a reset takes 90ms + 4ms, and it operates at 100Hz
      delay(104);
    }
  }

  if (bno08x_begun && !bno08x_reasonable) {
    sh2_Accelerometer_t accel;
    if (getIMUValues(&accel, nullptr)) {
      float magnitude = getMagnitude(accel);
      bno08x_reasonable = 9.6F <= magnitude && magnitude <= 10.0F;
    }
  }
}

void updateStatusLEDs() {
  bool good = baro_reasonable && bno08x_reasonable;
  digitalWrite(9, good);
  digitalWrite(10, !good);
}

void setup() {
  Serial.begin(9600);

  // Pin A1: analog input from VOC sensor
  pinMode(A1, PinMode::INPUT);
  // Pin D9: OK LED
  pinMode(9, PinMode::OUTPUT);
  // Pin D10: Error LED
  pinMode(10, PinMode::OUTPUT);

  // Initialize sensors
  initializeAltimeter();
  initializeIMU();
  updateStatusLEDs();

  while (!Serial) { /* wait for serial port to connect */ }
}

void printAcceleration(const sh2_Accelerometer_t& accel) {
  Serial.print('(');
  Serial.print(accel.x);
  Serial.print(", ");
  Serial.print(accel.y);
  Serial.print(", ");
  Serial.print(accel.z);
  Serial.print(") - magnitude ");

  float magnitude = getMagnitude(accel);
  Serial.print(magnitude);
}

void loop() {
  delay(500);

  int v = analogRead(1);
  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);

  if (baro_reasonable) {
    float altitude = baro.getAltitude();
    Serial.print("Altitude (meters): ");
    Serial.println(altitude);

    float temperature = baro.getTemperature();
    Serial.print("Temperature (Celsius): ");
    Serial.println(temperature);
  } else {
    initializeAltimeter();
    updateStatusLEDs();
  }

  if (bno08x_reasonable) {
    sh2_Accelerometer_t accel;
    sh2_Gyroscope_t gyro;
    if (getIMUValues(&accel, &gyro)) {
      Serial.print("Acceleration (m/s^2): ");
      printAcceleration(accel);
      Serial.print("\nGyroscope (rad/s): (");
      Serial.print(gyro.x);
      Serial.print(", ");
      Serial.print(gyro.y);
      Serial.print(", ");
      Serial.print(gyro.z);
      Serial.println(')');
    } else {
      Serial.println("No sensor value read");
    }
  } else {
    initializeIMU();
    updateStatusLEDs();
  }
}
