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

// Setup button
void Button::setup(int pin) {
	pinMode(pin, INPUT_PULLUP);
}

// Processes a change in state
void Button::processChange(int reading) {
	lastState = currentState;
	currentState = reading;
	lastChangedTime = changedTime;
	changedTime = currentTime;

	if (currentState == LOW) checkPress(currentTime);
	else checkRelease(currentTime);

}

// Checks if button was pressed
void Button::checkPress() {
	isPressed = 1;
	lastPressedTime = pressedTime;
	pressedTime = currentTime;

	if (onPressCallback) onPressCallback();

	// Increment click count for multi click check
	if (currentTime - lastClickTime < multiClickDelay) {
		clickCount++;
	} else {
		// Reset counter
		clickCount = 1;
	}
	lastClickTime = currentTime;
}

// Checks if button was released
void Button::checkRelease() {
	isPressed = 0;
	lastReleasedTime = releasedTime;
	releasedTime = currentTime;

	if (onReleaseCallback) onReleaseCallback();

	// Check if multi click delay has passed to parse multi clicks
	if (currentTime - lastClickTime >= multiClickDelay) {
		if (clickCount == 1) {
			// Single click
			isSingleClick = 1;
			isDoubleClick = 0;
			isTripleClick = 0;

			// Run single click function
			if (onSingleClickCallback) onSingleClickCallback();
		} else if (clickCount == 2) {
			if (currentState == LOW && currentTime - pressedTime > holdTime) {
				isDoubleClickHold = 1;
				if (onDoubleClickHoldCallback) onDoubleClickHoldCallback();
			} else {
				// Double CLick
				isSingleClick = 0;
				isDoubleClick = 1;
				isTripleClick = 0;

				// Run doubleclick function
				if (onDoubleClickCallback) onDoubleClickCallback();
			}
		} else if (clickCount == 3) {
			// Triple click
			isSingleClick = 0;
			isDoubleClick = 0;
			isTripleClick = 1;

			// Run triple click function
			if (onTripleClickCallback) onTripleClickCallback();
		}
	}
}

// Checks if button was held
void Button::checkHold() {
	if (currentState == LOW && currentTime - pressedTime > holdTime && !isHeld) {
		isHeld = 1;
		if (onHoldCallback) onHoldCallback();
	}
}

// Reset button states if inactive
void Button:resetButtonState() {
	isPressed = 0;
	isHeld = 0;
	isDoubleClick = 0;
	isTripleClick = 0;
	clickCount = 0;
}

// Reads and updates button states
void Button::read() {
	// Read current state
	int reading = digitalRead(pin);

	// Get timestamp
	currentTime = millis();

	// If state has changed and previous changed time less than debounce update properties
	if (reading != currentState && currentTime - changedTime > debounceDelay) {
		processChange(reading, currentTime);
	} 
	
	// If reading unchanged, check for hold state
	else if (reading == currentState) {
		checkHold(currentTime);

		// Reset button state if inactive
		if (currentTime - lastChangedTime > inactivityTimeout) {
			resetButtonState();
		}
	}
}


// Print states (debugging)
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




