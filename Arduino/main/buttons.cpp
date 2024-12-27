#include "buttons.h"
#include <Arduino.h>

// Constructor
Poti::Poti(int pin, int min_value = 0, int max_value = 1023) 
	: pin(pin),
	min_value(min_value),
	max_value(max_value)
{
	setup();
}

float Poti::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Poti Class Implementation
void Poti::setup() {
	// nothing to do
}

// Reads current value
void Poti::read() {
	currentValue = analogRead(pin);
	if (currentValue > max_value) currentValue = max_value;
	if (currentValue < min_value) currentValue = min_value;
}

// Returns analog value
int Poti::getAnalog() {
	return currentValue;
}

// Returns mapped percent [0.0, 100.0]
float Poti::getPercent() {
	float percent = mapFloat(currentValue, min_value, max_value, 0.0, 100.0);
	return percent;
}

// Returns mapped float [0.0, 1.0]
float Poti::getFloat() {
	float value = mapFloat(currentValue, min_value, max_value, 0.0, 1.0);
	return value;
}

// Returns custom mapping
float Poti::getMap() {
	float value = mapFloat(currentValue, min_value, max_value, mapMin, mapMax);
	return value;
}

// Returns degrees [0.0, 360.0]
float Poti::getDegrees() {
	float value = mapFloat(currentValue, min_value, max_value, 0.0, 360.0);
	return value;
}

// Sets custom mapping
void Poti::setMap(float inputMin, float inputMax) {
	mapMin = inputMin;
	mapMax = inputMax;
}

// Set thresholds for custom trigger events
void Poti::setThresholds(float thresholdMinFloat, float thresholdMaxFloat) {
	thresholdMin = map(thresholdMinFloat, 0.0, 1.0, 0, 1023);
	thresholdMax = map(thresholdMaxFloat, 0.0, 1.0, 0, 1023);
}

//




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

	// If multiclick is not defined, set multiClickDelay to zero for fastest response
	bool isMulticlick = (onDoubleClickCallback || onDoubleClickHoldCallback || onTripleClickCallback || onTripleClickHoldCallback); // check if any of the multiclick callbacks are defined 
	if (!isMulticlick) multiClickDelay = 0; // set delay to zero
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

void Button::enableMultiClick(bool enabled) {
	if (enabled) {
		multiClickDelay = 250;
	}

	if (!enabled) {
		multiClickDelay = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Constructor
toggleButton::toggleButton(int pin) 
	: pin(pin)
{
	setup(pin);
}


void toggleButton::setup(int pin) {
	pinMode(pin, INPUT_PULLUP);
}

void toggleButton::read() {
	int reading = digitalRead(pin);

	currentTime = millis();
	
	// Toggle button if pressed and debounce delay has passed
	if (!reading && currentTime - lastToggle > debounceDelay) {
		state = !state;
		lastToggle = currentTime;
		if (onToggleCallback) onToggleCallback();

		if (state && toggledOnCallback) toggledOnCallback();
		if (!state && toggledOffCallback) toggledOffCallback();
	}

	if (state && whileOnCallback) whileOnCallback();
	if (state && whileOffCallback) whileOffCallback();
}

// Callback for when button is toggled
void toggleButton::onToggle(void (*callback())) {
	onToggleCallback = callback;
}

// Callback for when button is toggled ON
void toggleButton::whileOn(void (*callback())) {
	whileOnCallback = callback;
}

void toggleButton::toggledOn(void (*callback())) {
	toggledOnCallback = callback;
}

void toggleButton::toggledOff(void (*callback())) {
	toggledOffCallback = callback;
}