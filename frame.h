#pragma once

#include "imu.h"
#include "gps.h"

#ifndef CAPSULE
#pragma GCC error "CAPSULE is not defined. Please define it to either 1 or 2."
#elif CAPSULE == 1
#pragma message "Compiling for capsule 1 (camera)..."
#elif CAPSULE == 2
#pragma message "Compiling for capsule 2 (atmospheric sensing)..."
#else
#pragma GCC error "Unexpected value for CAPSULE (expected 1 or 2)"
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wpadded"

struct Frame {
  /** 4 bytes: Latitude, in 10^-7 degrees */
  int32_t latitude;
  /** 4 bytes: Longitude, in 10^-7 degrees */
  int32_t longitude;
  /** 4 bytes: Milliseconds since midnight */
  uint32_t timestamp;
  /** 1 byte: Number of GPS satellites visible */
  uint8_t numSat;

  /** 1 byte: Gyroscope X in deg/s */
  int8_t pitch;
  /** 1 byte: Gyroscope Y in deg/s */
  int8_t roll;
  /** 1 byte: Gyroscope Z in deg/s */
  int8_t yaw;
  /** 2 bytes: Acceleration X in cm/s^2 */
  int16_t xAccel;
  /** 2 bytes: Acceleration Y in cm/s^2 */
  int16_t yAccel;
  /** 2 bytes: Acceleration Z in cm/s^2 */
  int16_t zAccel;

  /** 2 bytes: Altitude in feet AGL */
  int16_t altitude;

#if CAPSULE == 2
  /** 2 bytes: Raw readout (0-1023) from VOC sensor */
  uint16_t vocReading;

  /** 2 bytes: Temperature in tenths of a degree Celsius */
  int16_t temp;
  /** 1 byte: Relative humidity */
  uint8_t humidity;

  /**
   * The pragmas above are to prevent any hidden padding inserting itself between fields. However,
   * GCC wants to align this struct to a multiple of four bytes (the size of its largest field). In
   * order to facilitate this, I am adding an explicit member "padding" of three bytes. These three
   * bytes will not be sent over the air.
   */
  int8_t padding[3];
#endif
};

#pragma GCC diagnostic pop

constexpr size_t FRAME_SIZE() {
  Frame f{0};
  return sizeof f 
#if CAPSULE == 2
    - sizeof f.padding
#endif
  ;
}

int8_t _radiansToCappedDegrees(float rad) {
  float deg = degrees(rad);
  if (abs(deg) > 127.0F) {
    return -128;
  } else {
    return (int8_t)deg;
  }
}

Frame createFrame(
  const GPS::Coordinates& coords,
  const IMU::vector3& accel,
  const IMU::vector3& gyro,
  float altitude,
  int analogReading,
  float humidity,
  float temperature
) noexcept {
  // Weird C syntax that lets you label each argument to the struct initialization
  return (Frame){
    .latitude = (int32_t)(coords.latitude * 10e+7),
    .longitude = (int32_t)(coords.longitude * 10e+7),
    .timestamp = GPS::getTotalMS(coords.timestamp),
    .numSat = coords.numSatellites,
    .pitch = _radiansToCappedDegrees(gyro.x),
    .roll = _radiansToCappedDegrees(gyro.y),
    .yaw = _radiansToCappedDegrees(gyro.z),
    .xAccel = (int16_t)(accel.x * 100.0F),
    .yAccel = (int16_t)(accel.y * 100.0F),
    .zAccel = (int16_t)(accel.z * 100.0F),
    .altitude = (int16_t)altitude,
#if CAPSULE == 2
    .vocReading = analogReading,
    .temp = (int16_t)(temperature * 10.0F),
    .humidity = (uint8_t)humidity,
    .padding = { 0, 0, 0 }
#endif
  };
}
