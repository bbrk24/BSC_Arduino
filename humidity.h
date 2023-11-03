#pragma once

#include <Adafruit_SHT4x.h>

class HumiditySensor {
private:
  Adafruit_SHT4x m_sensor;
  TwoWire* m_i2c;
  bool m_begun;
public:
  /**
   * @param[in] theI2C Pointer to a TwoWire instance representing the I2C interface. Must be valid
   * for the whole lifetime of the IMU instance.
   */
  HumiditySensor(TwoWire* theI2C) :
    m_sensor(),
    m_i2c(theI2C),
    m_begun(false) {}

  enum Status {
    /** I2C communications have not been established. */
    NOT_CONNECTED,
    /** The sensor is ready to be read. */
    ACTIVE,
  };

  /** Check the sensor status. */
  Status getStatus() const noexcept {
    if (m_begun) {
      return Status::NOT_CONNECTED;
    } else {
      return Status::ACTIVE;
    }
  }

  /** Initialize communications with the sensor. */
  void initialize() {
    if (!m_begun) {
      m_begun = m_sensor.begin(m_i2c);

      if (m_begun) {
        // Even at 'low' precision, the sensor accuracy is worse than the specified precision.
        m_sensor.setPrecision(SHT4X_LOW_PRECISION);
      }
    }
  }

  /**
   * Get the humidity and/or temperature values.
   * @param[out] humidity Pointer to the humidity. Pass `nullptr` to only get temperature.
   * @param[out] temperature Pointer to the temperature. Pass `nullptr` to only get humidity.
   * @param heater What heating level to use. Defaults to no heat.
   * @return A boolean indicating whether the values were successfully read.
   */
  bool getValues(
    float* humidity,
    float* temperature,
    sht4x_heater_t heater = SHT4X_NO_HEATER
  ) {
    m_sensor.setHeater(heater);

    if (temperature == nullptr) {
      if (humidity == nullptr) {
        return true;
      }

      sensors_event_t humidEvent;
      bool success = m_sensor.getEvent(&humidEvent, nullptr);
      *humidity = humidEvent.relative_humidity;
      return success;
    } else if (humidity == nullptr) {
      sensors_event_t tempEvent;
      bool success = m_sensor.getEvent(nullptr, &tempEvent);
      *temperature = tempEvent.temperature;
      return success;
    } else {
      sensors_event_t humidEvent, tempEvent;
      bool success = m_sensor.getEvent(&humidEvent, &tempEvent);
      *humidity = humidEvent.relative_humidity;
      *temperature = tempEvent.temperature;
      return success;
    }
  }
};
