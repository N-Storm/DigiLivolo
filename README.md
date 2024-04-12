# DigiLivolo

Firmware (for Digispark Arduino compatible module) & software to control Livolo RF 433 Mhz light switches.

## Description

Firmware are located inside `firmware` directory. It's supposed to be built with
[PlatformIO](https://platformio.org/). [VSCode](https://code.visualstudio.com/) preferred.

This code has been developed for Digispark module by Digistump. As it has already everything for
[V-USB](https://www.obdev.at/products/vusb/index.html). But probably would work on any AVR-based Arduino
module. But you'll have to add USB related parts and adjust V-USB config.

I've also included custom Digispark board config `digispark-micronucleus-6586` which are based on the
default Digispark board, but I've tweaked flash size for recent Micronucleus bootloader.

Because PlatformIO doesn't have latest [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino)
framework, I've included one in `firmware/packages/framework-arduino-avr-digistump`. But the firmware
should compile on standard framework as well.

Library `DLUSB` for the HID USB communications are based on DigiUSB library, included with
`DigistumpArduino`. It has been modified to allow more than 1 byte communications and other things which
make whole thing easier.

## Usage

Download compiled firmware from [releases](https://github.com/N-Storm/DigiLivolo/releases) or build from
sources (see below for instructions).

Upload firmware to your Digispark module. Connect DATA pin of 433 Mhz transmitter module (SYN115 based modules
were used & confirmed to work by me) to P5 of Digispark module (this pin can be changed in main sketch code).
Also connect VCC & GND of the module.

Once plugged to USB, the device should be recognized as USB HID device, not requiring any driver as every
modern OS have this standard USB class driver built-in.

To send codes to Livolo switches you can use [hidapitester](https://github.com/todbot/hidapitester) for now.
Work on the dedicated PC control software are in progress.

USB protocol are simple 8 bytes. First are HID REPORT ID, which are hardcoded to 76 (0x4C). 2nd are CMD ID,
which is only 0x01 (send code) for now. Next 2 bytes are Livolo Remote ID, little-endian (means you have to
reverse byte order from "normal" representation). 5th byte are Livolo Key ID. Remaining 3 bytes are reserved
and not used. They can be left as zeroes.

Example usage:

```hidapitester.exe --vidpid 16c0:05df -l 8 --open --send-feature 76,1,77,33,16```

This should send Livolo command as remote ID 0x214d (77,33) and key ID 0x10 (16).

## Building

```console
git checkout --recurse-submodules https://github.com/N-Storm/digiLivolo/
cd digiLivolo/firmware
pio run
```

Or open VSCode workspace after checkout, providing you have VSCode with PlatformIO plugin installed.

## Software used

* [PlatformIO](https://platformio.org/)
* [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino)
* [V-USB](https://www.obdev.at/products/vusb/index.html)
* [hidapi](https://github.com/libusb/hidapi)
* [hidapitester](https://github.com/todbot/hidapitester)
