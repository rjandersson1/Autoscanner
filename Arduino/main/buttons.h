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

	// Timestamp variables [ms]
	unsigned long pressedTime = 0;		// timestamp when pressed
	unsigned long releasedTime = 0;		// timestamp when released
	unsigned long changedTime = 0;		// timetsamp when state changed
	// unsigned long doubleClickTime = 0;
	unsigned long lastPressedTime = 0;	// timestamp when last pressed
	unsigned long lastReleasedTime = 0;	// timestamp when last released
	unsigned long lastChangedTime = 0;	// timestamp when last state changed
	// unsigned long lastDoubleClickTime = 0;
	
	// Timing variables [ms]
	unsigned long holdTime = 1000;			// hold time
	unsigned long debounceDelay = 250; 	// debounce timer
	unsigned long doubleClickDelay = 300;
	
	// Boolean Properties
	bool isPressed = 0;	
	bool isHeld = 0;
	bool isDoubleClick = 0;


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
	void onDoubleClick(void (*callback()));


	// Methods
	void read();

    // Prints current value
    void print();
private:
	// Callback function pointers
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
