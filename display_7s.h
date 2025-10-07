/*
 * display_7s.h
 *
 * Created: 12.09.2023 21:50:57
 *  Author: marci
 */ 
#include <avr/io.h>

#define LED_DATA_PORT PORTB
#define LED_CONTROL_PORT_1 PORTA
#define LED_CONTROL_PORT_2 PORTD
#define VFD0 PA2
#define VFD1 PA3
#define VFD2 PA4
#define VFD3 PA5
#define VFD4 PA6
#define VFD5 PA7
#define VFD6 PD0
#define VFD7 PD1

void displayChars(unsigned char chars[]);
void my_delay_us(int microseconds);
void dimmer(unsigned int brightness);
void displayInit();
void displaySegment(unsigned char displayNumber, unsigned char segmentCode, unsigned char brightness);
void setTempDimmer(const bool newValues[8]);
void clearTempDimmer();