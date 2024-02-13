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
static unsigned int shortDelay = 5; //THIS IS THE NUMBER OF MILLISECONDS THAT YOU WANT THE IMU TO RECORD TO CSV (ie 5 milliseconds is 200Hz)
static int maxTime = 10; //THIS IS THE AMOUNT OF TIME YOU WANT THE IMU TO RECORD. YOU CAN NOT PULL THE PLUG ON THE BATTERY
                          //UNTIL THIS AMOUNT OF TIME IS UP, OTHERWISE THE CSV FILE WILL NOT PROPERLY CLOSE AND THAT RUNS DATA WILL BE LOST
bool firstTime = true;

// Keep trying to initialize the SD card until it works
void initializeCard() {
    SDCard::Status status;
    do {
        status = card.getStatus();
        switch (status) {
            case SDCard::NOT_CONNECTED:
                card.initialize();
                break;
            case SDCard::UNTESTED:
                card.initialize();
                break;
            case SDCard::FILE_CLOSED:
                card.initialize();
                break;
        }
    } while (status != SDCard::ACTIVE);
}

void setup() {
  // put your setup code here, to run once: this is in case of 'print' statements
  //Serial.begin(9600);
  // (!Serial){
    //Nothing
  //}
  initializeCard();

  card.writeHeaders();

  testIMU.initialize();
}

//In this Dynamic Test we want to record data to the SD card. We will drop the IMU, save the data to an SD card, 
//and we will look at the data afterwards and determine how close the IMU got to feeling 0 acceleration
void loop(){ 

  testIMU.getValues(&testAccel, nullptr);
  card.writeToCSV(fakeCoords, testAccel, 0, fakeGyro);

  //wait until 2 milliseconds have passed, taking into account runtime of code
  delayMicroseconds((shortDelay * 1000) - (micros() % (shortDelay * 1000))); 

  //This is to make the time a lot easier for us to handle
  fakeCoords.timestamp.milliseconds += shortDelay;
  if (int(fakeCoords.timestamp.milliseconds) >= 1000){
    fakeCoords.timestamp.seconds += 1U;
    fakeCoords.timestamp.milliseconds -= 1000U;
  }

  if (fakeCoords.timestamp.seconds >= (unsigned int)maxTime){
    if (firstTime){
      card.closeFile();
      firstTime = false;
    }
    
  } 
}

#endif