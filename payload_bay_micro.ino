// When uploading to the payload bay micro, change this to true and the other INO to false
#if false
#include "altimeter.h"
#include "Buffer.h"

// Set this to "true" to allow the radio to force ejection
// Only turn this on during testing -- this should be "false" on the rocket!
#define EJECT_COMMAND false

// How often to signal the radio, in Hertz
const unsigned long RADIO_FREQ = 20;
// The last time the altitude was transmitted, in us
unsigned long lastRadioTime = 0;

bool turnedOff = true;

unsigned long time;

Altimeter alt;

constexpr float mapFloat(
  float value,
  float xMin, float xMax,
  float yMin, float yMax
) noexcept {
  return (yMax - yMin) / (xMax - xMin) * (value - xMin) + yMin;
}

float readTankPressure() {
  int rawADCReading = analogRead(2);
  float voltage = 5.0F/1023.0F * rawADCReading;
  // 0.5V = 0MPa, 4.5V = 3MPa, linear
  float pressureMPa = mapFloat(voltage, 0.5F, 4.5F, 0.0F, 3.0F);
  const float PSI_PER_MPA = 145.03774F;
  return pressureMPa * PSI_PER_MPA;
}

void sendToRadio(float altitude, float tankPressure, bool ejected) {
  char buf[19];
  snprintf(
    buf,
    sizeof buf,
    "%+06d,%03d,%d\n",
    (int)altitude,
    max(0, (int)tankPressure),
    ejected ? 0 : 1
  );
  lastRadioTime = micros();
  Serial1.print(buf);
}

void setup() {
  Serial1.begin(57600, SERIAL_8N2);

  pinMode(A2, INPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); //Sets up pin 8 to be the signal to eject capsules
  pinMode(9, OUTPUT);
  // digitalWrite(9, LOW); // Sets up pin 9 for status LEDs
  do { //Until the altimeter is active, attempt to initialize
    alt.initialize();
  } while (alt.getStatus() != Altimeter::ACTIVE);
  // digitalWrite(9, HIGH);
  // Wait for radio command
  while (true) {
    if (Serial1.available()) {
      String command = Serial1.readStringUntil('\n');
      if (command == "start") {
        digitalWrite(9, HIGH);
        break;
      } else if (command == "transmit_data") {
        sendToRadio(alt.getAltitude(), readTankPressure(), false);
      } else if (command == "fill") {
        // Send data without doing anything else until told to stop
        // Used to monitor tank pressure while filling
        while (true) {
          sendToRadio(0.0f, readTankPressure(), false);
          if (Serial1.available()) {
            String newCommand = Serial1.readStringUntil('\n');
            if (newCommand == "fill") {
              break;
            }
          }
          delay(1000 / RADIO_FREQ);
        }
#if EJECT_COMMAND
      } else if (command == "eject") {
        // Send data until we get a second "eject" command
        while (true) {
          sendToRadio(alt.getAltitude(), readTankPressure(), false);
          if (Serial1.available()) {
            String newCommand = Serial1.readStringUntil('\n');
            if (newCommand == "eject") {
              break;
            }
          }
          delay(1000 / RADIO_FREQ);
        }
        // Turn on the relay and go back to sending data
        digitalWrite(8, LOW);
        while (true) {
          sendToRadio(alt.getAltitude(), readTankPressure(), true);
          delay(1000 / RADIO_FREQ);
        }
#endif
      } else {
        sendToRadio(0.0f, 0.0f, false);
      }
    }
  }
}

enum Mode {
  BELOW_5K,
  WATCHING,
  PAST_APOGEE,
};

void loop() {
  static Buffer data;
  static Mode mode = BELOW_5K;

  float altitude;
  

  switch (mode) {
  case BELOW_5K:
    delay(1000 / RADIO_FREQ);
    altitude = alt.getAltitude();
    if (altitude >= 5000.0F) {
      mode = WATCHING;
    }
    break;
  case WATCHING:
    altitude = alt.getAltitude();
    data.addPoint(altitude);
    if (data.isDecreasing()) { //If we notice our altitude is decreasing, we've reached apogee
      time = millis();
      turnedOff = false;
      digitalWrite(8, LOW); //Flip ejection pin low
      mode = PAST_APOGEE; //Put us in 'PAST_APOGEE' mode
    }
    break;
  case PAST_APOGEE:
    // Track the altitude just for the radio
    if (!turnedOff && ((millis() - time) > 10000)){ //greater than 10 seconds
      turnedOff = true;
      digitalWrite(8, HIGH);
    }
    altitude = alt.getAltitude();
    break;
  }

  if (micros() - lastRadioTime >= 1e6 / RADIO_FREQ) {
    sendToRadio(altitude, readTankPressure(), mode == PAST_APOGEE);
  }
}

#endif
