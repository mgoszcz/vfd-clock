/*
 * CFile1.c
 *
 * Created: 12.09.2023 21:50:27
 *  Author: marci
 */ 
#define F_CPU 4000000
#include <util/delay.h>
#include <stdbool.h>
#include "display_7s.h"
#include "coder_7seg.h"

unsigned int TIME_OFF, TIME_ON;
bool separator_on = false;

void displayInit() {
	TIME_OFF = 1900;
	TIME_ON = 100;
}
	
void my_delay_us(int microseconds) {
	int micro = microseconds % 1000;
	int mili = microseconds / 1000;
	for (int i=0; i<mili; i++) {
		_delay_ms(1);
	}
	for (int i=0; i< micro; i+= 10) {
		_delay_us(10);
	}
}

void dimmer(unsigned int brightness) {
	TIME_ON = 20 * brightness / 100;
	TIME_ON = TIME_ON * 100;
	TIME_OFF = 2000 - TIME_ON;
}

void displayChars (unsigned char chars[]) {
	// temporary begin
	//unsigned int num = TIME_ON, i=0;
	//num = 3000 * (100 / 100);
// 	while(num > 0) {
// 		int mod  = num % 10;
// 		num = num / 10;
// 		currentTime[5-i] = mod;
// 		i++;
// 	}
	// temporary end
	LED_DATA_PORT = codeToDisplay(chars[0]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[0]) != 0) LED_CONTROL_PORT_1 |= (1 << VFD0); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_1 &= ~(1 << VFD0);  // low
	_delay_us(10);
	
	
	LED_DATA_PORT = codeToDisplay(chars[1]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[1]) != 0) LED_CONTROL_PORT_1 |= (1 << VFD1); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_1 &= ~(1 << VFD1);  // low
	_delay_us(10);
	
	LED_DATA_PORT = codeToDisplay(chars[2]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[2]) != 0) LED_CONTROL_PORT_1 |= (1 << VFD2	); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_1 &= ~(1 << VFD2);  // low
	_delay_us(10);
	
	LED_DATA_PORT = codeToDisplay(chars[3]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[3]) != 0) LED_CONTROL_PORT_1 |= (1 << VFD3); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_1 &= ~(1 << VFD3);  // low
	_delay_us(10);
	
	LED_DATA_PORT = codeToDisplay(chars[4]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[4]) != 0) LED_CONTROL_PORT_1 |= (1 << VFD4); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_1 &= ~(1 << VFD4);  // low
	_delay_us(10);
	
	LED_DATA_PORT = codeToDisplay(chars[5]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[5]) != 0) LED_CONTROL_PORT_1 |= (1 << VFD5); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_1 &= ~(1 << VFD5);  // low
	_delay_us(10);
	
	LED_DATA_PORT = codeToDisplay(chars[6]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[6]) != 0) LED_CONTROL_PORT_2 |= (1 << VFD6); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_2 &= ~(1 << VFD6);  // low
	_delay_us(10);
	
	LED_DATA_PORT = codeToDisplay(chars[7]);
	my_delay_us(TIME_OFF);
	if (codeToDisplay(chars[7]) != 0) LED_CONTROL_PORT_2 |= (1 << VFD7); // high
	my_delay_us(TIME_ON);
	LED_CONTROL_PORT_2 &= ~(1 << VFD7);  // low
	_delay_us(10);
}

// Display specific segment on specific display with specific brightness
// Display number [0-7], segmentCode [100-107], brightness [0-100]
void displaySegment(unsigned char displayNumber, unsigned char segmentCode, unsigned char brightness) {
	int time_on = (20 * brightness / 100) * 100;
	int time_off = 2000 - time_on;
	unsigned char controlPort, display;
	LED_DATA_PORT = codeToDisplay(segmentCode);
	my_delay_us(time_off);
	if (displayNumber < 6) {
		controlPort = LED_CONTROL_PORT_1;
		display = displayNumber + 2;
	} else {
		controlPort = LED_CONTROL_PORT_2;
		display = displayNumber - 6;
	}
	if (codeToDisplay(segmentCode) != 0) controlPort |= (1 << display); // high
	my_delay_us(time_on);
	controlPort &= ~(1 << display);  // low
	_delay_us(10);
	LED_DATA_PORT = 0;
}