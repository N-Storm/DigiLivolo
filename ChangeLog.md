Changelog
=========

v0.8.1 - 2026-03-07
-------------------
- Added option to list USB devices in the software front end (-l, --list).

v0.8.0 - 2025-04-28
-------------------
- Updated CI and release workflow, changed Github Actions build options to
  fix builds with newer runner environments and toolchains, to improve
  reliability of automated builds.

v0.6.3 - 2024-06-24
-------------------
- Software fix for incorrect firmware version detection on Linux/hidraw
  builds, corrected the source of device version information to use
  the new info handle.
- Changed warning behavior for the legacy transmit algorithm option,
  the warning for unsupported old algorithm now appears only if the
  -o option is explicitly selected.

v0.6.2 - 2024-05-21
-------------------
- Minor repository housekeeping, fixed markdown lint warnings and
  documentation formatting.

v0.6.1 - 2024-05-20
-------------------
- New transmitter code. Better accuracy and less error prone.

v0.6.0-pre0 - 2024-05-20
------------------------
- Fixed non MSYS2/gcc build issues, introducing a wrapper macro for the
  sleep function.
- Livolo transmitter related code improvements.
- Reduced sleep duration when waiting for ACK reports from device.

v0.4.5 - 2024-05-01
-------------------
- Internal fixes and improvements.

v0.4.4 - 2024-04-29
-------------------
- Fixed runner path inside CI container build scripts, improving
  CI reliability.

v0.4.3 - 2024-04-29
-------------------
- Minor documentation corrections.

v0.4.2 - 2024-04-22
-------------------
- Github Actions build improvements, added MSYS2 UCRT64 Windows builds,
  these Windows builds are now used by default for releases.

v0.4.1 - 2024-04-22
-------------------
- Fix for Windows path handling, addressing errors when building or
  running tools on Windows platforms.

v0.2.1 - 2024-04-17
-------------------
- First stable tested release with multiple feature additions and
  reorganization:
  * Added PC hidapi based console software to issue commands to device.
  * Hidapi moved into software/lib for distribution with the project.
  * Added argp-standalone submodule to support argument parsing on
    systems without argp.
  * Changed USB device name and vendor name, and added name to
    dlusb_packet struct.
  * Firmware version increased to 1.02.

v0.0.2 - 2024-04-13
-------------------
- Documentation fixes

v0.0.1 - 2024-04-12
-------------------
- Initial release
