#if true

#include "imu.h"
#include "gps.h"
#include "SD_Card.h"

IMU testIMU(&Wire);
SDCard card;

// Write two lines of fake data
GPS::Coordinates fakeCoords{0};

IMU::vector3 fakeGyro{0};
IMU::vector3 testAccel;
static int shortDelay = 5; //This is number of milliseconds per measurement, ie, 2 milliseconds is 500Hz data. Best to keep this a whole number
static int maxTime = 10;
bool firstTime = true;

// Keep trying to initialize the SD card until it works
void initializeCard() {
    SDCard::Status status;
    do {
        status = card.getStatus();
        switch (status) {
            case SDCard::NOT_CONNECTED:
                //Serial.println("SPI communications not initialized.");
                card.initialize();
                break;
            case SDCard::UNTESTED:
                //Serial.println("Internal self-test has not passed. Retrying...");
                card.initialize();
                break;
            case SDCard::FILE_CLOSED:
                //Serial.println("File closed. Reopening...");
                card.initialize();
                break;
        }
    } while (status != SDCard::ACTIVE);
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  // (!Serial){
    //Nothing
  //}
  //Serial.println("Testing SD Card...");
  initializeCard();

  card.writeHeaders();

  //Serial.println("Connecting to sensor...");
  testIMU.initialize();
}
/* THIS IS FOR STATIC TEST
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
} THIS IS FOR STATIC TEST
*/

//In this Dynamic Test we want to record data to the SD card. We will drop the IMU, save the data to an SD card, 
//and we will look at the data afterwards and determine how close the IMU got to feeling 0 acceleration
void loop(){ 

  testIMU.getValues(&testAccel, nullptr);
  card.writeToCSV(fakeCoords, testAccel, 0, fakeGyro);

  //wait until 2 milliseconds have passed, taking into account runtime of code
  delayMicroseconds((shortDelay * 1000) - (micros() % (shortDelay * 1000))); 

  //This is to make the time a lot easier for us to handle
  fakeCoords.timestamp.milliseconds += (unsigned int)shortDelay;
  if (int(fakeCoords.timestamp.milliseconds) >= 1000){
    //Serial.println("Another Second Passed...");
    fakeCoords.timestamp.seconds += (unsigned int)1;
    fakeCoords.timestamp.milliseconds -= (unsigned int)1000;
  }

  if (fakeCoords.timestamp.seconds >= (unsigned int)maxTime){
    if (firstTime){
      //Serial.println("File is closed...");
      //Serial.flush();
      card.closeFile();
      firstTime = false;
    }
    
  } 
}

#endif