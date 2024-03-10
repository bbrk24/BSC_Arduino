// When uploading to the payload bay micro, change this to true and the other INO to false
#if false

#include "altimeter.h"
#include "Buffer.h"

// How often to signal the radio, in Hertz
const unsigned long RADIO_FREQ = 20;
// The last time the altitude was transmitted, in us
unsigned long lastRadioTime = 0;

Altimeter alt;

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
        lastRadioTime = micros();
        Serial.println(alt.getAltitude());
      } else {
        Serial.println(0.0f);
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

  switch (mode) {
  case BELOW_5K:
    delay(1000); //Wait 1 second
    if (alt.getAltitude() >= 5000.0F) {
      mode = WATCHING;
    }
    break;
  case WATCHING:
    data.addPoint(alt.getAltitude());
    if (data.isDecreasing()) { //If we notice our altitude is decreasing, we've reached apogee
      digitalWrite(8, HIGH); //Flip ejection pin high
      mode = PAST_APOGEE; //Put us in 'PAST_APOGEE' mode
    }
    break;
  case PAST_APOGEE:
    // Track the altitude just for the radio
    data.addPoint(alt.getAltitude());
    break;
  }

  unsigned long time = micros();
  if (time - lastRadioTime >= 1e6 / RADIO_FREQ) {
    lastRadioTime = time;
    Serial.println(data.lastValue());
  }
}

#endif
