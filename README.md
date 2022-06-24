# Overview
I got a Hyperkin Ranger for use with my C64, but I discovered it uses non-standard signaling:  When a button is pressed, it sends GROUND (which is correct). When buttons are not pressed, the pins should be "floating" but instead they are high (+5). This conflicts with the C64 keyboard.

I created this prototype of hardware + an Arduino sketch (in the form of a PlatformIO project) to tame the signals from the Hyperkin Ranger. Additionally, I added a switch to change which joystick port the signal is being directed to.

This doesn't pass pins 5 or 9 either way. So it isn't compatible with mice or paddles. I'm NOT SURE if this will make it compatible with Genesis controllers or not.

I ended up using an Arduino Nano where I used ALL of the GPIO (except TX and RX, so I can have Serial, additionally A6, A7 cannot be used for digital IO).  I needed pin 13 (the LED pin) so I de-soldered the LED and resistor related to pin 13 (not too hard, actually). The wiring is pretty straight forward using a an ElectroCookie half length protoboard.

I could use less GPIO if I wired it with some 4066's or something similar, but that would be a ton more wiring. I could use a different microcontroller with more GPIO, likely requiring a 3v/5v level converter to read the input stick. An esp32 would almost certainly work.

I mentioned this issue to Hyperkin technical support. They said they would pass the issue along to their engineers, but I expect nothing to come of it.

Video demonstration available at https://youtu.be/-K58jPbteVA

# Wiring

## **WARNINGs (READ THESE!!)!**

* Do not connect the Nano's USB port while the circuit is connected to the C64!
* To use this with a Nano, you MUST either sacrafice one of TX, RX, or the built-in LED.
* If you decide to use pin 13 (the LED) you will need to de-solder the LED and resistor associated with pin 13.
* You must use a 5V microcontroller for this project, or you must add a 3v/5v level converter for the inputs from the joystick. The Hyperkin Ranger outputs ~+5V on each of the directions that isn't being pressed.
* Using an Arduino Uno for this doesn't help. i2c is still on pins A4, A5 - the Uno has no more GPIO than the Nano.

## Notes
* TX, RX are unused so one can still use Serial print for debugging
* A6, A7 are unused (only usable with analog signals)
* A4, A5 are used with i2c for the SSD1306 OLED

## Used GPIO
* Input direction UP on pin A3
* Input direction DOWN on pin A2
* Input direction LEFT on pin A1
* Input direction RIGHT on pin A0
* Input FIRE on pin 13
* Output to joystick port 2 direction UP on pin 2
* Output to joystick port 2 direction DOWN on pin 3
* Output to joystick port 2 direction LEFT on pin 4
* Output to joystick port 2 direction RIGHT on pin 5
* Output to joystick port 2 FIRE on pin 6
* Input port select switch on pin 7
* Output to joystick port 1 direction UP on pin 8
* Output to joystick port 1 direction DOWN on pin 9
* Output to joystick port 1 direction LEFT on pin 10
* Output to joystick port 1 direction RIGHT on pin 11
* Output to joystick port 1 FIRE on pin 12

## Powering the circuit from the C64
* `+5` from both joystick ports should go to the Nano's `5V` pin
* `Ground` from both joystick ports should go to the Nano's `GND` pin

## Behavior

* When an input direction or fire is pressed, the appropriate output is set to Ground.
* For all directions or fires that are not currently being pressed pressed, the output signal will be floating. This is done by setting non-pressed output pin to be "inputs".

# Building and deploying to an Arduino Nano

This is build using PlatformIO. The general process is

* Plug in your Arduino Nano via USB
* Make sure you have appropriate drivers for it (you may need to install CH340 drivers)
* Install Visual Studio Code
* Install the PlatformIO plugin
* Pull or download the code from Github
* Within PlatformIO "Open" a project, selecting your c64-joytamer source folder (from Github)
* Open the command palette `Control-Shift-P` and select `PlatformIO: Upload`
* NOTE: You may need to specify the `upload_port` in `platformio.ini` (or even better use `platformio_override.ini`)
