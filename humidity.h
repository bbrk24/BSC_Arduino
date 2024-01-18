#pragma once

#include <Adafruit_SHT4x.h>

class HumiditySensor {
private:
  Adafruit_SHT4x m_sensor;
  TwoWire* m_i2c;
  unsigned long m_lastHeated;
  float m_lastTemp;
  float m_lastHumid;
  bool m_begun;

  bool safeToHeat() const noexcept {
    // According to the datasheet, the heater must only be run at temperatures below 65C and at 10% duty cycle.
    return m_lastTemp < 65.0F && millis() - m_lastHeated > 900;
  }

  sht4x_heater_t recommendedHeatLevel() const noexcept {
    // If it isn't safe to turn on the heater, don't.
    if (!safeToHeat()) {
      return SHT4X_NO_HEATER;
    }
    // The sensor gives most accurate values above 5C
    if (m_lastTemp < 5.0F) {
      return SHT4X_HIGH_HEATER_100MS;
    }
    // If it's too humid, the heater helps prevent drift
    if (m_lastHumid > 80.0F) {
      return SHT4X_LOW_HEATER_100MS;
    }
    // By default, don't run the heater, as it makes measurements slower
    return SHT4X_NO_HEATER;
  }
public:
  /**
   * @param[in] theI2C Pointer to a TwoWire instance representing the I2C interface. Must be valid
   * for the whole lifetime of the IMU instance.
   */
  HumiditySensor(TwoWire* theI2C) :
    m_sensor(),
    m_i2c(theI2C),
    m_lastHeated(0),
    m_lastTemp(NAN),
    m_lastHumid(NAN),
    m_begun(false) {}

  enum Status {
    /** I2C communications have not been established. */
    NOT_CONNECTED,
    /** The sensor is ready to be read. */
    ACTIVE,
  };

  /** Check the sensor status. */
  Status getStatus() const noexcept {
    if (!m_begun) {
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

        // get initial values for m_lastHumid and m_lastTemp with no heating
        m_sensor.setHeater(SHT4X_NO_HEATER);
        sensors_event_t humidEvent, tempEvent;
        m_sensor.getEvent(&humidEvent, &tempEvent);
        m_lastHumid = humidEvent.relative_humidity;
        m_lastTemp = tempEvent.temperature;
      }
    }
  }

  /**
   * Get the humidity and/or temperature values.
   * @param[out] humidity Pointer to the humidity. Pass `nullptr` to only get temperature.
   * @param[out] temperature Pointer to the temperature. Pass `nullptr` to only get humidity.
   * @return A boolean indicating whether the values were successfully read.
   */
  bool getValues(
    volatile float* humidity,
    volatile float* temperature
  ) {
    if (humidity == nullptr && temperature == nullptr) {
      // there's nothing to try
      return true;
    }

    sht4x_heater_t heater = recommendedHeatLevel();
    m_sensor.setHeater(heater);

    bool success;

    if (temperature == nullptr) {
      sensors_event_t humidEvent;
      success = m_sensor.getEvent(&humidEvent, nullptr);
      if (success) {
        *humidity = m_lastHumid = humidEvent.relative_humidity;
      }
    } else if (humidity == nullptr) {
      sensors_event_t tempEvent;
      success = m_sensor.getEvent(nullptr, &tempEvent);
      if (success) {
        *temperature = m_lastTemp = tempEvent.temperature;
      }
    } else {
      sensors_event_t humidEvent, tempEvent;
      success = m_sensor.getEvent(&humidEvent, &tempEvent);
      if (success) {
        *humidity = m_lastHumid = humidEvent.relative_humidity;
        *temperature = m_lastTemp = tempEvent.temperature;
      }
    }

    if (heater != SHT4X_NO_HEATER) {
      m_lastHeated = millis();
    }

    return success;
  }
};
