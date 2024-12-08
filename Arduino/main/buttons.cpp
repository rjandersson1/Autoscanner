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

// Checks if button was pressed
void Button::processPressEvent() {
	isPressed = 1;
	lastPressedTime = pressedTime;
	pressedTime = currentTime;
	
	if (onPressCallback) onPressCallback();

	// Record timestamp first click in sequence 
	if (clickCount == 0) firstClickTime = currentTime;

	// Increase click count if multiclick observed
	if (currentTime - lastPressedTime < multiClickDelay) {
		clickCount++;
	}
	// If no multiclick, set click count to 1.
	else {
		clickCount = 1;
	}

	lastClickTime = currentTime;
}

// Processes a change in state
void Button::processChangeEvent(int reading) {
	lastState = currentState;
	currentState = reading;
	lastChangedTime = changedTime;
	changedTime = currentTime;

	if (currentState == LOW) processPressEvent();
	else processReleaseEvent();

}

// Processes button release event
void Button::processReleaseEvent() {
	isPressed = 0;
	lastReleasedTime = releasedTime;
	releasedTime = currentTime;

	if (onReleaseCallback) onReleaseCallback();

	// Reset hold flags
	isHeld = 0;
	isSingleClickHold = 0;
	isDoubleClickHold = 0;
	isTripleClickHold = 0;

	// Check if multiclick
	processClickSequence();
}

// Processes multiclicks if time between clicks is more than multiclick delay.
void Button::processClickSequence() {
	if (currentTime - lastClickTime >= multiClickDelay) {
		if (clickCount == 1) {
			isSingleClick = 1;
			isDoubleClick = 0;
			isTripleClick = 0;

			// Trigger single-click callback
			if (onSingleClickCallback) onSingleClickCallback();
		} 
		else if (clickCount == 2) {
			isSingleClick = 0;
			isDoubleClick = 1;
			isTripleClick = 0;

			// Trigger double-click callback
			if (onDoubleClickCallback) onDoubleClickCallback();
		} 
		else if (clickCount == 3) {
			isSingleClick = 0;
			isDoubleClick = 0;
			isTripleClick = 1;

			// Trigger triple-click callback
			if (onTripleClickCallback) onTripleClickCallback();
		}

		// Reset click count after processing
		clickCount = 0;
		firstClickTime = 0;
	}
}

// Checks if button is held for hold time and processes cases
void Button::processHoldEvent() {
	// Ensure the button is in the pressed state and the hold time has passed
	if (currentState == LOW && currentTime - pressedTime > holdTime) {
		isHeld = 1; // Mark the button as held
		if (onHoldCallback) onHoldCallback(); // Trigger generic hold callback if defined

		// Check click count and determine the hold type
		if (clickCount == 1 && !isSingleClickHold) {
			isSingleClickHold = 1; // Single click hold detected
			if (onSingleClickHoldCallback) onSingleClickHoldCallback(); // Trigger single-click hold callback
		} 
		else if (clickCount == 2 && !isDoubleClickHold) {
			isDoubleClickHold = 1; // Double click hold detected
			if (onDoubleClickHoldCallback) onDoubleClickHoldCallback(); // Trigger double-click hold callback
		} 
		else if (clickCount == 3 && !isTripleClickHold) {
			isTripleClickHold = 1; // Triple click hold detected
			if (onTripleClickHoldCallback) onTripleClickHoldCallback(); // Trigger triple-click hold callback
		}
	}
}

// Reset button states if inactive
void Button::resetButtonState() {
	clickCount = 0;
	isSingleClick = 0;
	isSingleClickHold = 0;
	isDoubleClick = 0;
	isDoubleClickHold = 0;
	isTripleClick = 0;
	isTripleClickHold = 0;
}

// Reads and updates button states
void Button::read() {
	// Read current state
	int reading = digitalRead(pin);

	// Get timestamp
	currentTime = millis();

	// If state has changed and previous changed time less than debounce update properties
	if (reading != currentState && currentTime - changedTime > debounceDelay) {
		processChangeEvent(reading);
	} 
	// If reading unchanged, check for hold state
	else if (reading == currentState) {
		processClickSequence();
		processHoldEvent();

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

void Button::onSingleClick(void (*callback)()) {
    onSingleClickCallback = callback; // Store the user-defined function pointer
}

void Button::onDoubleClick(void (*callback)()) {
    onDoubleClickCallback = callback; // Store the user-defined function pointer
}

void Button::onTripleClick(void (*callback)()) {
    onTripleClickCallback = callback; // Store the user-defined function pointer
}





