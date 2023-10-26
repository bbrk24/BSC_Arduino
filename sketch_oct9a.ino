#if true

#include "altimeter.h"
#include "imu.h"

Altimeter alt;

// The altimeter is already using the default pins, so wire this somewhere else
static TwoWire imuI2C(
// difference between Nano and MKRZero
#ifdef _SERCOM_CLASS_
  &sercom5,
#endif
  /*sda:*/6,
  /*scl:*/7
);
IMU imu(&imuI2C, 5);

void updateStatusLEDs() {
  IMU::Status imuStatus = imu.getStatus();
  Altimeter::Status altimeterStatus = alt.getStatus();
  bool good = (altimeterStatus == Altimeter::ACTIVE) && (imuStatus == IMU::ASLEEP || imuStatus == IMU::ACTIVE);
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

  // Turn on the error LED until initialization finishes
  digitalWrite(10, HIGH);

  // Initialize sensors
  alt.initialize();
  imu.initialize();
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

  float magnitude = IMU::getMagnitude(accel);
  Serial.print(magnitude);
}

void loop() {
  delay(500);

  int v = analogRead(1);
  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);

  if (alt.getStatus() == Altimeter::ACTIVE) {
    float altitude = alt.getAltitude();
    Serial.print("Altitude (feet): ");
    Serial.println(altitude);
  } else {
    alt.initialize();
    updateStatusLEDs();
  }

  if (imu.getStatus() == IMU::ACTIVE) {
    sh2_Accelerometer_t accel;
    sh2_Gyroscope_t gyro;
    if (imu.getValues(&accel, &gyro)) {
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
    imu.initialize();
    updateStatusLEDs();
  }
}

#endif
