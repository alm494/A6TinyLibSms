// This example replies with 'Hello!' message to any incoming SMS messages

#include <Arduino.h>
#include <A6TinyLibSms.h>

#define powerPin 5
#define serialRx 8
#define serialTx 9

// using SoftwareSerial:
A6TinyLibSms A6M(serialRx, serialTx);

// using AltSoftSerial or HardwareSerial:
//A6TinyLibSms A6M;

void setup() {
  A6M.init(powerPin);
  //A6M.init(powerPin, "1234"); // provide SIM PIN if required
}

// You shoud define a callback function here to be fired by incoming SMS:
void onSmsReceived(char *callerId, char *message) {
  char mes[] = "Hello!";
  A6M.sendSMS(callerId, mes);
}

void loop() {
  A6M.checkIncomingSms(&onSmsReceived);
}
