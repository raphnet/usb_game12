# usb_game12: Firmware for USB game controller with 12 inputs (8 buttons + 4 directions)

usb_game12 is a firmware for Atmel ATmega8 which supports a total of 12 inputs (4 directions
and 8 buttons) to build a game controller. Ideal for Arcade-style controls and other
simple (one-wire-per-button) controllers such as Neo-Geo, Atari, etc.

The device connects to an USB port and appears to the PC as standard HID joystick.

## Project homepage

Schematic and additional information such as build examples are available on the project homepage:

* English: [USB game controller with 12 inputs (8 buttons + 4 directions)](http://www.raphnet.net/electronique/usb_game12/index_en.php)
* French: [Manette de jeu USB 12 entrées (8 boutons + 4 directions)](http://www.raphnet.net/electronique/usb_game12/index.php)

## Supported micro-controller(s)

Currently supported micro-controller(s):

* Atmega8

Adding support for other micro-controllers should be easy, as long as the target has enough
IO pins, enough memory (flash and SRAM) and is supported by V-USB.

## Built with

* [avr-gcc](https://gcc.gnu.org/wiki/avr-gcc)
* [avr-libc](http://www.nongnu.org/avr-libc/)
* [gnu make](https://www.gnu.org/software/make/manual/make.html)

## License

This project is licensed under the terms of the GNU General Public License, version 2.

## Acknowledgments

* Thank you to Objective development, author of [V-USB](https://www.obdev.at/products/vusb/index.html) for a wonderful software-only USB device implementation.
