// This example demonstrates how to display all incoming SMS messages
// on the standard HD44780 LCD 1602 keypad shield
// Pins 2 and 3 are used for serial communication with A6 board
// Pin 11 used to power recyle A6 board
// Pins 4,5,6,7,8,9,10 used by LCD shield

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <A6TinyLibSms.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7 );
A6TinyLibSms A6M(2, 3);

void setup() {
	lcd.begin(16, 2);
	lcd.print(F("STARTING..."));
	uint8_t init_code = A6M.init(11); 
	lcd.clear();
	if ((init_code & RES_ERROR) == RES_ERROR) {
		lcd.print(F("INIT ERROR"));
	} else {
		lcd.print(F("READY"));
	}
	lcd.setCursor(0, 1);
	lcd.print(F("FREE RAM:"));
	lcd.print(A6M.freeRAM());
}

void onSmsReceived(char *callerId, char *message) {
	lcd.clear();
	lcd.print(String(callerId));
	lcd.setCursor(0, 1);
	lcd.print(String(message));
}

void loop() {
	A6M.checkIncomingSms(&onSmsReceived);
}
