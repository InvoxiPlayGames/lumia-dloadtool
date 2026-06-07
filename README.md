# lumia-dloadtool

Tool for interfacing with Nokia's "DLOAD" bootloader, as used in the Lumia
series of Windows Phone 7 devices, over USB.

This tool is heavily **WORK IN PROGRESS**! There are **BUGS**! Things can, and
will, **GO WRONG!**

**THIS TOOL IS PROVIDED WITHOUT WARRANTY OF ANY KIND. I AM NOT RESPONSIBLE FOR
ANY DAMAGE THAT MIGHT ARISE AS A RESULT OF PROPER OR IMPROPER USAGE OF THIS
TOOL.**

## Usage

Because this is **VERY WORK IN PROGRESS**, I highly recommend you only use this
tool if you are a power user and are comfortable in trying to fix things if
anything goes wrong.

### Building

Compile with CMake. **Pre-compiled release or CI builds are not available yet.**

**(Non-Windows)** Requires libusb-1.0 development packages installed.

**(Windows)** Builds with MINGW64 (clang or GCC), **not** MSVC.

### Drivers

**Linux:** Install
[75-gen1-lumia-bootloader.rules](misc/75-gen1-lumia-bootloader.rules)
as udev rules (in `/etc/udev/rules.d/` or similar), or run `lumia-dloadtool` as
root.

**Windows:** Install
[Windows Device Recovery Tool](https://learn.microsoft.com/en-us/previous-versions/mixed-reality/hololens-1/hololens1-recovery#recover-your-hololens)
(click the "Windows Device Recovery Tool (WDRT) Installer" link), or find and
install a 2012 - 2014 version of Nokia Care Suite.

**macOS:** The tool will work out of the box.

### Commands

`./lumia-dloadtool`: detects a DLOAD device and displays build timestamp

`./lumia-dloadtool test`: detects a DLOAD device and fetches some information,
for [attempts to identify DLOAD](re-docs/attempts_to_identify_dload.txt)

`./lumia-dloadtool fwinfo [.cert file]`: prints info about a firmware .cert file

`./lumia-dloadtool fwverify [.cert file] [data file]`:
verifies the checksum integrity of a given .cert file and its corresponding data

`./lumia-dloadtool flash [.cert file] [data file]`:
detects a DLOAD device and then flashes the given .cert and data files to it -
**This is dangerous and could brick your device!**

Flashing OSBL (DLOAD/QCOM) has been tested. Flashing an OS image has **not** and
may leave your device **permanently damaged**. Use Care Suite for that.

Modem firmware (AMSS, etc) is not supported and neither is user_data_erase.
Use Care Suite for that.

## TODO

* Fix the stability issues. There's a lot of those. C is memory safe btw
* Detecting connected device type
* .esco support
* Testing on more devices
* Support for flashing modem firmware and user data erase
* Use system SHA256 libraries where available
* Better CLI interface
* Package for Linux/BSD distros
* Library-ize, C#/Python/Rust bindings
* Code clean-up (lots of duplicated code!)
* Make WinUSB code better.

(and more! see "TODO" in the codebase.)

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This software is licensed under the GNU General Public License version 2.
See [LICENSE.txt](LICENSE.txt) for more information.

Public domain SHA256 library by Kent "ethereal" Williams-King was used.

Header files contain structures, enums and constants from reverse engineered
bootloaders. You may use this information under the public domain.
