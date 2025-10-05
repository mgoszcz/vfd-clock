/*
 * main.c
 *
 * Created: 5/21/2024 10:22:24 PM
 *  Author: marci
 */ 

#define F_CPU 4000000
#define SET_BUTTON PD4
#define PLUS_BUTTON PD5
#define ALM_BUTTON PD3

#include <avr/io.h>
#include <xc.h>
#include <util/delay.h>
#include <stdbool.h>
#include "display_7s.h"
#include "RTC.h"

unsigned char displayString[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char currentTime[3] = {0, 0, 0};
unsigned char currentDate[4] = {0, 0, 0, 0};
unsigned char mainCounter = 0;
unsigned char brightness = 100;
unsigned char mode = 0;
unsigned char setButtonCounter = 0;
unsigned char plusButtonCounter = 0;
unsigned char almButtonCounter = 0;
bool blinker = true;
bool alarmActive = false;
bool editMode = false;
char marker = 0;
char editIndex = 0;

char getBrigthness() {
	ADCSRA |=(1<<ADSC);
	char r = ADC / 10;
	ADCSRA &= ~(1<<ADSC);
	if (r > 100) r = 100;
	return r;
}

void my_delay_ms(int miliseconds) {
	for (int i=0; i<miliseconds; i++) {
		_delay_ms(1);
	}
}

void displayLed() {
	PORTC |= (1 << PC3);
	my_delay_us(brightness);
	PORTC &= ~(1 << PC3);
}

void almButtonLedDisplay() {
	if (alarmActive) {
		PORTD |= (1 << PD6);
	} else {
		PORTD &= ~(1 << PD6);
	}
}

void ledDisplayAction() {
	if (mainCounter%10 == 0) {
		PORTC |= (1 << PC3);
	}
	if (brightness == 0 && mainCounter == 0) {
		PORTC &= ~(1 << PC3);
		return;
	}
	if (mainCounter == 0) {
		PORTC |= (1 << PC3);
		return;
	}
	unsigned char offCounter = ((250 - 50) * 100) / 100;
	if (mainCounter == offCounter) {
		PORTC &= ~(1 << PC3);
		return;
	}
}

void getTime(bool ignoreMarker) {
	if (ignoreMarker == true) {
		currentTime[2] = GetSeconds();
		currentTime[1] = GetMinutes();
		currentTime[0] = GetHours() & 0b111111;
		} else {
		switch (marker) {
			case 0:
			currentTime[2] = GetSeconds();
			break;
			case 1:
			currentTime[1] = GetMinutes();
			break;
			case 2:
			currentTime[0] = GetHours() & 0b111111;
			break;
		}
		marker++;
		if (marker == 3) marker = 0;
	}
	displayString[7] = (currentTime[2] &0b00001111);
	displayString[6] = (currentTime[2] &0b01110000)>>4;
	displayString[4] = (currentTime[1] &0b00001111);
	displayString[3] = (currentTime[1] &0b01110000)>>4;
	displayString[1] = (currentTime[0] &0b00001111);
	displayString[0] = (currentTime[0] &0b01110000)>>4;
	if (blinker) {
		displayString[5] = 62;
		displayString[2] = 62;
	} else {
		displayString[5] = 20;
		displayString[2] = 20;
	}
}

void getDate(bool ignoreMarker) {
	if (ignoreMarker == true) {
		currentDate[3] = GetMonthDay();
		currentDate[2] = GetMonth() & 0b11111;
		currentDate[1] = GetYear();
		currentDate[0] = GetMonth() & 0b10000000 >> 7;
		} else {
		switch (marker) {
			case 0:
			currentDate[3] = GetMonthDay();
			break;
			case 1:
			currentDate[2] = GetMonth() & 0b11111;
			break;
			case 2:
			currentDate[1] = GetYear();
			currentDate[0] = GetMonth() & 0b10000000 >> 7;
			break;
		}
		marker++;
		if (marker == 3) marker = 0;
	}
	displayString[0] = (currentDate[3] & 0b00110000)>>4;
	displayString[1] = (currentDate[3] & 0b00001111) + 10;
	displayString[2] = (currentDate[2] & 0b00010000)>>4;
	displayString[3] = (currentDate[2] & 0b00001111) + 10;
	if (currentDate[0] == 1) {
		displayString[4] = 2;
		displayString[5] = 0;
	} else {
		displayString[4] = 1;
		displayString[5] = 9;
	}
	displayString[6] = (currentDate[1] & 0b11110000)>>4;
	displayString[7] = (currentDate[1] & 0b00001111);
}

void getEditDataDisplay() {
	if (editIndex == 0 || editIndex == 1) {
		displayString[7] = 0;
		displayString[6] = 0;
		displayString[4] = (currentTime[1] &0b00001111);
		displayString[3] = (currentTime[1] &0b01110000)>>4;
		displayString[1] = (currentTime[0] &0b00001111);
		displayString[0] = (currentTime[0] &0b01110000)>>4;
		displayString[5] = 20;
		displayString[2] = 20;
		if (!blinker) {
			switch (editIndex) {
				case 0:
					displayString[1] = 20;
					displayString[0] = 20;
					break;
				case 1:
					displayString[3] = 20;
					displayString[4] = 20;
					break;
			}
		}
	}
}

void getBlinker() {
	unsigned char seconds = GetSeconds();
	unsigned char units = seconds & 0x0F;
	blinker = (units % 2 == 0);
}

void getDataToDisplay() {
	getBlinker();
	if (editMode) {
		getEditDataDisplay();
		return;
	}
	switch (mode) {
		case 0:
			getTime(false);
			break;
		case 1:
			getDate(false);
			break;
	}
}

void toggleMode() {
	mode++;
	if (mode > 1) {
		mode = 0;
	}
}

void stoper() {
	// 		displayChars(displayString);
	// 		counter++;
	// 		if (counter%3 == 0) {
	// 			if (displayString[7] != 9) {
	// 				displayString[7]++;
	// 			} else {
	// 				displayString[7] = 0;
	// 				if (displayString[6] != 9) {
	// 					displayString[6]++;
	// 				} else {
	// 					displayString[6] = 0;
	// 					if (displayString[5] != 9) {
	// 						displayString[5]++;
	// 						} else {
	// 						displayString[5] = 0;
	// 						if (displayString[4] != 9) {
	// 							displayString[4]++;
	// 							} else {
	// 							displayString[4] = 0;
	// 							if (displayString[3] != 9) {
	// 								displayString[3]++;
	// 								} else {
	// 								displayString[3] = 0;
	// 								if (displayString[2] != 9) {
	// 									displayString[2]++;
	// 									} else {
	// 									displayString[2] = 0;
	// 									if (displayString[1] != 9) {
	// 										displayString[1]++;
	// 										} else {
	// 										displayString[1] = 0;
	// 										if (displayString[0] != 9) {
	// 											displayString[0]++;
	// 											} else {
	// 											displayString[0] = 0;
	// 										}
	// 									}
	// 								}
	// 							}
	// 						}
	// 					}
	// 				}
	// 			}
	// 		}
}

int main(void)
{
    // sprawdzic blinker i jasnosc ledow po ustawieniu DDRC
	// edit mode
	// alarm button led
	my_delay_ms(500);
	Initialise_TWI_Master();
	getTime(true);
	getDate(true);
	
// 	ADMUX |=(1<<REFS0);
// 	ADCSRA |=(1<<ADEN)|(1<<ADPS0);
	DDRD &= (1 << PIND4);
	PORTD |= ( 1 << PIND4);
	
	DDRD |= (1 << PD6);
	DDRC |= (1 << PC3);
	PORTC &= ~(1 << PC3);
	PORTD |= (1 << PD6);
	
	
	
	while(1)
    {
		dimmer(brightness);
		if (!(PIND & (1 << SET_BUTTON)) || !(PIND & (1 << PLUS_BUTTON)) || !(PIND & (1 << ALM_BUTTON))) {
			if (!(PIND & (1 << SET_BUTTON))) {
				if (setButtonCounter > 125 && !editMode) {
					editMode = true;
					setButtonCounter = 0;
				}
			} else {
				setButtonCounter++;
			}
			if (!(PIND & (1 << PLUS_BUTTON))) plusButtonCounter++;
			if (!(PIND & (1 << ALM_BUTTON))) {
				if (almButtonCounter > 125) {
					// alm edit mode
					almButtonCounter = 0;
				}	
			} else {
				almButtonCounter++;
			}
		} else {
			if (setButtonCounter > 10) {
				toggleMode();
				setButtonCounter = 0;
			}
			plusButtonCounter = 0;
			if (almButtonCounter > 10) {
				alarmActive = !alarmActive;
				// save to eeprom
				almButtonLedDisplay();
				almButtonCounter = 0;	
			}
		}
		if (mainCounter%3 == 0){
			getDataToDisplay();
		}
		displayChars(displayString);
		displayLed();
		mainCounter++;
		if (mainCounter == 251) mainCounter = 0;
		
    }
}