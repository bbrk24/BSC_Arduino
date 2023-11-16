#include <SD.h>
#include <SPI.h>

#include "gps.h"
#include "imu.h"

#pragma once

class SDCard {
  private:
    File m_sdCardDataFile;
    static const int m_chipSelect = 4; // this is not the final value - SDCARD_SS_PIN is the correct one for the MKR Zero
    static const String m_fileName = "rocketry_data";
  public:
    void writeToCSV( // this is equivalent 
      const GPS::Coordinates& coords,
      const IMU::vector3& accel,
      const IMU::vector3& gyro,
      float altitude,
      int analogReading, // this is VOC
      float humidity,
      float temperature) {
        // checks if SD card is open and good to be written to 
        if (m_sdCardDataFile) {
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
          
          dataOutputString += String(gyro.x);
          dataOutputString += ",";
          dataOutputString += String(gyro.y);
          dataOutputString += ",";
          dataOutputString += String(gyro.z);
          dataOutputString += ",";

          dataOutputString += String(altitude);

          dataOutputString += ",";

          dataOutputString += String(analogReading);

          dataOutputString += ",";

          dataOutputString += String(humidity);

          dataOutputString += ",";

          dataOutputString += String(temperature);

          m_sdCardDataFile.println(dataOutputString);
        }
      }

    void sd_initialize() { // this is equivalent to setup()
      if (!SD.begin(m_chipSelect)) {
        Serial.println("SD card not present!!");

        while (1);
      }

      Serial.println("SD Card detected and initialized");

      m_sdCardDataFile = SD.open(m_fileName + ".csv", FILE_WRITE);
    }

    void sd_closeFile() {
      m_sdCardDataFile.close();
    }
};
