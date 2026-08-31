#pragma once

// Kick-and-hold PWM driver for a solenoid.
// Pulls in at full duty, then drops to a lower hold duty to limit heat and current.
class Solenoid {
public:
	Solenoid(int pin);

	int pin;

	// PWM duty [0-255]
	int kickDuty = 255;
	int holdDuty = 80;

	// Kick duration [ms]
	unsigned long kickTime = 120;

	bool engaged = false;

	void setup(int pin);
	void engage();                   // kick, then hold. blocks for kickTime
	void release();                  // drop to zero
	void fire(unsigned long onTime); // engage, hold for onTime, release

private:
};
