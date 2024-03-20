#if true

#include "wiring_private.h"
#include "Scheduler.h"

// 1 for camera, 2 for atmospheric sensors
#define CAPSULE 2

#include "altimeter.h"
#include "imu.h"
#include "gps.h"

#if CAPSULE == 2
#include "humidity.h"
#endif

#include "SD_Card.h"

// Baud rate for radio UART
const unsigned long RADIO_BAUD = 230400;

// How many times per second the data will be sent over radio.
// If this is more than ~18, some packets will have outdated GPS info.
// If this is more than ~200, the radio will be rate-limited by the sensors.
const unsigned long RADIO_FREQ = 144;

const unsigned long GPS_FREQ = 18;

volatile bool ledsOn = true;

/*
On the Arduino MKR series, all pins for a I2C/UART/SPI instance must be on the same "sercom" object.
Additionally, for I2C, only some pins are suitable for clock and only some pins are suitable for
data. There are six sercoms:

sercom0: 2, 3, 11 (SDA), 12 (SCL), A3, A4, A5, A6
sercom1: 8 (SDA), 9 (SCL), 10
sercom2: 2, 3, 11 (SDA), 12 (SCL), <SD>
sercom3: 0 (SDA), 1 (SCL), 6, 7, 8 (SDA), 9 (SCL), 10, <USB>
sercom4: 4, 5, <SD>
sercom5: 0 (SDA), 1 (SCL), 13, 14, A1 (SDA), A2 (SCL)

Pins not marked as "SDA" or "SCL" can only be used for SPI or UART.

Information derived from
https://raw.githubusercontent.com/arduino/ArduinoCore-samd/master/variants/mkrzero/variant.cpp
and
https://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D21_DA1_Family_DataSheet_DS40001882F.pdf

There is also some additional code that must be added, as per
https://docs.arduino.cc/tutorials/communication/SamdSercom#create-a-new-wire-instance
*/

static TwoWire altI2C(
  &sercom3,
  /*sda:*/0,
  /*scl:*/1
);

Altimeter alt;
volatile float last_alt;
float max_alt = -INFINITY;

IMU imu(&Wire);
volatile IMU::vector3 last_accel;
volatile IMU::vector3 last_gyro;

#if CAPSULE == 2
static TwoWire humidityI2C(
  &sercom1,
  /*sda:*/8,
  /*scl:*/9
);
HumiditySensor hum(&humidityI2C);
volatile float last_humid;
volatile float last_temp;

volatile int last_voc;
#endif

// Even though this is in simplex operation and doesn't even have a physical wire to write to,
// the initializer still needs a TX pin.
static Uart gpsUart(
  &sercom0,
  /*rx:*/3,
  /*tx:*/2,
  SERCOM_RX_PAD_3,
  UART_TX_PAD_2
);
Uart& radioUart = Serial1;

GPS gps(gpsUart);
volatile GPS::Coordinates last_coords;

SDCard card;

int8_t radiansToCappedDegrees(float rad) {
  float deg = degrees(rad);
  if (fabsf(deg) > 127.0F) {
    return -128;
  } else {
    return (int8_t)deg;
  }
}

extern "C" {
void SERCOM0_Handler(void) {
  gpsUart.IrqHandler();
}

#if CAPSULE == 2
void SERCOM1_Handler(void) {
  humidityI2C.onService();
}
#endif

void SERCOM3_Handler(void) {
  altI2C.onService();
}
} // extern "C"

#if CAPSULE == 2
void updateTempHumidLEDs() {
  if (!ledsOn) { return; }

  HumiditySensor::Status humidityStatus = hum.getStatus();
  bool good =  humidityStatus == HumiditySensor::ACTIVE;
  digitalWrite(6, good);
}
#endif

void updateMissionCriticalLEDs() {
  if (!ledsOn) { return; }

  GPS::Status gpsStatus = gps.getStatus();
  IMU::Status imuStatus = imu.getStatus();
  Altimeter::Status altimeterStatus = alt.getStatus();
  SDCard::Status sdCardStatus = card.getStatus();

  bool good = gpsStatus == GPS::ACTIVE
    && imuStatus == IMU::ACTIVE
    && altimeterStatus == Altimeter::ACTIVE
    && sdCardStatus == SDCard::ACTIVE;
  digitalWrite(7, good);
}

void saveDataToSD() {
  if (card.getStatus() != SDCard::ACTIVE) {
    card.initialize();
    updateMissionCriticalLEDs();
  }
  card.writeToCSV(
    last_coords,
    last_accel,
    last_alt,
#if CAPSULE == 2
    last_voc,
    last_humid,
    last_temp,
#endif
    last_gyro
  );
}

// Try to initialize all sensors. Has no effect once everything is initialized.
void initializeAll() {
#if CAPSULE == 2
  hum.initialize();
  updateTempHumidLEDs();
#endif

  alt.initialize(&altI2C);
  imu.initialize();
  card.initialize();
  gps.initialize();
  updateMissionCriticalLEDs();
}

// Convenience functions for reading sensor data
#if CAPSULE == 2
void readAtmospheric() {
  last_voc = analogRead(6);

  if (hum.getStatus() != HumiditySensor::ACTIVE) {
    hum.initialize();
    updateTempHumidLEDs();
  }

  hum.getValues(&last_humid, &last_temp);
}
#endif

void readGPS() {
  if (gps.getStatus() != GPS::ACTIVE) {
    gps.initialize();
    updateMissionCriticalLEDs();
  }

  gps.getLocation(&last_coords);
}

void readAltIMU() {
  if (imu.getStatus() != IMU::ACTIVE) {
    imu.initialize();
    updateMissionCriticalLEDs();
  }

  // This has to be done like this because imu.getValues cannot take a pointer to a volatile vector3
  IMU::vector3 accel, gyro;
  if (imu.getValues(&accel, &gyro)) {
    last_accel = accel;
    last_gyro = gyro;
  }

  if (alt.getStatus() != Altimeter::ACTIVE) {
    alt.initialize(&altI2C);
    updateMissionCriticalLEDs();
  }

  if (alt.getStatus() == Altimeter::ACTIVE) {
    float altitude = alt.getAltitude();
    last_alt = altitude;
    if (max_alt < altitude) {
      max_alt = altitude;
    }
  }
}

char sign(int n) {
  return n < 0 ? '-' : '+';
}

void sendDataToRadio() {
  char buf[79];
  
  int pitch = radiansToCappedDegrees(last_gyro.x);
  int roll = radiansToCappedDegrees(last_gyro.y);
  int yaw = radiansToCappedDegrees(last_gyro.z);

  int accelXcm = last_accel.x * 100;
  int accelYcm = last_accel.y * 100;
  int accelZcm = last_accel.z * 100;

  snprintf(
    buf,
    sizeof buf,
    "%c%08X,%c%08X,%07X,%X,%c%02X,%c%02X,%c%02X,%c%04X,%c%04X,%c%04X,%c%04X"
#if CAPSULE == 2
    ",%03X,%c%03X,%02X"
#endif
    "\n",
    sign(last_coords.latitude), abs(last_coords.latitude),
    sign(last_coords.longitude), abs(last_coords.longitude),
    GPS::getTotalMS(last_coords.timestamp),
    min(15U, (unsigned int)last_coords.numSatellites),
    sign(pitch), abs(pitch),
    sign(roll), abs(roll),
    sign(yaw), abs(yaw),
    sign(accelXcm), abs(accelXcm),
    sign(accelYcm), abs(accelYcm),
    sign(accelZcm), abs(accelZcm),
    sign(last_alt), (unsigned int)abs(last_alt)
#if CAPSULE == 2
    ,
    last_voc,
    sign(last_temp), (unsigned int)abs(last_temp),
    (unsigned int)last_humid
#endif
  );
  
  radioUart.write(buf);
}

void setup() {
  radioUart.begin(RADIO_BAUD);

#if CAPSULE == 2
  // Pin A6: analog input from VOC sensor
  pinMode(A6, PinMode::INPUT);
  // Pin D4: temp/humidity sensor LEDs
  pinMode(4, PinMode::OUTPUT);
#endif
  // Pin D7: mission-critical LEDs
  pinMode(7, PinMode::OUTPUT);

  pinPeripheral(0, PIO_SERCOM);
  pinPeripheral(1, PIO_SERCOM);
  pinPeripheral(2, PIO_SERCOM);
  pinPeripheral(3, PIO_SERCOM);
#if CAPSULE == 2
  pinPeripheral(8, PIO_SERCOM);
  pinPeripheral(9, PIO_SERCOM);
#endif

  // Turn on the error LEDs until initialization finishes
#if CAPSULE == 2
  digitalWrite(6, LOW);
#endif
  digitalWrite(7, LOW);

  // Wait for a command from the radio. Sometimes the sensors need multiple tries, and calling
  // initialize too many times does nothing, so put the initialize call in the loop.
  do {
    initializeAll();

    if (radioUart.available()) {
      //The string returned from readStringUntil does not include the terminator
      String command = radioUart.readStringUntil('\n');

      if (command == "start") {
        // disable LEDs
        ledsOn = false;
#if CAPSULE == 2
        pinMode(4, PinMode::INPUT);
#endif
        pinMode(7, PinMode::INPUT);

        break;
      } else if (command == "transmit_data") {
#if CAPSULE == 2
        readAtmospheric();
#endif
        readGPS();
        readAltIMU();
        sendDataToRadio();
        saveDataToSD();
      } else {
        // unrecognized command -- send back all zeros
#if CAPSULE == 1
        radioUart.write("+00000000,+00000000,0000000,0,+00,+00,+00,+0000,+0000,+0000,+0000\n");
#else
        radioUart.write("+00000000,+00000000,0000000,0,+00,+00,+00,+0000,+0000,+0000,+0000,000,+000,+00\n");
#endif
      }
    }
  } while (true);

  Scheduler.startLoop(gps_and_save_loop);
  Scheduler.startLoop(radio_loop);
}

void loop() {
#if CAPSULE == 2
  readAtmospheric();

  // Requesting data from the humidity sensor can be relatively slow
  yield();
#endif

  readAltIMU();

  // Leaving this yield in in capsule 2 increases the amount of time spent context switching by up to 9x!
#if CAPSULE == 1
  yield();
#endif

  if (max_alt > 2000 && last_alt < 1000) {
    if (card.getStatus() == SDCard::ACTIVE) {
      card.closeFile();
    }
  } else {
    saveDataToSD();
    yield();
  }
}

void gps_and_save_loop() {
  for (int i = 0; i < 2 * GPS_FREQ; ++i) {
    readGPS();
    delay(1000 / GPS_FREQ);
  }

  // If the file was intentionally closed, don't re-open it.
  if (card.getStatus() != SDCard::FILE_CLOSED) {
    card.closeAndReopen();
  }
}

void radio_loop() {
  delay(1000 / RADIO_FREQ);
  sendDataToRadio();
}

#endif
