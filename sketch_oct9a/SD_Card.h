#include <SD.h>
#include <SPI.h>

#include "gps.h"
#include "imu.h"

#define CAPSULE 2

#pragma once

class SDCard {
  private:
    File M_SDCARD_DATA_FILE;
    static const int M_CHIP_SELECT = 4; // this is not the final value - SDCARD_SS_PIN is the correct one for the MKR Zero
    static const String M_FILE_NAME = "rocketry_data";
  public:
    void writeToCSV( // this is equivalent to loop()
      const GPS::Coordinates& coords,
      const IMU::vector3& accel,
      const IMU::vector3& gyro,
      float altitude,
      int analogReading, // this is VOC
      float humidity,
      float temperature) {
        // checks if SD card is open and good to be written to 
        if (M_SDCARD_DATA_FILE) {
          String dataOutputString = "";

          dataOutputString += String(coords.latitude);
          dataOutputString += ",";
          dataOutputString += String(coords.longitude);
          dataOutputString += ",";
          dataOutputString += String(coords.altitudeMSL);
          dataOutputString += ",";
          dataOutputString += String(coords.numSatellites);
          dataOutputString += ",";
          dataOutputString += String(coords.timestamp.hours) + " : " + String(coords.timestamp.minutes) + " : " + String(coords.timestamp.seconds) + " : " + String(coords.timestamp.milliseconds);
          dataOutputString += ",";

          dataOutputString += String(accel.x);
          dataOutputString += ",";
          dataOutputString += String(accel.y);
          dataOutputString += ",";
          dataOutputString += String(accel.z);
          dataOutputString += ",";

          dataOutputString += String(altitude);

          dataOutputString += ",";

          #if CAPSULE == 2 {
            dataOutputString += String(analogReading);

            dataOutputString += ",";

            dataOutputString += String(humidity);

            dataOutputString += ",";

            dataOutputString += String(temperature);

            dataOutputString += ",";
          }

          #endif

          dataOutputString += String(gyro.x);
          dataOutputString += ",";
          dataOutputString += String(gyro.y);
          dataOutputString += ",";
          dataOutputString += String(gyro.z);

          M_SDCARD_DATA_FILE.println(dataOutputString);
        }
      }

    void initialize() { // this is equivalent to setup()

      Serial.println("SD Card detected and initialized");

      m_sdCardDataFile = SD.open(m_fileName + ".csv", FILE_WRITE);
    }

    void closeFile() {
      M_SDCARD_DATA_FILE.close();
    }
};
