#pragma once

#include <Adafruit_LSM9DS1.h>

class IMU {
private:
  Adafruit_LSM9DS1 m_sensor;
  bool m_begun;
  bool m_reasonableGravity;
public:
  union vector3 {
    float data[3];
    struct {
      float x;
      float y;
      float z;
    };

    // C++ boilerplate nonsense
    vector3() = default;
    vector3(const vector3&) = default;
    vector3(vector3&&) = default;
    vector3& operator=(const vector3&) = default;
    vector3& operator=(vector3&&) = default;

    volatile vector3& operator=(const vector3& other) volatile noexcept {
      this->x = other.x;
      this->y = other.y;
      this->z = other.z;
      return *this;
    }
    volatile vector3& operator=(vector3&& other) volatile noexcept {
      this->x = other.x;
      this->y = other.y;
      this->z = other.z;
      return *this;
    }
  };

  /**
   * @param[in] theI2C Pointer to a TwoWire instance representing the I2C interface. Must be valid
   * for the whole lifetime of the IMU instance.
   */
  IMU(TwoWire* theI2C) :
    m_sensor(theI2C),
    m_begun(false),
    m_reasonableGravity(false) {}

  enum Status {
    /** I2C communications have not been established. */
    NOT_CONNECTED,
    /** Communications have been established, but the accelerometer is reading an unusual value for gravity. */
    UNREASONABLE,
    /** The sensor is ready to be read. */
    ACTIVE,
  };

  /** Check the sensor status. */
  Status getStatus() const noexcept {
    if (!m_begun) {
      return Status::NOT_CONNECTED;
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
  bool getValues(vector3* accel, vector3* gyro) {
    if (accel == nullptr) {
      if (gyro == nullptr) {
        return true;
      }

      sensors_event_t gyroEvent;
      bool success = m_sensor.getEvent(nullptr, nullptr, &gyroEvent, nullptr);
      if (success) {
        // Because you can't copy arrays with the `=` operator, use memcpy instead
        memcpy(&(gyro->data), &gyroEvent.gyro.v, sizeof gyro->data);
      }
      return success;
    } else if (gyro == nullptr) {
      sensors_event_t accelEvent;
      bool success = m_sensor.getEvent(&accelEvent, nullptr, nullptr, nullptr);
      if (success) {
        memcpy(&(accel->data), &accelEvent.acceleration.v, sizeof accel->data);
      }
      return success;
    } else {
      sensors_event_t accelEvent, gyroEvent;
      bool success = m_sensor.getEvent(&accelEvent, nullptr, &gyroEvent, nullptr);
      if (success) {
        memcpy(&(gyro->data), &gyroEvent.gyro.v, sizeof gyro->data);
        memcpy(&(accel->data), &accelEvent.acceleration.v, sizeof accel->data);
      }
      return success;
    }
  }

  /** Get the magnitude of acceleration. */
  static float getMagnitude(const vector3& accel) noexcept {
    return sqrtf(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
  }

  /** Initialize communications with the IMU. Call this during initial setup or to wake the IMU from sleep. */
  void initialize() {
    if (!m_begun) {
      m_begun = m_sensor.begin();

      if (m_begun) {
        m_sensor.setupAccel(Adafruit_LSM9DS1::LSM9DS1_ACCELRANGE_16G);
        m_sensor.setupGyro(Adafruit_LSM9DS1::LSM9DS1_GYROSCALE_2000DPS);
      }
    }

    if (m_begun && !m_reasonableGravity) {
      vector3 accel;
      while (!getValues(&accel, nullptr)) {
        yield();
      }

      float magnitude = getMagnitude(accel);
      m_reasonableGravity = (8.93F <= magnitude && magnitude <= 10.69F);
    }
  }
};
