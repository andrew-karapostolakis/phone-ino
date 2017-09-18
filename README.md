# phone-ino
A phone sound and display driver for Arduino
## Overview
This program uses a 4x4 numeric keypad, two 4-digit, 7-segment displays, an 8 Ω speaker, and an Arduino Uno to create a simulation of telephone dialing.
## Setup
The displays must be connected to two shift registers connected in series, so that one feeds into the other.
Of two bytes fed subsequently to the latch on the register connected directly to the Arduino, the first controls the character displayed on a segment, and the second indicates which segment to display it on.
(I realise this may be hard to follow, I'll try to create a circuit diagram later.)
### Pinout
* Pin A0: Column 4 of keypad
* Pin 2: Column 3 of keypad
* Pin 3: Column 2 of keypad
* Pin 4: Column 1 of kayped
* Pin 5: Row 4 of keypad
* Pin 6: Row 3 of keypad
* Pin 7: Row 2 of keypad
* Pin 8: Row 1 of keypad
* Pin 9: Shift register latch
* Pin 10: Shift register clock
* Pin 11: Shift register data
* Pin 12: Speaker power 1
* Pin 13: Speaker power 2
### Dependencies
This program requires the Keypad.h library, which can be found in Keypad/src.
## Credits
Co-creator: Qi Dong

Sound output control adapted from code by David Dubins, which can be found [here](https://playground.arduino.cc/Main/DTMF "Arduino Playground - DTMF").

Shift register output control by Tom Igoe and Carlyn Maw, found [here](https://www.arduino.cc/en/Tutorial/ShftOut21 "Arduino - ShftOut21").

Keypad library by Mark Stanley and Alexander Brevig, found [here](https://playground.arduino.cc/Code/Keypad "Arduino Playground - Keypad Library").
