#if false

#include "imu.h"

IMU testIMU(&Wire);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial){
    //Nothing
  }
  Serial.println("Connecting to sensor...");
  testIMU.initialize();

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned int backoff = 1000;

  IMU::vector3 testAccel;
  IMU::vector3 testGyro;
  bool justChecking;

  justChecking = testIMU.getValues(&testAccel, &testGyro);

  if (justChecking){
    Serial.print("\n\nAccel X: ");
    Serial.print(testAccel.x);
    Serial.print("\nAccel Y: ");
    Serial.print(testAccel.y);
    Serial.print("\nAccel Z: ");
    Serial.print(testAccel.z);

    Serial.print("\n\nGyro X: ");
    Serial.print(testGyro.x);
    Serial.print("\nGyro Y: ");
    Serial.print(testGyro.y);
    Serial.print("\nGyro Z: ");
    Serial.print(testGyro.z);

  } else {
    Serial.println("Unable to get values...trying again");
    delay(backoff);
    backoff += backoff; //Doubling every time can't get value
  }
}

#endif