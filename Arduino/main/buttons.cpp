#include "buttons.h"
#include <Arduino.h>

// Poti Class Implementation
void Poti::setupPoti(int pin) {
	_potiPin = pin;
	// No need to configure analog pins explicitly
}

int Poti::getValue() {
	return analogRead(_potiPin);
}

void Poti::print() {
    Serial.println(this->getValue());
}

void Poti::printPct() {
    int val = this->getValue();
    float pct = map(val, 0, 1023, 0, 100);
    Serial.println(pct);
}

/////////////////////////////////////////////////////////////////////////////
// Button Class Implementation

//Constructor
Button::Button(int pin) 
	: pin(pin)
{
	setup(pin);
}

void Button::setup(int pin) {
	pinMode(pin, INPUT_PULLUP);
}

void Button::read() {
	// Read current state
	int reading = digitalRead(pin);

	// Get timestamp
	unsigned long currentTime = millis();

	// If state has changed and previous changed time less than debounce update properties
	if (reading != currentState && currentTime - changedTime > debounceDelay) {
		// Update timestamps
		lastChangedTime = changedTime;
		changedTime = currentTime;

		// Update states
		lastState = currentState;
		currentState = reading;
		isHeld = 0;

		// Check if was released
		if (currentState == HIGH) {
			isPressed = 0;

			// Update release times
			lastReleasedTime = releasedTime;
			releasedTime = currentTime;

			// Do defined function
			if (onReleaseCallback) onReleaseCallback();
		}

		// Check if was pressed
		if (currentState == LOW) {
			isPressed = 1;

			// Update pressed times
			lastPressedTime = pressedTime;
			pressedTime = currentTime;
			
			// Do defined function
			if (onPressCallback) onPressCallback();
		}

		// Check for double click
		if (pressedTime - lastPressedTime < doubleClickDelay) {
			isDoubleClick = 1;

			// Do defined function
			if (onDoubleClickCallback) onDoubleClickCallback();
		} else {
			isDoubleClick = 0;
		}	
	} 
	
	// If reading unchanged, check for hold state
	else if (reading == currentState) {
		// Button not pressed (realistically this will never be true)
		// if (reading == HIGH && isPressed) {
		// 	isPressed = 0;
		// 	isHeld = 0;
		// } 

		// Button still pressed
		if (reading == LOW) {
			// isPressed = 1;
			if (currentTime - pressedTime > holdTime) {
				if (!isHeld) {
					isHeld = 1;
					
					// Do defined function
					if (onPressCallback) onPressCallback();
				}
			}
		}
	}
	
}


void Button::print() {
    Serial.println(digitalRead(pin));
}


// Callback function definition
void Button::onPress(void (*callback)()) {
    onPressCallback = callback; // Store the user-defined function pointer
}

void Button::onRelease(void (*callback)()) {
    onReleaseCallback = callback; // Store the user-defined function pointer
}

void Button::onHold(void (*callback)()) {
    onHoldCallback = callback; // Store the user-defined function pointer
}

void Button::onDoubleClick(void (*callback)()) {
    onDoubleClickCallback = callback; // Store the user-defined function pointer
}




