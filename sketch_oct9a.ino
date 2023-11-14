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

#include "frame.h"
#include "SD_Card.h"

// How many times per second the data will be sent over radio.
// If this is more than ~18, some packets will have outdated GPS info.
// If this is more than ~200, the radio will be rate-limited by the sensors.
const unsigned long RADIO_FREQ = 1;

Altimeter alt;
volatile float last_alt;

/*
On the Arduino MKR series, all pins for a I2C/UART/SPI instance must be on the same "sercom" object.
Additionally, for I2C, only some pins are suitable for clock and only some pins are suitable for
data. There are six sercoms:

sercom0: 2, 3, 11 (SDA), 12 (SCL), A3 (SDA), A4 (SCL), A5, A6
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

static TwoWire imuI2C(
  &sercom1,
  /*sda:*/8,
  /*scl:*/9
);
IMU imu(&imuI2C);
volatile IMU::vector3 last_accel;
volatile IMU::vector3 last_gyro;

#if CAPSULE == 2
static TwoWire humidityI2C(
  &sercom0,
  /*sda:*/A3,
  /*scl:*/A4
);
HumiditySensor hum(&humidityI2C);
volatile float last_humid;
volatile float last_temp;

volatile int last_voc;
#endif

static TwoWire gpsI2C(
  &sercom3,
  /*sda:*/0,
  /*scl:*/1
);
GPS gps(&gpsI2C);
volatile GPS::Coordinates last_coords;

SDCard card;

extern "C" {
void SERCOM1_Handler(void) {
  imuI2C.onService();
}

void SERCOM0_Handler(void) {
  humidityI2C.onService();
}

void SERCOM3_Handler(void) {
  gpsI2C.onService();
}
} // extern "C"

void updateSensorLEDs() {
  IMU::Status imuStatus = imu.getStatus();
  Altimeter::Status altimeterStatus = alt.getStatus();
#if CAPSULE == 2
  HumiditySensor::Status humidityStatus = hum.getStatus();
#endif
  bool good =
    altimeterStatus == Altimeter::ACTIVE
    && imuStatus == IMU::ACTIVE
#if CAPSULE == 2
    && humidityStatus == HumiditySensor::ACTIVE
#endif
  ;
  digitalWrite(7, good);
}

void updateGPSLEDs() {
  bool good = gps.getStatus() == GPS::ACTIVE;
  digitalWrite(6, good);
}

void updateSDCardLEDs() {
  bool good = card.getStatus() == SDCard::ACTIVE;
  digitalWrite(5, good);
}

void setup() {
  // Since the RTS and CTS pins are private, I have to initialize a whole new object, rather than
  // just assigning those two pins
  Serial1 = Uart(
    // First five arguments are the same as default
    &sercom5, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX,
    // and then:
    /*rts:*/A1, /*cts:*/A2
  );
  Serial1.begin(9600);

#if CAPSULE == 2
  // Pin A6: analog input from VOC sensor
  pinMode(A6, PinMode::INPUT);
#endif
  // Pin D5: SD LEDs
  pinMode(5, PinMode::OUTPUT);
  // Pin D6: GPS LEDs
  pinMode(6, PinMode::OUTPUT);
  // Pin D7: sensor LEDs
  pinMode(7, PinMode::OUTPUT);

  pinPeripheral(0, PIO_SERCOM);
  pinPeripheral(1, PIO_SERCOM);
  pinPeripheral(8, PIO_SERCOM);
  pinPeripheral(9, PIO_SERCOM);
  pinPeripheral(A1, PIO_SERCOM);
  pinPeripheral(A2, PIO_SERCOM);
  pinPeripheral(A3, PIO_SERCOM);
  pinPeripheral(A4, PIO_SERCOM);

  // Turn on the error LEDs until initialization finishes
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  // Initialize sensors
  alt.initialize();
  imu.initialize();
#if CAPSULE == 2
  hum.initialize();
#endif
  updateSensorLEDs();

  gps.initialize();
  updateGPSLEDs();

  Scheduler.startLoop(gps_loop);
  Scheduler.startLoop(radio_loop);
}

void loop() {
#if CAPSULE == 2
  last_voc = analogRead(6);

  if (hum.getStatus() != HumiditySensor::ACTIVE) {
    hum.initialize();
    updateSensorLEDs();
  }

  float humid, temp;
  if (hum.getValues(&humid, &temp)) {
    last_humid = humid;
    last_temp = temp;
  }

  // Requesting data from the humidity sensor can be relatively slow
  yield();
#endif

  if (imu.getStatus() != IMU::ACTIVE) {
    imu.initialize();
    updateSensorLEDs();
  }

  IMU::vector3 accel, gyro;
  if (imu.getValues(&accel, &gyro)) {
    last_accel = accel;
    last_gyro = gyro;
  }

  if (alt.getStatus() != Altimeter::ACTIVE) {
    alt.initialize();
    updateSensorLEDs();
  }

  if (alt.getStatus() == Altimeter::ACTIVE) {
    last_alt = alt.getAltitude();
  }

  yield();

  // TODO: Write data to SD card
}

void gps_loop() {
  if (gps.getStatus() != GPS::ACTIVE) {
    gps.initialize();
    updateGPSLEDs();
  }

  GPS::Coordinates c;
  if (gps.getLocation(&c)) {
    last_coords = c;
  }

  const unsigned long GPS_FREQ = 18;
  delay(1000 / GPS_FREQ);
}

void radio_loop() {
  delay(1000 / RADIO_FREQ);

  Frame f = createFrame(
    last_coords,
    last_accel,
    last_gyro,
    last_alt
#if CAPSULE == 2
    ,
    last_voc,
    last_humid,
    last_temp
#endif
  );

  Serial1.write(
    (char*)&f,
    FRAME_SIZE()
  );
}

#endif
