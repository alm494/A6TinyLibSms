// A6TinyLibSms
// Copyright (C) 2017 Almaz Sharipov (alm494/at/gmail/d0t/com)
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
// This program is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <A6TinyLibSms.h>
#ifdef digitalWriteFastLib
#include <digitalWriteFast.h>
#endif
#ifdef FreeRTOS
#include <Arduino_FreeRTOS.h>
#endif

#if UART_LIB==0
#include <SoftwareSerial.h>
A6TinyLibSms::A6TinyLibSms(int receivePin, int transmitPin) {
	UART = new SoftwareSerial(receivePin, transmitPin);
	UART->setTimeout(100);
}
#elif UART_LIB==1
#include <AltSoftSerial.h>
A6TinyLibSms::A6TinyLibSms() {
	UART = new AltSoftSerial();
	UART->setTimeout(100);
}
#elif  UART_LIB==2
A6TinyLibSms::A6TinyLibSms() {
	UART.setTimeout(100);
}
#endif

// ================ Public members =============================================

// Init and startup sequence for A6 board
void A6TinyLibSms::init(int powerPin) {
	// Power cycling A6 board. Follow this checklist:
	// -> 'VCC' pin on A6 board should be connected to the power via any
	//    appropriated MOSFET relay;
	// -> 'PWR_KEY' pin on A6 board should be connected to its 'VCC';
	// -> 'powerPin' on Arduino should be connected to the MOSFET relay's
	//    driving point.
	// -> If your relay has an inversed driver then you may change 'HIGH' and
 	//    'LOW' values in the sequence below:
 	logln(F("PWR CYCLING"));
	#ifdef digitalWriteFastLib
	pinModeFast(powerPin, OUTPUT);
	digitalWriteFast(powerPin, LOW);
	delay(2000);
	digitalWriteFast(powerPin, HIGH);
	#else
	pinMode(powerPin, OUTPUT);
	digitalWrite(powerPin, LOW);
	delay(2000);
	digitalWrite(powerPin, HIGH);
	#endif

	#if UART_LIB==2
	UART.begin(A6_UART_SPEED);
	#else
	UART->begin(A6_UART_SPEED);
	#endif
	log(F("WAITING SERIAL "));
	// Default UART speed is 115200 on A6 boards; but if you flood it with 'AT'
	// commands the board will adjust its connection speed automatically:
	while (1) {
		log(F("#"));
		#if UART_LIB==2
		UART.flush();
		#else
		UART->flush();
		#endif
		writeAtCommand(NULL, 0);
		readAtResponse();
		if (strstr(msg, "OK") != NULL) {
			logln();
			break;
		}
	}
	logln(F("SERIAL OK"));
	// waiting for startup finished
	logln(F("DELAY 10s"));
	delay(10000);
	// clears startup messages
	serialFlush();
	logln(F("MODEM INIT"));
	// Factory reset
	writeAtCommand(_at_reset, 0);
	readAtResponse();
	logln(msg);
	// Caler ID on (do we really need this?)
	writeAtCommand(_at_clip, 0);
	readAtResponse();
	logln(msg);
	// SMS text mode (not PDU)
	writeAtCommand(_at_cmgf, 0);
	readAtResponse();
	logln(msg);
	// SMS indicator off, no logging for all incoming messages
	writeAtCommand(_at_cnmi, 0);
	readAtResponse();
	logln(msg);
	// Set the SMS storage in device's memory. New incoming message will be stored in the
	// first free memory slot. Indexes are beginning from 1.
	writeAtCommand(_at_cmms, 0);
	readAtResponse();
	logln(msg);
	// set charset (do we really need this?)
	writeAtCommand(_at_cscs, 0);
	readAtResponse();
	logln(msg);
	// Echo off
	writeAtCommand(_at_e0, 0);
	readAtResponse();
	logln(msg);
	// Clear stored messages
	clearMemory();
	logln(F("READY!"));
}

// Reads SMS content into public variables 'msg' and 'phone'
uint8_t A6TinyLibSms::readSMS(uint8_t index) {
	serialFlush();
	bufferOverflow();
	writeAtCommand(_at_cmgr, index);
	delay(A6_CMD_TIMEOUT);
	uint8_t ch;
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t k = 0;
	uint8_t field = FIELD_BARGAIN;
	if (bufferOverflow()) {
		log(F("BUFFER OVERFLOW! ID="));
		logln(index);
	}
	#if UART_LIB==2
	while (UART.available()) {
		ch = UART.read();
	#else
	while (UART->available()) {
		ch = UART->read();
	#endif
		// ignore non-ASCII symbols:
		if ((ch < 32) || (ch > 127)) continue;
		// check for " occurence
        	if(ch == 34) i++;

		if((field == FIELD_BARGAIN) && (i == 3)) {
			field = FIELD_CALLERID;
			continue;
		}
		if ((field == FIELD_CALLERID) && (i == 4)) {
			field = FIELD_BARGAIN;
			continue;
		}
		if ((field == FIELD_BARGAIN) && (i == 6)) {
			field = FIELD_SMS_BODY;
			continue;
		}

		if ((field == FIELD_CALLERID) && (j < PHONE_MAX_LENGTH - 1)) {
			phone[j] = char(ch);
			j++;
		}
		if ((field == FIELD_SMS_BODY) && (k < SMS_MAX_LENGTH - 1)) {
			msg[k] = char(ch);
			k++;
		}
	} // while

	phone[j] = '\0';
	msg[k] = '\0';
	// Remove trailing 'OK':
	if(strlen(msg) > 2) {
		msg[strlen(msg) - 2] = '\0';
	}

	if (strlen(msg) > 0) {
		log(F("SMS READ ID="));
		logln(index);
		logln(phone);
		logln(msg);
		deleteSMS(index);
		return SMS_OK;
	} else {
		return SMS_ERROR;
	}
}

// Use this to sends SMS
void A6TinyLibSms::sendSMS(char *phoneNumber, char *message) {
	log(F("SMS SENT TO="));
	logln(phoneNumber);
	logln(message);
	char cmd[24];
	strcpy_P(cmd, _at_cmgs);
	strcatc(cmd, '\"');
	strcat(cmd, phoneNumber);
	strcatc(cmd, '\"');
	serialFlush();
	writeAtCommand(cmd);
	delay(A6_CMD_TIMEOUT);
	readAtResponse();
	strcatc(message, char(0x1a));
	writeString(message);
	delay(10000);
	readAtResponse();
	logln(msg);
	bufferOverflow();
}

// Deletes SMS on A6 board
void A6TinyLibSms::deleteSMS(uint8_t index) {
	serialFlush();
	writeAtCommand(_at_cmgd, index);
	serialFlush();
	log(F("SMS DELETED, ID="));
	logln(index);
}

// Clears SMS storage on A6 board
void A6TinyLibSms::clearMemory() {
	for(uint8_t i = 1; i <= SMS_MEMORY_SIZE; i++) {
		deleteSMS(i);
	}
	logln(F("MEMORY CLEARED"));
}

// Checks for incoming SMS messages and fires a callback if something was found
void A6TinyLibSms::checkIncomingSms(onSmsReceivedEvent *callBackFunction) {
	for(uint8_t i = 1; i <= SMS_MEMORY_SIZE; i++) {
		if (readSMS(i) == SMS_OK) {
			callBackFunction(phone, msg);
			yield();
		}
	}
}

// =============== Private members =============================================

// Used to send general AT commands, reads a strings argument from PROGMEM
void A6TinyLibSms::writeAtCommand(const char *cmd, uint8_t intarg) {
	writePString(_at);
	if (cmd != NULL) writePString(cmd);
	#if UART_LIB==2
	if (intarg != 0) UART.print(intarg);
	UART.write('\r');
	#else
	if (intarg != 0) UART->print(intarg);
	UART->write('\r');
	#endif
	delay(A6_CMD_TIMEOUT);
}

// Used to send some complex AT commands, reads a strings argument from a RAM variable
void A6TinyLibSms::writeAtCommand(char *cmd) {
	writePString(_at);
	if (cmd != NULL) writeString(cmd);
	#if UART_LIB==2
	UART.write('\r');
	#else
	UART->write('\r');
	#endif
	delay(A6_CMD_TIMEOUT);
}

// Writes a string from PROGMEM
void A6TinyLibSms::writePString(const char *str) {
	for(uint8_t i = 0; i < strlen_P(str); i++) {
		#if UART_LIB==2
		UART.write(pgm_read_byte_near(str + i));
		#else
		UART->write(pgm_read_byte_near(str + i));
		#endif
	}
}

// Writes a string from RAM
void A6TinyLibSms::writeString(char *str) {
	for(uint8_t i = 0; i < strlen(str); i++) {
		#if UART_LIB==2
		UART.write(str[i]);
		#else
		UART->write(str[i]);
		#endif
	}
}

// Reads AT command response into public 'msg' variable
void A6TinyLibSms::readAtResponse() {
	delay(A6_CMD_TIMEOUT);
	uint8_t i = 0;
	#if UART_LIB==2
	while (UART.available() && i < SMS_MAX_LENGTH - 1) {
		msg[i] = UART.read();
		i++;
	}
	#else
	while (UART->available() && i < SMS_MAX_LENGTH - 1) {
		msg[i] = UART->read();
		i++;
	}
	#endif
	msg[i] = '\0';
}

// Clears all unread bytes from UART buffer
void A6TinyLibSms::serialFlush() {
	delay(A6_CMD_TIMEOUT);
	#if UART_LIB==2
	while (UART.available()) {
		UART.read();
	}
	#else
	while (UART->available()) {
		UART->read();
	}
	#endif
}

void A6TinyLibSms::strcatc(char *str, char c) {
	uint8_t l = strlen(str);
	str[l] = c;
	str[l + 1] = '\0';
}

bool A6TinyLibSms::bufferOverflow() {
	#if UART_LIB==2
	return false;
	#else
	return UART->overflow();
	#endif
}

int A6TinyLibSms::freeRAM() {
	extern int __heap_start, *__brkval;
  	int v;
  	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
	
