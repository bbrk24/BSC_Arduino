#include <SD.h>
#include <SPI.h>

#include "gps.h"
#include "imu.h"

#pragma once

class SDCard {
  private:
    File m_sdCardFile;
    bool m_begun; // SPI communications have been established
    bool m_proven; // The self-test passed

    static const int M_CHIP_SELECT = SDCARD_SS_PIN;
    static constexpr const char* M_FILE_NAME = "CAPS_INF.CSV";

    static constexpr const char* M_HEADERS =
      "Latitude,Longitude,Altitude (MSL),Satellites,Timestamp,Accel X,Accel Y,Accel Z,Altitude (AGL),"
      // Fun C fact: if you have multiple string literals with nothing but comments and whitespace
      // between them, the compiler treats them as one long string literal
#if CAPSULE == 2
      "VOC Reading,Humidity,Temperature,"
#endif
      "Gyro X,Gyro Y,Gyro Z";

    // Write some random data to a file and see if we can read it back
    bool selfTest() {
      if (!m_begun) {
        return false;
      }

      // Close the file for now to open a new one
      bool open = (bool)m_sdCardFile;
      if (open) {
        m_sdCardFile.close();
      }

      // Perform the test:
      // 1. Generate 64 random bytes
      // 2. Write those bytes to a file
      // 3. Close and reopen the file
      // 4. Compare the file contents to the original buffer

      // 1.
      // Seed RNG as suggested by the documentation
      randomSeed(analogRead(0));

      const size_t RANDOM_BUFFER_SIZE = 64;
      char data[RANDOM_BUFFER_SIZE];
      for (int i = 0; i < RANDOM_BUFFER_SIZE; ++i) {
        data[i] = random(256);
      }

      // 2.
      // FILE_WRITE appends to the file if it exists, rather than overwriting it, so delete the file
      // before writing to it
      const char* SELFTEST_FILE_NAME = "selftest";
      if (SD.exists(SELFTEST_FILE_NAME)) {
        SD.remove(SELFTEST_FILE_NAME);
      }

      File randomFile = SD.open(SELFTEST_FILE_NAME, FILE_WRITE);
      randomFile.write(data, RANDOM_BUFFER_SIZE);

      // 3.
      randomFile.close();
      randomFile = SD.open(SELFTEST_FILE_NAME, FILE_READ);

      // 4.
      // Initialize readBack to all zeroes so that we don't get garbage data if the read failed
      char readBack[RANDOM_BUFFER_SIZE] = {0};
      randomFile.read(readBack, RANDOM_BUFFER_SIZE);
      bool areDifferent = memcmp(data, readBack, RANDOM_BUFFER_SIZE);

      // Cleanup
      // Close the random file again
      randomFile.close();
      // If the data file was open before, re-open it
      if (open) {
        m_sdCardFile = SD.open(M_FILE_NAME, FILE_WRITE);
      }

      return !areDifferent;
    }
  public:
    SDCard() : m_sdCardFile(), m_begun(false), m_proven(false) {}

    enum Status {
      /** SPI communications have not yet been established. */
      NOT_CONNECTED,
      /** The SD card is connected, but the file is not open. */
      FILE_CLOSED,
      /** The SD card is connected, but the self-test has not run successfully. */
      UNTESTED,
      /** The SD card is ready to write to. */
      ACTIVE,
    };

    Status getStatus() {
      if (!m_begun) {
        return Status::NOT_CONNECTED;
      }

      if (!m_proven) {
        return Status::UNTESTED;
      } else if (!m_sdCardFile) {
        return Status::FILE_CLOSED;
      }

      return Status::ACTIVE;
    }

    void writeToCSV( // this is equivalent to loop()
      const volatile GPS::Coordinates& coords,
      const volatile IMU::vector3& accel,
      float altitude,
#if CAPSULE == 2
      int analogReading, // this is VOC
      float humidity,
      float temperature,
#endif
      const volatile IMU::vector3& gyro) {
        // checks if SD card is open and good to be written to 
        if (m_sdCardFile) {
          String dataOutputString = "";

          dataOutputString += String(coords.latitude);
          dataOutputString += ",";
          dataOutputString += String(coords.longitude);
          dataOutputString += ",";
          dataOutputString += String(coords.altitudeMSL);
          dataOutputString += ",";
          dataOutputString += String(coords.numSatellites);
          dataOutputString += ",";
          dataOutputString += String(coords.timestamp.hours) + ":" + String(coords.timestamp.minutes) + ":" + String(coords.timestamp.seconds) + ":" + String(coords.timestamp.milliseconds);
          dataOutputString += ",";

          dataOutputString += String(accel.x);
          dataOutputString += ",";
          dataOutputString += String(accel.y);
          dataOutputString += ",";
          dataOutputString += String(accel.z);
          dataOutputString += ",";

          dataOutputString += String(altitude);

          dataOutputString += ",";

#if CAPSULE == 2
          dataOutputString += String(analogReading);

          dataOutputString += ",";

          dataOutputString += String(humidity);

          dataOutputString += ",";

          dataOutputString += String(temperature);

          dataOutputString += ",";
#endif
          dataOutputString += String(gyro.x);
          dataOutputString += ",";
          dataOutputString += String(gyro.y);
          dataOutputString += ",";
          dataOutputString += String(gyro.z);

          m_sdCardFile.println(dataOutputString);
        }
      }

    void initialize() { // this is equivalent to setup()
      if (!m_begun) {
        m_begun = SD.begin(M_CHIP_SELECT);
      }

      if (m_begun && !m_proven) {
        m_proven = selfTest();
      }

      if (m_proven && !m_sdCardFile) {
        bool alreadyExists = SD.exists(M_FILE_NAME);
        m_sdCardFile = SD.open(M_FILE_NAME, FILE_WRITE);
        // If the file is being created by this action, we need to create headers
        // If the file already exists, assume it already has headers.
        if ((bool)m_sdCardFile && !alreadyExists) {
          m_sdCardFile.println(M_HEADERS);
        }
      }
    }

    void closeFile() {
      m_sdCardFile.close();
    }
};
