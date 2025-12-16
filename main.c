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
#include <avr/interrupt.h>
#include "display_7s.h"
#include "RTC.h"

unsigned char displayString[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char currentTime[3] = {0, 0, 0};
unsigned char alarmTime[3] = {0, 0, 0};
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
bool blockSetButtonCounter = false;
bool blockAlmButtonCounter = false;
bool almEditMode = false;
bool almPreview = false;
bool alarmTriggered = false;
bool resetFlagInBlinker = false;
bool settingsMode = false;
bool blockPlusButtonCounter = false;
char marker = 0;
char editIndex = 0;
char previewCounter = 3;

const int monthDaysCount[12] = {
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
const int century[2] = {1900, 2000};

void getBrigthness() {
	ADCSRA |=(1<<ADSC);
	while (ADCSRA & (1 << ADSC));
	uint16_t adc = ADC;
	uint8_t r = adc * 100UL / 1023;
	if ((100 - r )< 5) {
		brightness = 10;
	} else if ((100 - r) < 15) {
		brightness = 40;
	} else if ((100 - r) < 25) {
		brightness = 80;
	} else {
		brightness = 100;
	}
}

unsigned char bcdToDec(unsigned char bcd) {
	return (((bcd & 0xf0) >> 4) * 10) + (bcd &0xf);
}

unsigned char decToBcd(unsigned char dec) {
	char decimals = dec/10;
	char units = dec%10;
	return (decimals<<4) + units;
}

unsigned char incrementBcd(unsigned char value) {
	return decToBcd(bcdToDec(value) + 1);
}

bool isLeapYear() {
	int year = century[currentDate[0]] + bcdToDec(currentDate[1]);
	if (year%4 != 0) return false;
	if (year%100 != 0) return true;
	if (year%400 != 0) return false;
	return true;
}

void my_delay_ms(int miliseconds) {
	for (int i=0; i<miliseconds; i++) {
		_delay_ms(1);
	}
}

void displayLed() {
	if (brightness <= 10) return;
	PORTC |= (1 << PC3);
	my_delay_us(2 * brightness);
	PORTC &= ~(1 << PC3);
}

void almButtonLedDisplay(bool isOn) {
	if (isOn) {
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

void storeDateInDisplayString() {
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
	storeDateInDisplayString();
}

void getEditDataDisplay() {
	bool newArray[8] = {false, false, false, false, false, false, false, false};
	if (editIndex == 0 || editIndex == 1) {
		displayString[7] = 0;
		displayString[6] = 0;
		displayString[4] = (currentTime[1] &0b00001111);
		displayString[3] = (currentTime[1] &0b01110000)>>4;
		displayString[1] = (currentTime[0] &0b00001111);
		displayString[0] = (currentTime[0] &0b01110000)>>4;
		displayString[5] = 62;
		displayString[2] = 62;
		if (!blinker) {
			switch (editIndex) {
				case 0:
					newArray[1] = true;
					newArray[0] = true;
					break;
				case 1:
					newArray[3] = true;
					newArray[4] = true;
					break;
			}
		}
	}
	if (editIndex == 2 || editIndex == 3 || editIndex == 4) {
		storeDateInDisplayString();
		if (!blinker) {
			switch (editIndex) {
				case 2:
				newArray[4] = true;
				newArray[5] = true;
				newArray[6] = true;
				newArray[7] = true;
				break;
				case 3:
				newArray[2] = true;
				newArray[3] = true;
				break;
				case 4:
				newArray[1] = true;
				newArray[0] = true;
				break;
			}
		}
	}
	setTempDimmer(newArray);
}

void getAlarmTime() {
	alarmTime[1] = getAlarm1Minutes() & 0x7F;
	alarmTime[0] = getAlarm1Hours() & 0x3F;
}

void getAlmEditDataDisplay() {
	bool newArray[8] = {false, false, false, false, false, false, false, false};
	displayString[7] = 20;
	displayString[6] = 20;
	displayString[4] = (alarmTime[1] &0xF);
	displayString[3] = (alarmTime[1] &0x70)>>4;
	displayString[1] = (alarmTime[0] &0xF);
	displayString[0] = (alarmTime[0] &0x70)>>4;
	displayString[5] = 20;
	displayString[2] = 62;
	if (!blinker) {
		switch (editIndex) {
			case 0:
			newArray[1] = true;
			newArray[0] = true;
			break;
			case 1:
			newArray[3] = true;
			newArray[4] = true;
			break;
		}
	}
	setTempDimmer(newArray);
}

void incrementCurrentIndex() {
	switch (editIndex) {
		case 0:
		currentTime[0] = incrementBcd(currentTime[0]);
		if (bcdToDec(currentTime[0]) == 24) currentTime[0] = 0;
		break;
		case 1:
		currentTime[1] = incrementBcd(currentTime[1]);
		if (bcdToDec(currentTime[1]) == 60) currentTime[1] = 0;
		break;
		case 2:
		currentDate[1] = incrementBcd(currentDate[1]);
		if (bcdToDec(currentDate[1]) == 100) {
			if (currentDate[0] == 0) currentDate[0] = 1;
			else currentDate[0] = 0;
			currentDate[1] = 0;
		}
		break;
		case 3:
		currentDate[2] = incrementBcd(currentDate[2]);
		if (bcdToDec(currentDate[2]) == 13) currentDate[2] = 1;
		break;
		case 4:
		currentDate[3] = incrementBcd(currentDate[3]);
		if (bcdToDec(currentDate[3]) > 28 && bcdToDec(currentDate[2]) == 2 && !isLeapYear()) currentDate[3] = 1;
		if (bcdToDec(currentDate[3]) > monthDaysCount[bcdToDec(currentDate[2]) - 1]) currentDate[3] = 1;
		break;
	}
}

void incrementAlmCurrentIndex() {
	switch (editIndex) {
		case 0:
		alarmTime[0] = incrementBcd(alarmTime[0]);
		if (bcdToDec(alarmTime[0]) == 24) alarmTime[0] = 0;
		break;
		case 1:
		alarmTime[1] = incrementBcd(alarmTime[1]);
		if (bcdToDec(alarmTime[1]) == 60) alarmTime[1] = 0;
		break;
	}
}

void resetAlarmFlag() {
	unsigned char statusData = getStatusData();
	statusData &= 0xFE;
	sendStatusData(statusData);
}

void getBlinker() {
	unsigned char seconds = GetSeconds();
	unsigned char units = seconds & 0x0F;
	bool blinkerState = (units % 2 == 0);
	if (blinker != blinkerState) {
		blinker = blinkerState;	
		if (previewCounter < 3 && almPreview) previewCounter++;
		if (previewCounter >= 3 && almPreview) almPreview = false;
		if (resetFlagInBlinker) {
			resetAlarmFlag();
			resetFlagInBlinker = false;
		}
	}
}

void getSettingsDataDisplay() {
	switch (editIndex) {
		case 0:
			displayString[0] = 29;
			displayString[1] = 25;
			displayString[2] = 24;
			displayString[3] = 62;
			displayString[4] = 20;
			displayString[5] = 31;
			displayString[6] = 30;
			displayString[7] = 20;
			break;
		case 1:
			displayString[0] = 22;
			displayString[1] = 33;
			displayString[2] = 34;
			displayString[3] = 62;
			displayString[4] = 21;
			displayString[5] = 35;
			displayString[6] = 34;
			displayString[7] = 31;
			break;
	}
}

void getDataToDisplay() {
	getBlinker();
	if (editMode) {
		getEditDataDisplay();
		return;
	}
	if (almEditMode) {
		getAlmEditDataDisplay();
		return;
	}
	if (settingsMode) {
		getSettingsDataDisplay();
		return;
	}
	if (previewCounter < 3 && almPreview) {
		displayString[0] = 21;
		displayString[1] = 29;
		displayString[2] = 20;
		displayString[7] = (alarmTime[1] &0xF);
		displayString[6] = (alarmTime[1] &0x70)>>4;
		displayString[4] = (alarmTime[0] &0xF);
		displayString[3] = (alarmTime[0] &0x70)>>4;
		displayString[5] = 62;
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

void sendDataToRTC() {
// 	if (SendHours(currentTime[0]) != 0) displayError((unsigned char)2);
// 	if (SendMinutes(currentTime[1]) != 0) displayError((unsigned char)2);
// 	if (SendSeconds(0) != 0) displayError((unsigned char)2);
// 	if (SendMonthDay(currentDate[3]) != 0) displayError((unsigned char)2);
// 	if (SendMonth((currentDate[0] << 7) + currentDate[2]) != 0) displayError((unsigned char)2);
// 	if (SendYear(currentDate[1]) != 0) displayError((unsigned char)2);
	SendHours(currentTime[0]);
	SendMinutes(currentTime[1]);
	SendSeconds(0);
	SendMonthDay(currentDate[3]);
	SendMonth((currentDate[0] << 7) + currentDate[2]);
	SendYear(currentDate[1]);
}

void setAlarmActive() {
	alarmActive = true;
	previewCounter = 0;
	almPreview = true;
	resetAlarmFlag();
	unsigned char controlData = getControlData();
	sendControlData(controlData | 0b1);
}

void increaseEditIndex() {
	editIndex++;
	if (editIndex == 5 && editMode) {
		editMode = false;
		editIndex = 0;
		sendDataToRTC();
		clearTempDimmer();
	}
	else if (editIndex == 2 && almEditMode) {
		almEditMode = false;
		editIndex = 0;
		sendAlarm1Minutes(alarmTime[1]);
		sendAlarm1Hours(alarmTime[0]);
		sendAlarm1Seconds(0);
		clearTempDimmer();
		setAlarmActive();
		almButtonLedDisplay(alarmActive);
	} else if (editIndex == 2 && settingsMode) {
		settingsMode = false;
		editIndex = 0;
	}
}

void presetAlm() {
	sendAlarm1Minutes(1);
	sendAlarm1Hours(0);
	sendAlarm1Seconds(0);
	sendAlarm1Date(0x80);
	resetAlarmFlag();
}

void presetDate() {
	if (currentDate[1] == 0 && currentDate[2] == 1 && currentDate[3] == 1 && currentTime[0] == 0 && currentTime[1] == 0) {
		SendYear(0x20);
		SendMonth(0x81);
	}
}

void toggleAlarm() {
	if (!alarmActive) {
		setAlarmActive();
	} else {
		alarmActive = false;
		unsigned char controlData = getControlData();
		sendControlData(controlData & 0xFE);
	}
	almButtonLedDisplay(alarmActive);
}

void readAlarmActive() {
	if ((getControlData() & 0b1) == 1) {
		alarmActive = true;
	} else {
		alarmActive = false;
	}
	almButtonLedDisplay(alarmActive);
}

void dismissAlarm() {
	alarmTriggered = false;
	PORTC &= ~(1 << PC2);
	almButtonLedDisplay(alarmActive);
	if (alarmTime[2] != currentTime[2]) {
		resetAlarmFlag();
	} else {
		resetFlagInBlinker = true;
	}
}

ISR(INT0_vect)
{
	alarmTriggered = true;
	_delay_ms(50);
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
    // obsluga alarmu
	//   gotowe: alm edit mode, ustawianie czasu i wysylanie TYLKO CZASU do RTC
	//   gotowe: ustawianie rejestrow alarmu (hour + minutes + seconds to trigger)
	//   gotowe dodac wysylanie sekund (0)
	//   gotowe dodac obsluge przerwan w atmedze
	//   dodac alarm triggered i cale zachowanie zegara przy alarm triggered (mruganie i glosnik)
	//       done nie ma przebiegu na wyjsciu 555 (jest ciagle 5V) - do ogarniecia, obecny brzeczyk dziala przy stalym napieciu 5V
	//       done dodac obsluge alarmActive (rozwazyc ustawianie flaig na rtc zeby wylaczac wylaczac alarm)
    //       done dodac obsluge wylaczania alarmu
	//       done kasowanie flagi na rtc
	// done sprawdzic ustawianie roku - pierwsze ustawienie np 2014 powoduje zapis i wyswietlanie 1914 - chyba cos bylo zjebane w dsc ze pierwszy zapis resetowal century, preset time ponizej to rozwiazal
	// wyswietlanie temperatury
	// done dodac wstepne ustawianie daty gdy 01.01.2000
	// done obsluga przyciemniania
	// menu ustawien ( przytrzymanie przycisku + )
	//    LEDy - on/off
	//    jasnosc - 10, 40, 80, 100, auto
	//    eeprom
	my_delay_ms(500);
	Initialise_TWI_Master();
	my_delay_ms(500);
	presetAlm();
	readAlarmActive();
	
	getTime(true);
	getDate(true);
	getAlarmTime();
	presetDate();
	
	GICR |= (1 << INT0);
	MCUCR &= ~(1 << ISC00);
	MCUCR |=  (1 << ISC01);
	
	sei();
	
	
 	ADMUX  = (1 << REFS0);
 	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0);
	DDRD &= (1 << SET_BUTTON);
	DDRD &= (1 << PLUS_BUTTON);
	PORTD |= ( 1 << ALM_BUTTON);
	PORTD |= ( 1 << SET_BUTTON);
	PORTD |= ( 1 << PLUS_BUTTON);
	
	DDRD |= (1 << PD6);
	DDRC |= (1 << PC3);
	DDRC |= (1 << PC2);
	DDRC  &= ~(1 << PC0);
	PORTC &= ~(1 << PC0);
	PORTC &= ~(1 << PC3);

	while(1)
    {
		if (alarmTriggered) {
			if (blinker) {
				brightness = 100;
				almButtonLedDisplay(true);
				PORTC |= (1 << PC2);
			}
			else {
				brightness = 10;
				almButtonLedDisplay(false);
				PORTC &= ~(1 << PC2);
			}
		} else {
			
		}
		dimmer(brightness);
		if (!(PIND & (1 << SET_BUTTON)) || !(PIND & (1 << PLUS_BUTTON)) || !(PIND & (1 << ALM_BUTTON))) {
			if (!(PIND & (1 << SET_BUTTON))) {
				if (!blockSetButtonCounter) setButtonCounter++;
				if (setButtonCounter > 125 && !editMode && !almEditMode) {
					editMode = true;
					setButtonCounter = 0;
					blockSetButtonCounter = true;
					editIndex = 0;
				}
			}
			if (!(PIND & (1 << PLUS_BUTTON))) {
				if (!blockPlusButtonCounter) plusButtonCounter++;
				if (plusButtonCounter > 125 && !editMode && !almEditMode) {
					settingsMode = true;
					plusButtonCounter = 0;
					blockPlusButtonCounter = true;
					editIndex = 0;
				}
				if (plusButtonCounter > 10 && editMode) {
					incrementCurrentIndex();
					
					plusButtonCounter = 0;
				}
				if (plusButtonCounter > 10 && almEditMode) {
					incrementAlmCurrentIndex();
					
					plusButtonCounter = 0;
				}
			}
			if (!(PIND & (1 << ALM_BUTTON))) {
				if (!blockAlmButtonCounter) almButtonCounter++;
				if (almButtonCounter > 125 && !editMode && !almEditMode && !alarmTriggered) {
					almEditMode = true;
					almButtonCounter = 0;
					blockAlmButtonCounter = true;
					getAlarmTime();
					editIndex = 0;
				}	
			}
		} else {
			if (setButtonCounter > 10) {
				if (!editMode && !settingsMode) toggleMode();
				else increaseEditIndex();
			}
			if (almButtonCounter > 10) {
				if (!almEditMode && !alarmTriggered) toggleAlarm();
				else if (!almEditMode && alarmTriggered) dismissAlarm();
				else increaseEditIndex();
			}
			setButtonCounter = 0;
			plusButtonCounter = 0;
			almButtonCounter = 0;
			blockSetButtonCounter = false;
			blockAlmButtonCounter = false;
			blockPlusButtonCounter = false;
		}
		if (mainCounter%3 == 0){
			getDataToDisplay();
			if (!alarmTriggered) getBrigthness();
		}
		displayChars(displayString);
		displayLed();
		mainCounter++;
		if (mainCounter == 251) mainCounter = 0;
		
    }
}