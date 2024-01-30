#if false

#define CAPSULE 1

#include "SD_Card.h"

SDCard card;

// Keep trying to initialize the SD card until it works
void keepInitializingCard() {
    SDCard::Status status;
    do {
        status = card.getStatus();
        switch (status) {
            case SDCard::NOT_CONNECTED:
                Serial.println("SPI communications not initialized.");
                card.initialize();
                break;
            case SDCard::UNTESTED:
                Serial.println("Internal self-test has not passed. Retrying...");
                card.initialize();
                break;
            case SDCard::FILE_CLOSED:
                Serial.println("File closed. Reopening...");
                card.initialize();
                break;
        }
    } while (status != SDCard::ACTIVE);
}

void setup() {
    Serial.begin(9600);
    while (!Serial) { /* wait for serial monitor to connect */ }
    keepInitializingCard();

    // Write two lines of fake data
    GPS::Coordinates fakeCoords;
    fakeCoords.latitude = 391342467;
    fakeCoords.longitude = -845157413;
    fakeCoords.altitudeMSL = 790;
    fakeCoords.numSatellites = 7;
    fakeCoords.timestamp.hours = 12;
    fakeCoords.timestamp.minutes = 22;
    fakeCoords.timestamp.seconds = 12;
    fakeCoords.timestamp.milliseconds = 345;

    IMU::vector3 fakeAccel{0};
    fakeAccel.y = -9.81;

    IMU::vector3 fakeGyro{0};

    card.writeToCSV(fakeCoords, fakeAccel, 0, fakeGyro);
    card.writeToCSV(fakeCoords, fakeAccel, 0, fakeGyro);

    Serial.println("Fake data written. Closing file...");
    card.closeFile();
    Serial.println("File closed.");
}

void loop() {}
#endif
