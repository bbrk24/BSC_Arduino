#include <SD.h>
#include <SPI.h>

#include "gps.h"
#include "imu.h"

#pragma once

#ifndef CAPSULE
#pragma GCC error "CAPSULE is not defined. Please define it to either 1 or 2."
#elif CAPSULE == 1
#pragma message "Compiling for capsule 1 (camera)..."
#elif CAPSULE == 2
#pragma message "Compiling for capsule 2 (atmospheric sensing)..."
#else
#pragma GCC error "Unexpected value for CAPSULE (expected 1 or 2)"
#endif

class SDCard {
  private:
    File m_sdCardFile;
    char m_fileName[13]; // 12 characters + null terminator
    bool m_begun; // SPI communications have been established
    bool m_proven; // The self-test passed

    static const int M_CHIP_SELECT = SDCARD_SS_PIN;
    static constexpr const char* M_FILE_NAME = "CAPS_INF";
    static constexpr const char* M_FILE_EXT = ".CSV";

    static constexpr const char* M_HEADERS =
      "Latitude,Longitude,Altitude (MSL),Satellites,Timestamp,Accel X,Accel Y,Accel Z,Altitude (AGL),"
      // Fun C fact: if you have multiple string literals with nothing but comments and whitespace
      // between them, the compiler treats them as one long string literal
#if CAPSULE == 2
      "VOC Reading,Humidity,Temperature,"
#endif
      "Gyro X,Gyro Y,Gyro Z";

    // Finds an available filename and sets m_fileName accordingly.
    void findFileName() {
      // First, check if the base file name is available
      strcpy(m_fileName, M_FILE_NAME);
      strcat(m_fileName, M_FILE_EXT);
      if (!SD.exists(m_fileName)) {
        return;
      }

      // Then, try it with numbers
      const size_t BASENAME_LENGTH = strlen(M_FILE_NAME);
      for (int i = 0; i <= 99999999; ++i) {
        // Convert the number to a C-style string (= char[])
        char buf[9];
        sprintf(buf, "%d", i);
        // Overwrite just the end of the filename with the number
        // e.g. CAPS_INF + 13 => CAPS_I13
        size_t bufLen = strlen(buf);
        memcpy(m_fileName + BASENAME_LENGTH - bufLen, buf, bufLen);
        if (!SD.exists(m_fileName)) {
          return;
        }
      }

      // We're out of filenames, somehow...
      // Do we really have a hundred million files?
      // Whatever, reset to the default and delete the file
      strcpy(m_fileName, M_FILE_NAME);
      strcat(m_fileName, M_FILE_EXT);
      SD.remove(m_fileName);
    }

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
        m_sdCardFile = SD.open(m_fileName, FILE_WRITE);
      }

      return !areDifferent;
    }
  public:
    SDCard() : m_sdCardFile(), m_fileName{0}, m_begun(false), m_proven(false) {}

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
          char dataOutputString[126];
          // 126 *should* be long enough for any valid data.
          // However, unlike the radio packet, this isn't a fixed length.
          // So, in case there's invalid data or I miscounted, use snprintf, which cuts off the output
          // instead of corrupting data if the string is too long.
          snprintf(
            dataOutputString,
            sizeof dataOutputString,
            "%d,%d,%.5g,%hu,%u:%u:%u:%u,%.2f,%.2f,%.2f,%.2f,"
#if CAPSULE == 2
            "%d,%.2f,%.1f,"
#endif
            "%.3f,%.3f,%.3f",
            coords.latitude,
            coords.longitude,
            (double)coords.altitudeMSL,
            (unsigned short)coords.numSatellites,
            coords.timestamp.hours, coords.timestamp.minutes, coords.timestamp.seconds, coords.timestamp.milliseconds,
            (double)accel.x,
            (double)accel.y,
            (double)accel.z,
            (double)altitude,
#if CAPSULE == 2
            analogReading,
            (double)humidity,
            (double)temperature,
#endif
            (double)gyro.x,
            (double)gyro.y,
            (double)gyro.z
          );

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
        findFileName();
        m_sdCardFile = SD.open(m_fileName, FILE_WRITE);
        if (m_sdCardFile) {
          m_sdCardFile.println(M_HEADERS);
        }
      }
    }

    void writeHeaders(){
      m_sdCardFile.println(M_HEADERS);
    }

    void closeFile() {
      m_sdCardFile.close();
    }

    /** Make sure data is saved to the SD card, then immediately re-open the file. */
    void closeAndReopen() {
      if (getStatus() != Status::ACTIVE) {
        // The file was never opened in the first place. Make sure it's set up.
        initialize();
        return;
      }

      m_sdCardFile.close();
      m_sdCardFile = SD.open(m_fileName, FILE_WRITE);
    }
};
