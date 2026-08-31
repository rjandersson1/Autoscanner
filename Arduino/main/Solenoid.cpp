#include "Solenoid.h"
#include <Arduino.h>

Solenoid::Solenoid(int pin)
	: pin(pin)
{
	setup(pin);
}

void Solenoid::setup(int pin) {
	pinMode(pin, OUTPUT);
	analogWrite(pin, 0);
	engaged = false;
}

// Pull in at full duty, then settle to hold duty
void Solenoid::engage() {
	analogWrite(pin, kickDuty);
	delay(kickTime);
	analogWrite(pin, holdDuty);
	engaged = true;
}

void Solenoid::release() {
	analogWrite(pin, 0);
	engaged = false;
}

// Timed pulse: kick, hold for the remainder of onTime, release
void Solenoid::fire(unsigned long onTime) {
	engage();
	if (onTime > kickTime) delay(onTime - kickTime);
	release();
}
