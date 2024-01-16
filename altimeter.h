#pragma once

#include <Adafruit_BMP3XX.h>

class Altimeter {
private:
  Adafruit_BMP3XX m_baro;
  bool m_begun;
  bool m_reasonable;
  float m_seaLevel;
public:
  Altimeter() : m_baro(), m_begun(false), m_reasonable(false), m_seaLevel(NAN) {}

  enum Status {
    /** I2C communications have not been established. */
    NOT_CONNECTED,
    /** Communications have been established, but the initial pressure reading was unreasonable. */
    UNREASONABLE,
    /** The sensor is ready to be read. */
    ACTIVE,
  };

  /** Check the sensor status. */
  Status getStatus() const noexcept {
    if (!m_begun) {
      return Status::NOT_CONNECTED;
    }
    if (!m_reasonable) {
      return Status::UNREASONABLE;
    }
    return Status::ACTIVE;
  }

  /**
   * Initialize communications with the altimeter.
   * @param[in] theWire Pointer to TwoWire object representing the I2C interface. Must be valid for
   * the lifetime of the Altimeter instance.
   */
  void initialize(TwoWire* theWire = &Wire) {
    if (!m_begun) {
      m_begun = m_baro.begin_I2C(BMP3XX_DEFAULT_ADDRESS, theWire);
    }
    if (m_begun && !m_reasonable) {
      m_seaLevel = m_baro.readPressure() / 100.0F;
      m_reasonable = m_seaLevel >= 929.0F && m_seaLevel <= 1041.0F;
    }
  }

  void setSeaLevel(float currSeaLevel) {
    m_seaLevel = currSeaLevel;
    m_reasonable = true;
  }

  /**
   * Get the current altitude. Blocks until a value is available.
   * @return AGL in feet.
   */
  float getAltitude() {
    float meters = m_baro.readAltitude(m_seaLevel);
    const float FEET_PER_METER = 3.28084F;
    return meters * FEET_PER_METER;
  }
};
