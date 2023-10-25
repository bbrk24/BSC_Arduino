# BSC Arduino

This repo contains the Arduino code for the BSC 2023-24 team.

In order to compile this, you will need to install the Arduino libraries `Adafruit BMP3XX Library` and `Adafruit BNO08x`. The circuit schematic is included in the .ms14 files. The relay in PayloadBay.ms14 is a placeholder relay and not necessarily representative of the actual relay that will be used.

## On the `.ino` files

The Arduino IDE has two requirements on the .ino file(s) in a folder:

- One of them must have the same name as the folder itself
- All of them will be compiled and uploaded, not just one

The capsule microcontroller code is in `sketch_oct9a.ino` to satisfy the first requirement. To satisfy the second requirement, the payload bay microcontroller code is wrapped in `#if false`. When uploading to the payload bay micro rather than the capsule, switch the `#if false` and `#if true` at the top of the .ino files.

## Note on wait loops

For the capsule code, every wait loop (such as `while (!ready) {}`) must contain a call to `yield()` or `delay()`. Do not leave the body empty (as `{}`) or call `delayMicroseconds()`, as these both block the whole chip. `yield()` and `delay()` both allow other threads to make progress.
The exception to this is loops immediately inside of `setup()`, as the threads do not begin until that function ends.

> At the time of writing, this project is deployed to a Nano board that does not have SAMD and therefore does not support the Scheduler library. The final product will use a SAMD board and have at least two threads, one for the sensors and one for the radio.
