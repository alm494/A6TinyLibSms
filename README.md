# A6TinyLibSms
Implement your own SMS exchange protocol to use on ATmega328P based Arduino boards and AiThinker A6 GSM module.
Primary targets are Uno/Nano boards with 2K RAM so major efforts aimed at the both RAM and PROGMEM consume reducing.

## Available options
Look at and edit C++ header file to set up your project to use <b>SoftwareSerial</b>, [<b>AltSoftSerial</b>](https://github.com/PaulStoffregen/AltSoftSerial) or <b>HardwareSerial</b> to communicate with A6 board. Everything is explained there in details:

~~~
#define UART_LIB 0         // <- 'SoftwareSerial' (default)
//#define UART_LIB 1       // <- 'AltSoftSerial' (Rx->8, Tx->9, Unusable->10 on Uno)
//#define UART_LIB 2       // <- 'HardwareSerial' (Rx->0, Tx->1 on Uno)
~~~

Also you may define [<b>digitalWriteFast</b>](https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast) usage option and [<b>FreeRTOS</b>](http://www.freertos.org/) timing there:

~~~
#define digitalWriteFastLib
#define FreeRTOS
~~~

## A6 module initialization

To initialize A6 board a power cycling method is used: pass the digital output pin number to 'init()' method.<br>
Do not even try to power up your A6 board directly from this output pin! Use any appropriated MOSFET relay instead. Your relay schematic should be designed to open MOSFET with LOW value (GND) and close with HIGH (+5V).<br>
For example, follow this checklist to use a low-side driver (p-channell MOSFET):

* 'VCC' pin on A6 board should be connected to the +5V power via MOSFET relay;
* 'PWR_KEY' pin on A6 board should be connected to its 'VCC';
* 'POWER_PIN' on Arduino should be connected to the MOSFET driving point.
* Just change 'HIGH' and 'LOW' values in the init() method if you are using different design.

## How to use
This example replies with 'Hello!' message to any incoming messages:
~~~
#include <Arduino.h>
#include <A6TinyLibSms.h>

#define POWER_PIN 5

// using SoftwareSerial:
A6TinyLibSms A6M(8, 9);

// using AltSoftSerial or HardwareSerial:
//A6TinyLibSms A6M;

void setup() {
  A6M.init(POWER_PIN);
}

// You shoud define a callback function here to be fired by incoming SMS:
void onSmsReceived(char *callerId, char *message) {
  char mes[] = "Hello!";
  A6M.sendSMS(callerId, mes);
}

void loop() {
  A6M.checkIncomingSms(&onSmsReceived);
}
~~~
## Limitations

* Use ASCII symbols only in your exchange protocol: {31 &lt; char &lt; 128}, no Cyrillic, no Chinese, no unicode!
* Modem AT-protocol is critical to RX/TX buffer length on connected UART device or software emulation. Examine A6TinyLibSms.h header file and you'll find there some tips how to solve this issue.
* Please consider using shorter messages in your exchange protocol, this helps you to decrease RAM usage.
* This project was completely created and tested in [PlatformIO IDE](https://github.com/platformio/platformio-core). It may throw exceptions if you are compiling it in different development tools like Arduino IDE or Eclipse. Or may not :-)

## Other libraries

You may also want to try this library: [A6lib by skorokithakis](https://github.com/skorokithakis/A6lib). It is more advanced but uses SoftwareSerial and I couldn't read any SMS messages with it on Nano board due to unknown overflow or limitation.
