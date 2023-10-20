#pragma once

#include <Adafruit_MPL3115A2.h>

namespace altimeter {

Adafruit_MPL3115A2 _baro;
bool _begun = false, _reasonable = false;

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
    _begun = _baro.begin();
  }
  if (_begun && !_reasonable) {
    float basePressure = _baro.getPressure();
    _reasonable = basePressure >= 929.0F && basePressure <= 1041.0F;
    if (_reasonable) {
      _baro.setSeaPressure(basePressure);
    }
  }
}

/**
 * Get the current altitude. Blocks until a value is available.
 * @return AGL in feet.
 */
float getAltitude() {
  float meters = _baro.getAltitude();
  const float FEET_PER_METER = 3.28084F;
  return meters * FEET_PER_METER;
}

} // namespace altimeter
