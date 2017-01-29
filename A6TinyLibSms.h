// A6TinyLibSms
// Copyright (C) 2017 Almaz Sharipov (alm494/at/gmail/d0t/com)
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef A6TinyLibSms_h
#define A6TinyLibSms_h
#include <Arduino.h>
#include <avr/pgmspace.h>

// Uncomment the following line to read all debug messages in terminal console.
// Warning: this consumes both PROGMEM and RAM memory!
#define DEBUG

// Uncomment #define line to use 'SoftwareSerial', 'AltSoftSerial' or
// embedded 'HardwareSerial' to comminicate with A6 board.
// But first learn what is the difference between these libraries and which
// pins are affected!
// Warning: by selecting 'HardwareSerial' you may not use DEBUG mode because
// Uno boards which are the main target of this library have a single h/w UART
// port on pins 0 and 1. Also do not forget to disable 'reset on serial
// connection' on these boards.
#define UART_LIB 0	// <- 'SoftwareSerial' (default)
//#define UART_LIB 1    // <- 'AltSoftSerial' (Rx->8, Tx->9, Unusable->10 on Uno)
//#define UART_LIB 2	// <- 'HardwareSerial' (Rx->0, Tx->1 on Uno)

#if UART_LIB==0
#define UART swSerial
#include <SoftwareSerial.h>
// !!! ---Important warning--- !!!
// Play around with _SS_MAX_RX_BUFF 64 in 'SoftwareSerial.h'!
// Default 64 bytes might be too small to receive some answers from A6 board.
// Warning: This consumes your RAM respectively!
#elif UART_LIB==1
#define UART swSerial
#include <AltSoftSerial.h>
// !!! ---Important warning--- !!!
// Play around with RX_BUFFER_SIZE 80 in 'AltSoftSerial.cpp'!
// Default 80 bytes might be too small to receive some answers from A6 board.
// Warning: This consumes your RAM respectively!
#elif UART_LIB==2
// Nice choice for profies, you know what are you doing :-)
#define UART Serial
#undef DEBUG
#endif

// Uncomment the following line to use 'digitalWriteFast' library instead of
// common 'digitalWrite'. This consumes less memory and works faster:
//#define digitalWriteFastLib

// Uncomment the following line to use FreeRTOS delays.
// (This is experimental, not tested):
//#define FreeRTOS

// Adjust the following values carefully!

// Play around this value to achieve a stable communication:
const uint8_t A6_CMD_TIMEOUT = 200;
// It consumes RAM, you may lower this value and exchange with shorter messages.
// Do not forget adjust RX buffers accordingly:
const uint8_t SMS_MAX_LENGTH = 140;
// Do not change this:
const uint8_t PHONE_MAX_LENGTH = 14;
// You don't need to change this, really:
const uint32_t A6_UART_SPEED = 9600;
// A6 board has much more memory slots, but we'll use first 7 as a stack.
// Higher values may cause significantly slower loop. Anyway I have no idea
// how to protect it against DDOS attacks.
const uint8_t SMS_MEMORY_SIZE = 7;

// some constants to keep in PROGMEM (Arduino's flash memory):
const char _at[] PROGMEM  = "AT";
const char _at_e0[] PROGMEM  = "E0";
const char _at_reset[] PROGMEM  = "&F";
const char _at_clip[] PROGMEM  = "+CLIP=1";
const char _at_cmgf[] PROGMEM  = "+CMGF=1";
const char _at_cnmi[] PROGMEM  = "+CNMI=1,0";
const char _at_cmms[] PROGMEM  = "+CPMS=ME,ME,ME";
const char _at_cscs[] PROGMEM  = "+CSCS=GSM";
const char _at_cmgr[] PROGMEM  = "+CMGR=";
const char _at_cmgd[] PROGMEM  = "+CMGD=";
const char _at_cmgs[] PROGMEM  = "+CMGS=";
const char _at_cpin[] PROGMEM  = "+CPIN=";
const char _at_cusd[] PROGMEM  = "+CUSD=";

// c enums consume a little bit more PROGMEM, use constant macros instead:
#define FIELD_BARGAIN 0
#define FIELD_CALLERID 1
#define FIELD_SMS_BODY 2
#define RES_OK 0b00000001
#define RES_ERROR 0b00000010
#define RES_UNKNOWN 0b00000100

// Callback function declaration:
typedef void onSmsReceivedEvent (char *callerId, char *message);

#ifdef DEBUG
#define log(msg) Serial.print(msg)
#define logln(msg) Serial.println(msg)
#else
#define log(msg)
#define logln(msg)
#endif

#ifdef FreeRTOS
#define delay(s) vTaskDelay(s)
#endif

class A6TinyLibSms {
public:

	// RAM reservation for string buffers
	char msg[SMS_MAX_LENGTH];
	char phone[PHONE_MAX_LENGTH];

	#if UART_LIB==0
	A6TinyLibSms(int receivePin, int transmitPin);
	#else
	A6TinyLibSms();
	#endif

	uint8_t init(int powerPin, const char *sim_pin = NULL);
	uint8_t readSMS(uint8_t index);
	uint8_t sendSMS(char *phoneNumber, char *message);
	void deleteSMS(uint8_t index);
	void clearMemory();
	void checkIncomingSms(onSmsReceivedEvent *callBackFunction);
	int freeRAM();

private:

	#if UART_LIB==0
	SoftwareSerial *UART;
	#elif UART_LIB==1
	AltSoftSerial *UART;
	#endif

	void writeAtCommand(const char *cmd, uint8_t intarg);
	void writeAtCommand(char *cmd);
	void writePString(const char *str);
	void writeString(char *str);
	uint8_t readAtResponse();
	void serialFlush();
	void strcatc(char *str, char c);
	bool bufferOverflow();

};

#endif
