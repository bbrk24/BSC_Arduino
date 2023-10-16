#include <SD.h>

const int chipSelectPin = 28;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  while (!Serial) {
    Serial.print("Initializing SD card...");
  }

  if (!SD.begin(chipSelectPin)) {
    Serial.println("initialization failed. Things to check:");

    Serial.println("1. is a card inserted?");

    Serial.println("2. is your wiring correct?");

    Serial.println("3. did you change the chipSelect pin to match your shield or module?");

    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");

    while (true);
  }

  Serial.println("initialization done.");

  File dataFile = SD.open("datalog.txt");

  // if the file is available, write to it:

  if (dataFile) {

    while (dataFile.available()) {

      Serial.write(dataFile.read());

    }

    dataFile.close();

  }

  else {

    Serial.println("error opening datalog.txt");

  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
