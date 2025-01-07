#ifndef BUTTONS_H
#define BUTTONS_H

class Poti {
// Feature ides
// x Value Mapping: Map the raw analog value to a specific range (e.g., 0-1, 0-100, custom min/max).
// - Smoothing: Implement a simple moving average or exponential smoothing to reduce noise in the readings.
// - Threshold Detection: Add methods to check if the value crosses a specific threshold or is within a certain range.
// - Change Detection: Add a method to detect significant changes since the last read (e.g., greater than a specified delta).
// - Calibration: Allow setting and storing calibration values for min and max.
// - Non-linear Mapping: Implement custom mapping functions like exponential or logarithmic scaling.
// - Value History: Store a history of recent readings for analysis.
// - Interrupt Support: Include a feature to trigger an event or callback when the value changes significantly.
// - Debugging Information: Provide a printDebug() method to output the current status and settings via serial.
// - add a current direction property
// - add a current velocity property
public:
	// Constructor
	Poti(int pin, int min_value = 0, int max_value = 1023);

	// Proprties
	int pin;
	int min_value = 0;
	int max_value = 1023;
	int currentValue;
	int thresholdMin = min_value; // Analog 0-1023
	int thresholdMax = max_value; // Analog 0-1023
	float mapMin = min_value;
	float mapMax = max_value;


	// Utility functions
	float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
	void setup(); // Sets up poti
	int getAnalog(); // Value 0 to 1023
	float getPercent(); // Value 0.0% to 100.0%
	float getFloat(); // Value 0.00 to 1.00
	float getMap(); // Value from custom map
	float getDegrees();

	// Methods
	void read();
	void setMap(float inputMin, float inputMax); // Setup a custom mapping
	void setThresholds(float thresholdMinFloat, float thresholdMaxFloat); // Setup thresholds

private:
};

class Button {
public:
	// Constructor
	Button(int pin);
    

	// Properties
	int pin;
	int currentState = 1;
	int lastState = 1;
	unsigned long clickCount = 0;

	// Timestamp variables [ms]
	unsigned long currentTime = 0;
	unsigned long pressedTime = 0;		// timestamp when pressed
	unsigned long releasedTime = 0;		// timestamp when released
	unsigned long changedTime = 0;		// timetsamp when state changed
	unsigned long lastPressedTime = 0;	// timestamp when last pressed
	unsigned long lastReleasedTime = 0;	// timestamp when last released
	unsigned long lastChangedTime = 0;	// timestamp when last state changed
	unsigned long firstClickTime = 0;
	unsigned long lastClickTime = 0;
	
	// Timing variables [ms]
	unsigned long holdTime = 2000;			// hold time
	unsigned long debounceDelay = 50; 	// debounce timer
	unsigned long multiClickDelay = 250;
	unsigned long inactivityTimeout = 10000; // Reset states after 10s of inactivity
	
	// Boolean Properties
	bool isPressed = 0;	
	bool isHeld = 0;
	bool isSingleClick = 0;
	bool isSingleClickHold = 0;
	bool isDoubleClick = 0;
	bool isDoubleClickHold = 0;
	bool isTripleClick = 0;
	bool isTripleClickHold = 0;


	// Setup functions
	void setup(int pin);
	void setDebounce(unsigned long debounceDelay);
	void setHoldTime(unsigned long holdTime);

	// Utility functions
	void getState();
	void enableMultiClick(bool enabled);

	// Callback functions
	void onPress(void (*callback)());
	void onRelease(void (*callback)());
	// void onHold(void (*callback)());
	void onHoldStart(void (*callback)());
	void onWhileHeld(void (*callback)());
	void onSingleClick(void (*callback)());
	void onDoubleClick(void (*callback)());
	void onTripleClick(void (*callback)());


	// Methods
	void read();
	void processChangeEvent(int reading);
	void processPressEvent();
	void processReleaseEvent();
	void processHoldEvent();
	void processClickSequence();
	void checkInactivity();
	void resetButtonState();

	// Prints current value
	void print();
private:

	// Event callbacks
	void (*onPressCallback)() = nullptr;
	void (*onReleaseCallback)() = nullptr;
	// void (*onHoldCallback)() = nullptr;
	void (*onHoldStartCallback)() = nullptr;
	void (*onWhileHeldCallback)() = nullptr;

	// Click callbacks
	void (*onSingleClickCallback)() = nullptr;
	void (*onSingleClickHoldCallback)() = nullptr;
	void (*onDoubleClickCallback)() = nullptr;
	void (*onDoubleClickHoldCallback)() = nullptr;
	void (*onTripleClickCallback)() = nullptr;
	void (*onTripleClickHoldCallback)() = nullptr;
};


class toggleButton {
public:
	// Constructor
	toggleButton(int pin);

	// Properties
	int pin;
	bool state = 0;

	// Timing
	unsigned long currentTime;
	unsigned long lastToggle = 0;
	unsigned long debounceDelay = 250;

	// Utility functions

	// Methods
	void read();
	void setup(int pin);

	// Callbacks
	void onToggle(void (*callback()));
	void (*onToggleCallback)() = nullptr;
	
	void whileOn(void (*callback()));
	void (*whileOnCallback)() = nullptr;

	void whileOff(void (*callback()));
	void (*whileOffCallback)() = nullptr;

	void toggledOn(void (*callback()));
	void (*toggledOnCallback)() = nullptr;
	
	void toggledOff(void (*callback()));
	void (*toggledOffCallback)() = nullptr;

private:
};

class Timer {
    private:
        unsigned long startTimeMicros;
        unsigned long startTimeMillis;

    public:
        // Starts the timer using micros()
        void startMicros();

        // Ends the timer, calculates dT in microseconds, and prints it
        void endMicros();

        // Starts the timer using millis()
        void startMillis();

        // Ends the timer, calculates dT in milliseconds, and prints it
        void endMillis();
};


#endif // BUTTONS_H
