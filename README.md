# BSC Arduino

This repo contains the Arduino code for the BSC 2023-24 team.

In order to compile this, you will need to install the Arduino libraries `Adafruit MPL3115A2 Library` and `Adafruit BNO08x`. The circuit schematic is included as Design1.ms14.

## Note on wait loops

Every wait loop (such as `while (!ready) {}`) must contain a call to `yield()` or `delay()`. Do not leave the body empty (as `{}`) or call `delayMicroseconds()`, as these both block the whole chip. `yield()` and `delay()` both allow other threads to make progress.
The exception to this is loops immediately inside of `setup()`, as the threads do not begin until that function ends.

> At the time of writing, this project is deployed to a Nano board that does not have SAMD and therefore does not support the Scheduler library. The final product will use a SAMD board and have at least two threads, one for the sensors and one for the radio.
