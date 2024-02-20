#pragma once

#include <MicroNMEA.h>

class GPS {
private:
  Uart& m_bus;
  // Maximum valid NMEA string is 82 chars, add 1 for null terminator
  char m_buffer[83];
  MicroNMEA m_nmea;
  bool m_begun;
  bool m_valid;
public:
  GPS(Uart& bus) :
    m_bus(bus),
    m_buffer{0},
    m_nmea(m_buffer, sizeof m_buffer),
    m_begun(false),
    m_valid(false) {}

  union Timestamp {
    struct {
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
    uint32_t rawValue;

    // Explicitly default parameterless constructor and copy and move operators
    // The presence of other constructors and operator=s below would mean these don't exist
    Timestamp() = default;
    Timestamp(const Timestamp&) = default;
    Timestamp(Timestamp&&) = default;
    Timestamp& operator=(const Timestamp&) = default;
    Timestamp& operator=(Timestamp&&) = default;

    // Some things to make working with volatile Timestamps easier
    Timestamp(const volatile Timestamp& other) noexcept : rawValue(other.rawValue) {}
    Timestamp(volatile Timestamp&& other) noexcept : rawValue(other.rawValue) {}

    volatile Timestamp& operator=(const Timestamp& other) volatile noexcept {
      this->rawValue = other.rawValue;
      return *this;
    }
    volatile Timestamp& operator=(Timestamp&& other) volatile noexcept {
      this->rawValue = other.rawValue;
      return *this;
    }
  };

  struct Coordinates {
    /** Longitude, in 10^-7 degrees */
    int32_t longitude;
    /** Latitude, in 10^-7 degrees */
    int32_t latitude;
    /** Altitude, in feet above mean sea level (not AGL!) */
    float altitudeMSL;
    /** Number of visible satellites */
    uint8_t numSatellites;

    Timestamp timestamp;

    // Same idea as with Timestamp above
    Coordinates() = default;
    Coordinates(const Coordinates&) = default;
    Coordinates(Coordinates&&) = default;
    Coordinates& operator=(const Coordinates&) = default;
    Coordinates& operator=(Coordinates&&) = default;

    volatile Coordinates& operator=(const Coordinates& other) volatile noexcept {
      this->longitude = other.longitude;
      this->latitude = other.latitude;
      this->altitudeMSL = other.altitudeMSL;
      this->numSatellites = other.numSatellites;
      this->timestamp = other.timestamp;
      return *this;
    }
    volatile Coordinates& operator=(Coordinates&& other) volatile noexcept {
      this->longitude = other.longitude;
      this->latitude = other.latitude;
      this->altitudeMSL = other.altitudeMSL;
      this->numSatellites = other.numSatellites;
      this->timestamp = other.timestamp;
      return *this;
    }
  };
  
  void initialize() {
    if (!m_begun) {
      m_bus.begin(9600);
      m_begun = true;
    }
  }

  enum Status {
    ACTIVE,
    INVALID
  };

  Status getStatus() {
    if (!m_valid) {
      // try to get data off the GPS, because we haven't been successful yet
      Coordinates coords;
      getLocation(&coords);
    }
    return m_valid ? Status::ACTIVE : Status::INVALID;
  }

  bool getLocation(volatile Coordinates* coords) {
    while (m_bus.available()) {
      char c = m_bus.read();
      if (m_nmea.process(c)) {
        if (!m_nmea.isValid()) {
          return false;
        }

        m_valid = true;

        // MicroNMEA gives coords in 10^-6, not 10^-7, so multiply by 10
        coords->latitude = m_nmea.getLatitude() * 10;
        coords->longitude = m_nmea.getLongitude() * 10;

        long altitudeMillimeters;
        if (m_nmea.getAltitude(altitudeMillimeters)) {
          const float FEET_PER_MILLIMETER = 0.0032808399F;
          coords->altitudeMSL = FEET_PER_MILLIMETER * altitudeMillimeters;
        }

        // MicroNMEA is only precise to the hundredth of a second
        coords->timestamp.milliseconds = m_nmea.getHundredths() * 10;
        coords->timestamp.seconds = m_nmea.getSecond();
        coords->timestamp.minutes = m_nmea.getMinute();
        coords->timestamp.hours = m_nmea.getHour();

        m_nmea.clear();
        return true;
      }
    }

    // No data available for read
    return false;
  }

  static uint32_t getTotalMS(Timestamp t) noexcept {
    return ((t.hours * 60 + t.minutes) * 60 + t.seconds) * 1000 + t.milliseconds;
  }
};
