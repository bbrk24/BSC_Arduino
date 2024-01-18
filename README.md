# BSC Arduino

This repo contains the Arduino code for the BSC 2023-24 team.

In order to compile this, you will need to install the Arduino libraries `Scheduler`, `Adafruit BMP3XX Library`, `Adafruit LSM9DS1 Library`, `Adafruit SHT4x Library`, and `SparkFun u-blox GNSS v3`. The circuit schematic is included in the .ms14 files. The relay in PayloadBay.ms14 is a placeholder relay and not necessarily representative of the actual relay that will be used.

## On the `.ino` files

The Arduino IDE has two requirements on the .ino file(s) in a folder:

- One of them must have the same name as the folder itself
- All of them will be compiled and uploaded, not just one

The capsule microcontroller code is in `sketch_oct9a.ino` to satisfy the first requirement. To satisfy the second requirement, the payload bay microcontroller code and all test scripts are wrapped in `#if false`. Before uploading any code, make sure the file you intend to run has `#if true` and all other files have `#if false`.

## Note on wait loops

For the capsule code, every wait loop (such as `while (!ready) {}`) must contain a call to `yield()` or `delay()`. Do not leave the body empty (as `{}`) or call `delayMicroseconds()`, as these both block the whole chip. `yield()` and `delay()` both allow other threads to make progress.
The exception to this is loops immediately inside of `setup()`, as the threads do not begin until that function ends.
