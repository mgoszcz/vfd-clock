/*
 * RTC.h
 *
 * Created: 18.04.2023 19:34:43
 *  Author: marci
 */ 


#define SECONDS_REGISTER_ADDR 0x00
#define MINUTES_REGISTER_ADDR 0x01
#define HOURS_REGISTER_ADDR 0x02
#define WEEKDAYS_REGISTER_ADDR 0x03
#define MONTHDAYS_REGISTER_ADDR 0x04
#define MONTHS_REGISTER_ADDR 0x05
#define YEARS_REGISTER_ADDR 0x06
#define CONTROL_REGISTER_ADDR 0x0E
#define TEMP_REGISTER_UPPER_ADDR 0x11
#define TEMP_REGISTER_LOWER_ADDR 0x12
#define TWI_targetSlaveAddress 0x68
#define ALARM_1_SECONDS 0x07
#define ALARM_1_MINUTES 0x08
#define ALARM_1_HOURS 0x09

void Initialise_TWI_Master(void);
unsigned char GetSeconds(void);
unsigned char GetMinutes(void);
unsigned char GetHours(void);
unsigned char GetWeekDay(void);
unsigned char GetMonthDay(void);
unsigned char GetMonth(void);
unsigned char GetYear(void);
unsigned char GetTempUpper(void);
unsigned char GetTempLower(void);
unsigned char getAlarm1Seconds(void);
unsigned char getAlarm1Minutes(void);
unsigned char getAlarm1Hours(void);
char SendHours(char value);
char SendMinutes(char value);
char SendSeconds(char value);
char SendWeekDay(char value);
char SendMonthDay(char value);
char SendMonth(char value);
char SendYear(char value);
char sendAlarm1Seconds(char value);
char sendAlarm1Minutes(char value);
char sendAlarm1Hours(char value);