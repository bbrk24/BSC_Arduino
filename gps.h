#pragma once

#ifndef kUBLOXGNSSDefaultMaxWait
#define kUBLOXGNSSDefaultMaxWait 250
#endif

#include <SparkFun_u-blox_GNSS_v3.h>

class GPS {
private:
  SFE_UBLOX_GNSS m_gnss;
  TwoWire* m_i2c;
  // 0 = no I2C connection
  // 1... = various stages of GNSS configuration
  uint8_t m_phase; //Keeps track of initialization phase
public:
  /**
   * @param[in] theI2C Pointer to a TwoWire instance representing the I2C interface. Must be valid
   * for the whole lifetime of the IMU instance.
   */
  GPS(TwoWire* theI2C) :
    m_gnss(),
    m_i2c(theI2C),
    m_phase(0) {
      // *2 to make the hardware timeout longer than the software timeout
      m_i2c->setTimeout(kUBLOXGNSSDefaultMaxWait * 2);
  }

  enum Status {
    /** Communications have not been established. */
    NOT_CONNECTED,
    /** Communications are established, but the GNSS constellation may not be fully configured. */
    NOT_CONFIGURED,
    /** The sensor is ready to be read. */
    ACTIVE,
  };

  /** Check the GNSS status. */
  Status getStatus() const noexcept {
    if (m_phase == 0) {
      return Status::NOT_CONNECTED;
    } else if (m_phase <= 6) {
      return Status::NOT_CONFIGURED;
    } else {
      return Status::ACTIVE;
    }
  }
  // This goes through a series of steps to initialize the GPS Module. Once 6 is completed, initialization is done
  // WARNING: This may hang if connections cannot be established! Use it with care
  void initialize() {

#define ENABLE_PHASE(gnss, enable) \
  if (!m_gnss.enableGNSS(enable, gnss)) { \
    break; \
  } \
  ++m_phase;

    switch (m_phase) {
    case 0:
      if (!m_gnss.begin(*m_i2c)) {
        break;
      }
      m_phase = 1;
    case 1:
      // Possibly not necessary, but recommended by the documentation to do just in case
      ENABLE_PHASE(SFE_UBLOX_GNSS_ID_GPS, true);
    case 2:
      // Disabled in a specific order so that all intermediate configurations are valid for both
      // SAM-M10Q and ZOE-M8Q (we've been going back and forth on the two)
      ENABLE_PHASE(SFE_UBLOX_GNSS_ID_SBAS, false);
    case 3:
      ENABLE_PHASE(SFE_UBLOX_GNSS_ID_BEIDOU, false);
    case 4:
      ENABLE_PHASE(SFE_UBLOX_GNSS_ID_QZSS, false);
    case 5:
      ENABLE_PHASE(SFE_UBLOX_GNSS_ID_GLONASS, false);
    case 6:
      ENABLE_PHASE(SFE_UBLOX_GNSS_ID_GALILEO, false);
    default:
      break;
    }

#undef ENABLE_PHASE
  }

  using Timestamp = struct {
    unsigned int milliseconds : 10;
    unsigned int seconds : 6;
    unsigned int minutes : 6;
    unsigned int hours : 5;
    // Storage layout:
    // 31              23              15               7             0
    // +- - - - - - - -+- - - - - - - -+- - - - - - - -+- - - - - - - -+
    // | (zero)  |  hours  |  minutes  |  seconds  |   milliseconds    |
    // +- - - - - - - -+- - - - - - - -+- - - - - - - -+- - - - - - - -+
  };

  using Coordinates = struct {
    /** Longitude, in degrees */
    double longitude;
    /** Latitude, in degrees */
    double latitude;
    /** Altitude, in feet above mean sea level (not AGL!) */
    float altitudeMSL;
    /** Number of visible satellites */
    uint8_t numSatellites;

    Timestamp timestamp;
  };

  /**
   * Get the location and time data from the GPS.
   * @param[out] loc Pointer to location data. Must not be nullptr.
   * @return A boolean indicating whether the values were successfully read.
   */
  bool getLocation(Coordinates* loc) {
    bool success = m_gnss.getPVT();
    if (success) {
      loc->longitude = 1.0e-7 * (double)m_gnss.getLongitude();
      loc->latitude = 1.0e-7 * (double)m_gnss.getLatitude();

      const float FEET_PER_MILLIMETER = 0.0032808399F;
      loc->altitudeMSL = FEET_PER_MILLIMETER * (float)m_gnss.getAltitudeMSL();

      loc->numSatellites = m_gnss.getSIV();

      loc->timestamp.milliseconds = m_gnss.getMillisecond();
      loc->timestamp.seconds = m_gnss.getSecond();
      loc->timestamp.minutes = m_gnss.getMinute();
      loc->timestamp.hours = m_gnss.getHour();
    }
    return success;
  }

  static uint32_t getTotalMS(Timestamp t) noexcept {
    return ((t.hours * 60 + t.minutes) * 60 + t.seconds) * 1000 + t.milliseconds;
  }
};
