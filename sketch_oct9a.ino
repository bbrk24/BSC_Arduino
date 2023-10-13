#include "altimeter.h"
#include "imu.h"

void updateStatusLEDs() {
  imu::status imuStatus = imu::getStatus();
  altimeter::status altimeterStatus = altimeter::getStatus();
  bool good = (altimeterStatus == altimeter::ACTIVE) && (imuStatus == imu::ASLEEP || imuStatus == imu::ACTIVE);
  digitalWrite(9, good);
  digitalWrite(10, !good);
}

void goToSleep() {
  imu::sleep();
  digitalWrite(4, HIGH);
}

void wakeUp() {
  imu::initialize();
  digitalWrite(4, LOW);
  updateStatusLEDs();
}

void setup() {
  Serial.begin(9600);

  // Pin A1: analog input from VOC sensor
  pinMode(A1, PinMode::INPUT);
  // Pin D4: VOC enable
  pinMode(4, PinMode::OUTPUT);
  // Pin D9: OK LED
  pinMode(9, PinMode::OUTPUT);
  // Pin D10: Error LED
  pinMode(10, PinMode::OUTPUT);

  // Initialize sensors
  altimeter::initialize();
  imu::initialize();
  digitalWrite(4, LOW);
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

  float magnitude = imu::getMagnitude(accel);
  Serial.print(magnitude);
}

void loop() {
  delay(500);

  int v = analogRead(1);
  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);

  if (altimeter::getStatus() == altimeter::ACTIVE) {
    float altitude = altimeter::getAltitude();
    Serial.print("Altitude (feet): ");
    Serial.println(altitude);
  } else {
    altimeter::initialize();
    updateStatusLEDs();
  }

  if (imu::getStatus() == imu::ACTIVE) {
    sh2_Accelerometer_t accel;
    sh2_Gyroscope_t gyro;
    if (imu::getValues(&accel, &gyro)) {
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
    imu::initialize();
    updateStatusLEDs();
  }
}
