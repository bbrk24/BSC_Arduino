#pragma once

#include <Adafruit_BNO08x.h>

namespace imu {

Adafruit_BNO08x _bno08x(5);

// The altimeter is already using the default pins, so wire this somewhere else
TwoWire _theI2C(
// difference between Nano and MKRZero
#ifdef _SERCOM_CLASS_
  &sercom5,
#endif
  /*sda:*/6,
  /*scl:*/7
);

bool _registeredAccelerometer = false, _registeredGyroscope = false, _reasonableGravity = false;

enum status {
  /** I2C communications have not been established. */
  NOT_CONNECTED,
  /** Communications have been established, but the accelerometer is reading an unusual value for gravity. */
  UNREASONABLE,
  /** Communications have been established, but the sensors are not collecting data. */
  ASLEEP,
  /** The sensor is ready to be read. */
  ACTIVE,
};

/**
 * Check the sensor status.
 */
status getStatus() noexcept {
  if (!_registeredAccelerometer || !_registeredGyroscope) {
    if (_reasonableGravity) {
      // A reasonable value was read, but then the sensors were turned off.
      return status::ASLEEP;
    } else {
      // Nothing works
      return status::NOT_CONNECTED;
    }
  } else if (_reasonableGravity) {
    return status::ACTIVE;
  } else {
    return status::UNREASONABLE;
  }
}

/**
 * Get the accelerometer and/or gyroscope values.
 * @param[out] accel Pointer to the accelerometer values. Pass `nullptr` to only get gyroscope
 * values.
 * @param[out] gyro Pointer to the calibrated gyroscope values. Pass `nullptr` to only get
 * accelerometer values.
 * @return A boolean indicating whether the values were successfully read.
 */
bool getValues(sh2_Accelerometer_t* accel, sh2_Gyroscope_t* gyro) {
  if (accel == nullptr && gyro == nullptr) {
    // seriously?
    return true;
  }

  // Don't bother if the sensor didn't register properly
  if ((accel != nullptr && !_registeredAccelerometer) || (gyro != nullptr && !_registeredGyroscope)) {
    return false;
  }

  sh2_SensorValue_t value;
  if (!_bno08x.getSensorEvent(&value)) {
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
    while (!_bno08x.getSensorEvent(&value)) {
      yield();
    }
  }
}

/**
 * Get the magnitude of acceleration.
 */
float getMagnitude(const sh2_Accelerometer_t& accel) noexcept {
  return sqrtf(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
}

/**
 * Initialize communications with the IMU. Call this during initial setup or to wake the IMU from sleep.
 */
void initialize() {
  static bool begun = false;

  if (!begun) {
    begun = _bno08x.begin_I2C(BNO08x_I2CADDR_DEFAULT, &_theI2C);
  }

  if (begun) {
    if (!_registeredAccelerometer) {
      _registeredAccelerometer = _bno08x.enableReport(SH2_ACCELEROMETER);
    }
    if (!_registeredGyroscope) {
      _registeredGyroscope = _bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED);
    }

    if (!_reasonableGravity) {
      sh2_Accelerometer_t accel;
      while (!getValues(&accel, nullptr)) {
        delay(10);
      }

      float magnitude = getMagnitude(accel);
      _reasonableGravity = (9.6F <= magnitude && magnitude <= 10.0F);
    }
  }
}

/**
 * Stop collecting measurements.
 */
void sleep() {
  _bno08x.hardwareReset();
  _registeredAccelerometer = false;
  _registeredGyroscope = false;
}

} // namespace imu
