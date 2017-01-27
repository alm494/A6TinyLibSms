// This example replies with 'Hello!' message to any incoming messages

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
