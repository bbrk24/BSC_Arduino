#if false

#include "gps.h"

GPS testGPS(&Wire);

void setup() {
  Serial.begin(9600);
  while (!Serial) { 
    //Nothing
  }
  Serial.println("\nConnecting to sensor...");
  testGPS.initialize();
}

void loop() {
  // put your main code here, to run repeatedly:

  GPS::Coordinates testCoordinates;

  bool justChecking = testGPS.getLocation(&testCoordinates);

  if (justChecking){
    Serial.print("\nLatitude: ");
    Serial.print(testCoordinates.latitude);
    Serial.print("\nLongitude: ");
    Serial.print(testCoordinates.longitude);
    Serial.print("\nAltitude: ");
    Serial.print(testCoordinates.altitudeMSL);
    Serial.print("ft");

    Serial.print("Hour: ");
    Serial.print(testCoordinates.timestamp.hours);
    Serial.print("\nMinute: ");
    Serial.print(testCoordinates.timestamp.minutes);
    Serial.print("\nSecond: ");
    Serial.print(testCoordinates.timestamp.seconds);
    Serial.print("\nMillisecond: ");
    Serial.print(testCoordinates.timestamp.milliseconds);

    delay(1000);

  } else {
    Serial.println("Unable to read values for some reason...");
    delay(1000);
  }
}

#endif 
