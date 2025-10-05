/*
 * RTC.c
 *
 * Created: 18.04.2023 19:34:00
 *  Author: marci
 */ 
#define F_CPU 4000000			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>
#include <avr/interrupt.h>
#include "RTC.h"
#include "TWI_Master.h"


void Initialise_TWI_Master(void) {
	TWI_Master_Initialise();
	sei();
}

unsigned char RTCRead(char addr) {
	unsigned char messageBuf[4], response;
	messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // The first byte must always consists of General Call code or the TWI slave address.
	messageBuf[1] = addr;             // Send register address
	response = TWI_Start_Transceiver_With_Data( messageBuf, 2 );
	if (response != 0 ) return response;
	messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT); // The first byte must always consists of General Call code or the TWI slave address.
	response = TWI_Start_Transceiver_With_Data( messageBuf, 2 );
	if (response != 0 ) return response;
	TWI_Get_Data_From_Transceiver(messageBuf, 2);
	//if (response != 0 ) return response;
	return messageBuf[1];
}

char RTCWrite(char addr, char value) {
	unsigned char messageBuf[3], response;
	messageBuf[0] = (TWI_targetSlaveAddress<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // The first byte must always consists of General Call code or the TWI slave address.
	messageBuf[1] = addr;             // Register address
	messageBuf[2] = value; // data to send
	response = TWI_Start_Transceiver_With_Data( messageBuf, 3 );
	if (response != 0 ) return response;
	return 0;
}

unsigned char GetSeconds(void) {
	
	char RTCseconds = RTCRead(SECONDS_REGISTER_ADDR);
	return RTCseconds;
}

unsigned char GetMinutes(void) {
	char RTCminutes = RTCRead(MINUTES_REGISTER_ADDR);
	return RTCminutes;
}

unsigned char GetHours(void) {
	char RTChours = RTCRead(HOURS_REGISTER_ADDR);
	return RTChours;
}

unsigned char GetWeekDay(void) {
	return RTCRead(WEEKDAYS_REGISTER_ADDR);
}

unsigned char GetMonthDay(void) {
	return RTCRead(MONTHDAYS_REGISTER_ADDR);
}

unsigned char GetMonth(void) {
	return RTCRead(MONTHS_REGISTER_ADDR);
}

unsigned char GetYear(void) {
	return RTCRead(YEARS_REGISTER_ADDR);
}

unsigned char GetTempUpper(void) {
	return RTCRead(TEMP_REGISTER_UPPER_ADDR);
}

unsigned char GetTempLower(void) {
	return RTCRead(TEMP_REGISTER_LOWER_ADDR);
}

char SendHours(char value) {
	return RTCWrite(HOURS_REGISTER_ADDR, value);
}

char SendMinutes(char value) {
	return RTCWrite(MINUTES_REGISTER_ADDR, value);
}

char SendSeconds(char value) {
	return RTCWrite(SECONDS_REGISTER_ADDR, value);
}

char SendWeekDay(char value) {
	return RTCWrite(WEEKDAYS_REGISTER_ADDR, value);
}

char SendMonthDay(char value) {
	return RTCWrite(MONTHDAYS_REGISTER_ADDR, value);
}

char SendMonth(char value) {
	return RTCWrite(MONTHS_REGISTER_ADDR, value);
}

char SendYear(char value) {
	return RTCWrite(YEARS_REGISTER_ADDR, value);
}