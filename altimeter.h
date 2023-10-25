#pragma once

#include <Adafruit_BMP3XX.h>

namespace altimeter {

Adafruit_BMP3XX _baro;
bool _begun = false, _reasonable = false;

float _seaLevel;

enum status {
  /** I2C communications have not been established. */
  NOT_CONNECTED,
  /** Communications have been established, but the initial pressure reading was unreasonable. */
  UNREASONABLE,
  /** The sensor is ready to be read. */
  ACTIVE,
};

/**
 * Check the sensor status.
 */
status getStatus() noexcept {
  if (!_begun) {
    return status::NOT_CONNECTED;
  }
  if (!_reasonable) {
    return status::UNREASONABLE;
  }
  return status::ACTIVE;
}

/**
 * Initialize communications with the altimeter.
 */
void initialize() {
  if (!_begun) {
    _begun = _baro.begin_I2C();
  }
  if (_begun && !_reasonable) {
    _seaLevel = _baro.readPressure() / 100.0F;
    _reasonable = _seaLevel >= 929.0F && _seaLevel <= 1041.0F;
  }
}

/**
 * Get the current altitude. Blocks until a value is available.
 * @return AGL in feet.
 */
float getAltitude() {
  float meters = _baro.readAltitude(_seaLevel);
  const float FEET_PER_METER = 3.28084F;
  return meters * FEET_PER_METER;
}

} // namespace altimeter
