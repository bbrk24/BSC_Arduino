#pragma once

#include <Adafruit_BNO08x.h>

class IMU {
private:
  Adafruit_BNO08x m_bno08x;
  TwoWire* m_i2c;
  bool m_registeredAccelerometer;
  bool m_registeredGyroscope;
  bool m_reasonableGravity;
public:
  /**
   * @param[in] theI2C Pointer to a TwoWire instance representing the I2C interface. Must be valid
   * for the whole lifetime of the IMU instance.
   * @param resetPin The pin on the Arduino connected to the IMU's reset pin. Use -1 if not connected.
   */
  IMU(TwoWire* theI2C, int8_t resetPin = -1) :
    m_bno08x(resetPin),
    m_i2c(theI2C),
    m_registeredAccelerometer(false),
    m_registeredGyroscope(false),
    m_reasonableGravity(false) {}

  enum Status {
    /** I2C communications have not been established. */
    NOT_CONNECTED,
    /** Communications have been established, but the accelerometer is reading an unusual value for gravity. */
    UNREASONABLE,
    /** Communications have been established, but the sensors are not collecting data. */
    ASLEEP,
    /** The sensor is ready to be read. */
    ACTIVE,
  };

  /** Check the sensor status. */
  Status getStatus() const noexcept {
    if (!m_registeredAccelerometer || !m_registeredGyroscope) {
      if (m_reasonableGravity) {
        // A reasonable value was read, but then the sensors were turned off.
        return Status::ASLEEP;
      } else {
        // Nothing works
        return Status::NOT_CONNECTED;
      }
    } else if (m_reasonableGravity) {
      return Status::ACTIVE;
    } else {
      return Status::UNREASONABLE;
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
    if ((accel != nullptr && !m_registeredAccelerometer) || (gyro != nullptr && !m_registeredGyroscope)) {
      return false;
    }

    sh2_SensorValue_t value;
    if (!m_bno08x.getSensorEvent(&value)) {
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
      while (!m_bno08x.getSensorEvent(&value)) {
        yield();
      }
    }
  }

  /** Get the magnitude of acceleration. */
  static float getMagnitude(const sh2_Accelerometer_t& accel) noexcept {
    return sqrtf(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
  }

  /** Initialize communications with the IMU. Call this during initial setup or to wake the IMU from sleep. */
  void initialize() {
    static bool begun = false;

    if (!begun) {
      begun = m_bno08x.begin_I2C(BNO08x_I2CADDR_DEFAULT, m_i2c);
    }

    if (begun) {
      if (!m_registeredAccelerometer) {
        m_registeredAccelerometer = m_bno08x.enableReport(SH2_ACCELEROMETER);
      }
      if (!m_registeredGyroscope) {
        m_registeredGyroscope = m_bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED);
      }

      if (!m_reasonableGravity) {
        sh2_Accelerometer_t accel;
        while (!getValues(&accel, nullptr)) {
          delay(10);
        }

        float magnitude = getMagnitude(accel);
        m_reasonableGravity = (9.6F <= magnitude && magnitude <= 10.0F);
      }
    }
  }

  /** Stop collecting measurements. */
  void sleep() {
    m_bno08x.hardwareReset();
    m_registeredAccelerometer = false;
    m_registeredGyroscope = false;
  }
};
