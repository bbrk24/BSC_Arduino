#if true

#include "altimeter.h"
#include "imu.h"
#include "humidity.h"
#include "gps.h"

Altimeter alt;

static TwoWire imuI2C(
// difference between Nano and MKRZero
#ifdef _SERCOM_CLASS_
  &sercom5,
#endif
  /*sda:*/7,
  /*scl:*/6
);
IMU imu(&imuI2C);

static TwoWire humidityI2C(
#ifdef _SERCOM_CLASS_
  &sercom4,
#endif
  /*sda:*/4,
  /*scl:*/5
);
HumiditySensor hum(&humidityI2C);

static TwoWire gpsI2C(
#ifdef _SERCOM_CLASS_
  &sercom2,
#endif
  /*sda:*/3,
  /*scl:*/2
);
GPS gps(&gpsI2C);

void updateStatusLEDs() {
  IMU::Status imuStatus = imu.getStatus();
  Altimeter::Status altimeterStatus = alt.getStatus();
  HumiditySensor::Status humidityStatus = hum.getStatus();
  bool good =
    altimeterStatus == Altimeter::ACTIVE
    && imuStatus == IMU::ACTIVE
    && humidityStatus == HumiditySensor::ACTIVE;
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

  while (!Serial) { /* wait for serial port to connect */ }

  // Initialize sensors
  alt.initialize();
  imu.initialize();
  hum.initialize();
  updateStatusLEDs();
}

void printAcceleration(const IMU::vector3& accel) {
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

void printGPSData(const GPS::Coordinates& loc) {
  if (loc.latitude < 0) {
    Serial.print(-loc.latitude);
    Serial.print("S ");
  } else {
    Serial.print(loc.latitude);
    Serial.print("N ");
  }
  if (loc.longitude < 0) {
    Serial.print(-loc.longitude);
    Serial.print('W');
  } else {
    Serial.print(loc.longitude);
    Serial.print('E');
  }
  Serial.print(", ")
  Serial.print(loc.altitudeMSL);
  Serial.println("ft");

  Serial.print(loc.timestamp.hours);
  Serial.print(':');
  if (loc.timestamp.minutes < 10) {
    Serial.print('0');
  }
  Serial.print(loc.timestamp.minutes);
  Serial.print(':');
  if (loc.timestamp.seconds < 10) {
    Serial.print('0');
  }
  Serial.print(loc.timestamp.seconds);
  Serial.print('.');
  if (loc.timestamp.milliseconds < 100) {
    Serial.print('0');
  }
  if (loc.timestamp.milliseconds < 10) {
    Serial.print('0');
  }
  Serial.println(loc.timestamp.milliseconds)
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
    IMU::vector3 accel;
    IMU::vector3 gyro;
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
      Serial.println("No IMU value read");
    }
  } else {
    imu.initialize();
    updateStatusLEDs();
  }

  if (hum.getStatus() == HumiditySensor::ACTIVE) {
    float humidity;
    float temp;
    if (hum.getValues(&humidity, &temp)) {
      Serial.print("Humidity (%RH):");
      Serial.println(humidity);
      Serial.print("Temperature (C):");
      Serial.println(temp);
    } else {
      Serial.println("No humidity value read");
    }
  } else {
    hum.initialize();
    updateStatusLEDs();
  }

  if (gps.getStatus() == GPS::ACTIVE) {
    GPS::Coordinates loc;
    if (gps.getLocation(&loc)) {
      printGPSData(loc);
    } else {
      Serial.println("No GPS value read");
    }
  }
}

#endif
