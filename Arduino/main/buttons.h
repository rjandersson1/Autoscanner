#ifndef BUTTONS_H
#define BUTTONS_H

class Poti {
public:
	// Sets up the potentiometer
	void setupPoti(int pin);

	// Gets the raw analog value from the potentiometer
	int getValue();

	// Prints current value
    void print();
	void printPct();
private:
	int _potiPin; // Pin connected to the potentiometer
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
	// unsigned long doubleClickTime = 0;
	unsigned long lastPressedTime = 0;	// timestamp when last pressed
	unsigned long lastReleasedTime = 0;	// timestamp when last released
	unsigned long lastChangedTime = 0;	// timestamp when last state changed
	unsigned long lastClickTime = 0;
	
	// Timing variables [ms]
	unsigned long holdTime = 1000;			// hold time
	unsigned long debounceDelay = 250; 	// debounce timer
	unsigned long multiClickDelay = 500;
	unsigned long inactivityTimeout = 10000; // Reset states after 10s of inactivity
	
	// Boolean Properties
	bool isPressed = 0;	
	bool isHeld = 0;
	bool isSingleClick = 0;
	bool isSingleClickHold = 0;
	bool isDoubleClick = 0;
	bool isDoubleClickHold = 0;
	bool isTripleClick = 0;


	// Setup functions
	void setup(int pin);
	void setDebounce(unsigned long debounceDelay);
	void setHoldTime(unsigned long holdTime);

	// Utility functions
	void getState();

	// Callback functions
	void onPress(void (*callback()));
	void onRelease(void (*callback()));
	void onHold(void (*callback()));
	void onSingleClick(void (*callback()));
	void onDoubleClick(void (*callback()));
	void onTripleClick(void (*callback()));


	// Methods
	void read();
	void processChange(int reading);
	void checkPress();
	void checkRelease();
	void checkHold();
	void checkInactivity();

	// Prints current value
	void print();
private:

	// Private callbacks
	void (*onPressCallback)() = nullptr;
	void (*onReleaseCallback)() = nullptr;
	void (*onHoldCallback)() = nullptr;
	void (*onDoubleClickCallback)() = nullptr;
};


class toggleButton {
public:
	int pin;
	bool state;

private:
};

#endif // BUTTONS_H
