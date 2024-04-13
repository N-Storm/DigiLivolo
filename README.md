# DigiLivolo

Firmware for Digispark Arduino compatible module (any probably other AVR parts capable runninng V-USB)
to control Livolo RF 433 Mhz light switches via USB.

Once flashed, this firmware turns DigiSpark module into USB HID device. Which should have drivers
shipped with most modern OSes. I.e. no driver will be required.

Device accepts commands from USB Host with Livolo Remote ID & keycode and sends it on radio with
some kind of 433 MHz dumb OOK/ASK transmitter, like cheap SYN115 modules.

## Description

Firmware are located inside `firmware` directory. It's supposed to be built with
[PlatformIO](https://platformio.org/). [VSCode](https://code.visualstudio.com/) with PlatformIO plugin
are used for development. VSCode workspace file `DigiLivolo.code-workspace` are included and you can
open it with VSCode. Building project from VSCode are supported if you have PlatformIO plugin installed.

Firmware code has been developed for Digispark module by Digistump. As it has already everything for
[V-USB](https://www.obdev.at/products/vusb/index.html). Probably would work on any AVR-based Arduino
module. But you'll have to add USB related parts and adjust V-USB config.

I've also included custom Digispark board config `digispark-micronucleus-6586` which are based on the
default Digispark board, but I've tweaked flash size for recent Micronucleus bootloader.

Because PlatformIO doesn't have the latest [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino)
framework, I've included one in `firmware/packages/framework-arduino-avr-digistump`. But the firmware
should compile on standard framework as well.

Library `DLUSB` for the HID USB communications are based on DigiUSB library, included with
`DigistumpArduino`. It has been modified to allow more than 1 byte bidirectional USB communication via
USB HID Reports. As well as other various improvements.

## Usage

Download compiled firmware from [releases](https://github.com/N-Storm/DigiLivolo/releases) or build from
sources (see below for instructions).

Upload firmware to your Digispark module. Connect DATA pin of 433 Mhz transmitter module (SYN115 based modules
were used & confirmed to work by me) to P5 of Digispark module (this pin can be changed in main sketch code).
Also connect VCC & GND of the module.

![f](https://raw.githubusercontent.com/N-Storm/DigiLivolo/main/wiring.jpg)

Once plugged to USB, the device should be recognized as USB HID device, not requiring any driver as every
modern OS have this standard USB class driver built-in.

To send codes to Livolo switches you can use [hidapitester](https://github.com/todbot/hidapitester) for now.
Work on the dedicated PC control software are in progress.

USB HID reports size set to 8 bytes. First are the HID REPORT ID, hardcoded to 76 (0x4C) in USB descriptor.
Second byte are the CMD ID. Only 0x01 (send Livolo code) are suppored for Host to Device reports.
Next 2 bytes are the Livolo Remote ID, little-endian (means you have to reverse byte order from "normal"
representation). 5th byte are the Livolo Key code. Remaining 3 bytes are reserved and not used, can be omitted
in `hidapitester` invocation (will be sent as zeros).

Example usage:

```hidapitester.exe --vidpid 16c0:05df -l 8 --open --send-feature 76,1,77,33,16```

This should send Livolo command as remote ID 0x214d (77,33) and key code 0x10 (16).

## Building

### With PlatformIO

Building with PlatformIO installed are simple. Just clone the repo & issue `pio run` from the firmware
directory.

```console
git clone --recurse-submodules https://github.com/N-Storm/DigiLivolo
cd DigiLivolo/firmware
pio run
```

Or open VSCode workspace file `DigiLivolo.code-workspace` after cloning the repo if you want to build
it from there. Requires PlatformIO plugin installed.

### With Arduino IDE

* Download or clone this repo.
* Install [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino) core.
* Create new directory `DigiLivolo`, copy `firmware/src/DigiLivolo.cpp` as `DigiLivolo.ino` there.
* Copy `DLUSB` and `Livolo` libraries from `firmware/lib` to your Arduino libraries directory.
* Open `DigiLivolo.ino` with Arduino IDE, set board to DigiSpark and compile/upload.

## Software used

* [PlatformIO](https://platformio.org/)
* [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino)
* [V-USB](https://www.obdev.at/products/vusb/index.html)
* [Livolo Arduino library](https://forum.arduino.cc/t/control-livolo-switches-livolo-switch-library/149850)
* [hidapi](https://github.com/libusb/hidapi)
* [hidapitester](https://github.com/todbot/hidapitester)
