// When uploading to the payload bay micro, change this to true and the other INO to false
#if false

#include "altimeter.h"
#include "Buffer.h"

// How often to signal the radio, in Hertz
const unsigned long RADIO_FREQ = 20;
// The last time the altitude was transmitted, in us
unsigned long lastRadioTime = 0;

Altimeter alt;

void sendToRadio(float altitude) {
  char buf[11];
  // '+': always print the sign character, even if it's + instead of -
  // '0': pad the number with leading zeros if necessary (otherwise uses spaces)
  // '9': the whole number is 9 characters (+23456.89)
  // '.2': show two digits after the decimal point
  // 'f': the number is a double (there is none for float)
  sprintf(buf, "%+09.2f\n", (double)altitude);
  lastRadioTime = micros();
  Serial.print(buf);
}

void setup() {
  Serial.begin(230400);

  pinMode(8, OUTPUT);
  digitalWrite(8, LOW); //Sets up pin 8 to be the signal to eject capsules
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW); // Sets up pin 9 for status LEDs
  do { //Until the altimeter is active, attempt to initialize
    alt.initialize();
  } while (alt.getStatus() != Altimeter::ACTIVE);
  digitalWrite(9, HIGH);
  // Wait for radio command
  while (true) {
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      if (command == "start") {
        break;
      } else if (command == "transmit_data") {
        sendToRadio(alt.getAltitude());
      } else {
        sendToRadio(0.0f);
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
    delay(1000); //Wait 1 second
    altitude = alt.getAltitude();
    if (altitude >= 5000.0F) {
      mode = WATCHING;
    }
    break;
  case WATCHING:
    altitude = alt.getAltitude();
    data.addPoint(altitude);
    if (data.isDecreasing()) { //If we notice our altitude is decreasing, we've reached apogee
      digitalWrite(8, HIGH); //Flip ejection pin high
      mode = PAST_APOGEE; //Put us in 'PAST_APOGEE' mode
    }
    break;
  case PAST_APOGEE:
    // Track the altitude just for the radio
    altitude = alt.getAltitude();
    break;
  }

  if (micros() - lastRadioTime >= 1e6 / RADIO_FREQ) {
    sendToRadio(altitude);
  }
}

#endif
