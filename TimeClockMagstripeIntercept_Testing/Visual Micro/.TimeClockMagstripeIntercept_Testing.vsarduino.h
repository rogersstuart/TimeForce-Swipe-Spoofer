/* 
	Editor: http://www.visualmicro.com
	        arduino debugger, visual micro +, free forum and wiki
	
	Hardware: Arduino Pro or Pro Mini (5V, 16 MHz) w/ ATmega328, Platform=avr, Package=arduino
*/

#define __AVR_ATmega328p__
#define __AVR_ATmega328P__
#define ARDUINO 101
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define __cplusplus
extern "C" void __cxa_pure_virtual() {;}

//
//
void initalizePins();
uint16_t convertUIDToTimeForce(uint8_t UID[], uint8_t UID_length);
uint8_t emulateMagstripe(uint16_t to_convert);
void shiftOutByte(uint8_t to_shift);
void pulseClock();

#include "C:\Users\Stuart\Desktop\arduino-1.0.5-r2 (MasterController and Panels)\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Users\Stuart\Desktop\arduino-1.0.5-r2 (MasterController and Panels)\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\Stuart\Documents\Arduino\TimeClockMagstripeIntercept_Testing\TimeClockMagstripeIntercept_Testing.ino"
