#if true

#include "wiring_private.h"

// 1 for camera, 2 for atmospheric sensors
#define CAPSULE 2

#include "altimeter.h"
#include "imu.h"
#include "humidity.h"
#include "gps.h"
#include "frame.h"

Altimeter alt;

/*
On the Arduino MKR series, all pins for a I2C/UART/SPI instance must be on the same "sercom" object.
Additionally, for I2C, only some pins are suitable for clock and only some pins are suitable for
data. There are six sercoms:

sercom0: 2, 3, 11 (SDA), 12 (SCL), A3 (SDA), A4 (SCL), A5, A6
sercom1: 8 (SDA), 9 (SCL), 10
sercom2: 2, 3, 11 (SDA), 12 (SCL), <SD>
sercom3: 0 (SDA), 1 (SCL), 6, 7, 8 (SDA), 9 (SCL), 10, <USB>
sercom4: 4, 5, <SD>
sercom5: 0 (SDA), 1 (SCL), 13, 14, A1 (SDA), A2 (SCL)

Pins not marked as "SDA" or "SCL" can only be used for SPI or UART.

Information derived from
https://raw.githubusercontent.com/arduino/ArduinoCore-samd/master/variants/mkrzero/variant.cpp
and
https://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D21_DA1_Family_DataSheet_DS40001882F.pdf

There is also some additional code that must be added, as per
https://docs.arduino.cc/tutorials/communication/SamdSercom#create-a-new-wire-instance

On the Arduino Nano series, pins 0 and 1 must be disconnected while uploading code or using the
built-in `Serial`. The Nano series does not have sercoms, so I test for their presence with
`#ifdef _SERCOM_CLASS_`.
*/

static TwoWire imuI2C(
#ifdef _SERCOM_CLASS_
  &sercom1,
#endif
  /*sda:*/8,
  /*scl:*/9
);
IMU imu(&imuI2C);

#if CAPSULE == 2
static TwoWire humidityI2C(
#ifdef _SERCOM_CLASS_
  &sercom0,
  /*sda:*/A3,
  /*scl:*/A4
#else
  // A4 is taken by the altimeter on the nano, so route this elsewhere
  /*sda:*/4,
  /*scl:*/5
#endif
);
HumiditySensor hum(&humidityI2C);
#endif

static TwoWire gpsI2C(
#ifdef _SERCOM_CLASS_
  &sercom3,
#endif
  /*sda:*/0,
  /*scl:*/1
);
GPS gps(&gpsI2C);

#ifdef _SERCOM_CLASS_
extern "C" {
void SERCOM1_Handler(void) {
  imuI2C.onService();
}

void SERCOM0_Handler(void) {
  humidityI2C.onService();
}

void SERCOM3_Handler(void) {
  gpsI2C.onService();
}
} // extern "C"
#endif

void updateSensorLEDs() {
  IMU::Status imuStatus = imu.getStatus();
  Altimeter::Status altimeterStatus = alt.getStatus();
#if CAPSULE == 2
  HumiditySensor::Status humidityStatus = hum.getStatus();
#endif
  bool good =
    altimeterStatus == Altimeter::ACTIVE
    && imuStatus == IMU::ACTIVE
#if CAPSULE == 2
    && humidityStatus == HumiditySensor::ACTIVE
#endif
  ;
  digitalWrite(7, good);
}

void updateGPSLEDs() {
  bool good = gps.getStatus() == GPS::ACTIVE;
  digitalWrite(6, good);
}

// TODO: updateSDCardLEDs
// Without having an SD card implementation, I don't even know what to check.

void setup() {
#ifdef _SERCOM_CLASS_
  // Since the RTS and CTS pins are private, I have to initialize a whole new object, rather than
  // just assigning those two pins
  Serial1 = Uart(
    // First five arguments are the same as default
    &sercom5, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX,
    // and then:
    /*rts:*/A1, /*cts:*/A2
  );
#endif

  Serial.begin(9600);

  // Pin A6: analog input from VOC sensor
  pinMode(A6, PinMode::INPUT);
  // Pin D5: SD LEDs
  pinMode(5, PinMode::OUTPUT);
  // Pin D6: GPS LEDs
  pinMode(6, PinMode::OUTPUT);
  // Pin D7: sensor LEDs
  pinMode(7, PinMode::OUTPUT);

#ifdef _SERCOM_CLASS_
  pinPeripheral(0, PIO_SERCOM);
  pinPeripheral(1, PIO_SERCOM);
  pinPeripheral(8, PIO_SERCOM);
  pinPeripheral(9, PIO_SERCOM);
  pinPeripheral(A3, PIO_SERCOM);
  pinPeripheral(A4, PIO_SERCOM);
#endif

  // Turn on the error LEDs until initialization finishes
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  while (!Serial) { /* wait for serial port to connect */ }

  // Initialize sensors
  alt.initialize();
  imu.initialize();
#if CAPSULE == 2
  hum.initialize();
#endif
  updateSensorLEDs();
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
  Serial.print(", ");
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
  Serial.println(loc.timestamp.milliseconds);
}

void loop() {
  delay(500);

#if CAPSULE == 2
  int v = analogRead(6);
  Serial.print("VOC voltage: ");
  Serial.println(3.3F / 1023.0F * (float)v);
#endif

  if (alt.getStatus() == Altimeter::ACTIVE) {
    float altitude = alt.getAltitude();
    Serial.print("Altitude (feet): ");
    Serial.println(altitude);
  } else {
    alt.initialize();
    updateSensorLEDs();
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
    updateSensorLEDs();
  }

#if CAPSULE == 2
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
    updateSensorLEDs();
  }
#endif

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
