# DigiLivolo

[![Build firmware](https://github.com/N-Storm/DigiLivolo/actions/workflows/build-firmware.yml/badge.svg?branch=main)](https://github.com/N-Storm/DigiLivolo/actions/workflows/build-firmware.yml)
[![Build DigiLivolo software](https://github.com/N-Storm/DigiLivolo/actions/workflows/build-software.yml/badge.svg?branch=main)](https://github.com/N-Storm/DigiLivolo/actions/workflows/build-software.yml)

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

Download compiled firmware from [releases page](https://github.com/N-Storm/DigiLivolo/releases) or build from
sources (see below for instructions).

Software binaries for Windows x64, Linux x86_64 and ARM can be downloaded from
[releases page](https://github.com/N-Storm/DigiLivolo/releases) as well. Instructions for building from
sources are provided below.

_A more generic tool - [hidapitester](https://github.com/todbot/hidapitester) could be used to control device_
_as well._

Upload firmware to your Digispark module. Connect DATA pin of 433 Mhz transmitter module (SYN115 based modules
were used & confirmed to work by me) to P5 of Digispark module (this pin can be changed in main sketch code).
Also connect VCC & GND of the module.

![f](https://raw.githubusercontent.com/N-Storm/DigiLivolo/main/wiring.jpg)

Once plugged to USB, the device should be recognized as USB HID device, not requiring any driver as every
modern OS have this standard USB class driver built-in.

To send ON/OFF codes to Livolo switches with the `digilivolo` software you need to provide it with
Livolo Remote ID and key code as positional arguments:

```shell
Usage: digilivolo [OPTION...] REMOTE_ID KEY_ID

Software to control DigiLivolo devices.

 Positional arguments:
  KEY_CODE                   Livilo Key ID (1-255)
  REMOTE_ID                  Livilo Remote ID (1-65535)

 Options:
  -o, --old-alg              Use deperecated original transmit algorithm
  -v, --verbose              Produce verbose output

  -?, --help                 Give this help list
  -V, --version              Print program version
      --usage                Give a short usage message

Report bugs to https://github.com/N-Storm/DigiLivolo/
Copyright (c) 2024 GitHub user N-Storm.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
```

REMOTE_ID and KEY_ID can be specified either as deciman numbers or hex numbers if preceeded by `'0x'`.

Examples:

```shell
#  Remote ID 0x214d (8525) and key code 0x10 (16).
./digilivolo 0x214d 0x10

# Same in decimal numbers:
./digilivolo 8525 16
```

### Using from hidapitester

You can use [hidapitester](https://github.com/todbot/hidapitester) to communicate with device instead. It's a
generic tool for HID devices which allows to send or read feature reports which is the way to communicate with
the device.

Example usage:

```shell
hidapitester --vidpid 16c0:05df -l 8 --open --send-feature 76,1,77,33,16
```

This should send Livolo command as remote ID 0x214d (77,33) and key code 0x10 (16).

USB HID reports size set to 8 bytes. First are the HID REPORT ID, hardcoded to 76 (0x4C) in the USB descriptor.
Second byte are the CMD ID (command ID). Currently only one command 0x01 (send Livolo code) are suppored
for Host to Device reports. Next 2 bytes are the Livolo Remote ID, little-endian (means you have to reverse
byte order from "normal" representation). 5th byte are the Livolo Key code. Remaining 3 bytes are reserved and
not used, can be omitted in `hidapitester` invocation (will be sent as zeros).

## Building firmware

### With PlatformIO

Building with PlatformIO installed are simple. Just clone the repo & issue `pio run` from the firmware
directory.

```shell
git clone --recurse-submodules https://github.com/N-Storm/DigiLivolo
cd DigiLivolo/firmware
pio run
```

Or open VSCode workspace file `DigiLivolo.code-workspace` after cloning the repo if you want to build
it from there. Requires PlatformIO plugin installed.

### With Arduino IDE

* Download or clone this repo.
* Install [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino) core.
* Locate Digistump core install directory and replace `cores/tiny/core_build_options.h` file with version, included
  with this project: `firmware/packages/framework-arduino-avr-digistump/cores/core_build_options.h`
* Create new directory `DigiLivolo`, copy `firmware/src/DigiLivolo.cpp` as `DigiLivolo.ino` there.
* Copy `DLUSB`, `DLTransmitter` and `Livolo` libraries from `firmware/lib` to your Arduino libraries directory.
* Open `DigiLivolo.ino` with Arduino IDE, set board to DigiSpark and compile/upload.

## Building software

From the `DigiLivolo/software` directory:

```shell
cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev -B ./build . && cmake --build ./build
```

Or simply run `bash build.sh` script from this directory.

For building on Windows [MSYS2](https://www.msys2.org/) UCRT64 has been tested to work.

Resulting binary should be compiled as `build/digilivolo[.exe]`.

By default project compiles with `hidapi` library built from sources (linked as a git submodule) and
statically linked. If you wish to use system installed `hidapi` library and you have dev files (headers, etc)
installed, add `-DUSE_SYSTEM_HIDAPI=true` option to first cmake command on the example above.

## Software & libraries used

### Project

* [PlatformIO](https://platformio.org/)
* [DigistumpArduino](https://github.com/ArminJo/DigistumpArduino)
* [V-USB](https://www.obdev.at/products/vusb/index.html)
* [Livolo Arduino library](https://forum.arduino.cc/t/control-livolo-switches-livolo-switch-library/149850)
* [hidapi](https://github.com/libusb/hidapi)
* [hidapitester](https://github.com/todbot/hidapitester)
* [argp-standalone](https://github.com/tom42/argp-standalone)

### GitHub Actions

* [Checkout V3](https://github.com/actions/checkout/tree/releases/v3)
* [action gh-release](https://github.com/softprops/action-gh-release)
* [Setup MSYS2](https://github.com/msys2/setup-msys2)
* [install-package](https://github.com/ConorMacBride/install-package/)
* [Get Ninja GitHub Action](https://github.com/turtlesec-no/get-ninja)
* [GitHub Action: Setup Alpine Linux](https://github.com/marketplace/actions/setup-alpine-linux-environment)
