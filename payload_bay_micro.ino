// When uploading to the payload bay micro, change this to true and the other INO to false
#if false

#include "altimeter.h"
#include "Buffer.h"

Altimeter alt;

void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW); //Sets up pin 8 to be the signal to eject capsules
  do { //Until the altimeter is active, attempt to initialize
    alt.initialize();
  } while (alt.getStatus() != Altimeter::ACTIVE);
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
    if (alt.getAltitude() >= 30.0F) {
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
    // nothing to do
    break;
  }
}

#endif
