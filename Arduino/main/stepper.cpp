#include "stepper.h"
#include <Arduino.h>

// Define Functions here


// Constructor implementation
Stepper::Stepper(int stepPin, int dirPin, int enablePin, int ms1Pin, int ms2Pin, int ms3Pin, int resetPin, int sleepPin, int stepsPerRevolution)
    // Member initializer list
    : stepPin(stepPin),
      dirPin(dirPin),
      enablePin(enablePin),
      ms1Pin(ms1Pin),
      ms2Pin(ms2Pin),
      ms3Pin(ms3Pin),
      resetPin(resetPin),
      sleepPin(sleepPin),
      microstepMode(1),
      stepsPerRevolution(stepsPerRevolution) {
    
    // Constructor body logic (if needed)
    // Set default steps per revolution if the provided value is invalid
    if (stepsPerRevolution <= 0) {
        this->stepsPerRevolution = 200;
    }

    // Configure pins as outputs
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    pinMode(ms1Pin, OUTPUT);
    pinMode(ms2Pin, OUTPUT);
    pinMode(ms3Pin, OUTPUT);

    // Default: Disable the driver
    digitalWrite(enablePin, HIGH); // Disable driver by default (active LOW)

    // Initialize RESET and SLEEP pins
    pinMode(resetPin, OUTPUT);
    pinMode(sleepPin, OUTPUT);
    digitalWrite(resetPin, HIGH);  // Keep the driver active (RESET HIGH)
    digitalWrite(sleepPin, HIGH);  // Keep the driver awake (SLEEP HIGH)

    // Initialize microstepping mode to full step
    digitalWrite(ms1Pin, LOW);
    digitalWrite(ms2Pin, LOW);
    digitalWrite(ms3Pin, LOW);
}

// Overload Constructor implementation
// Stepper::Stepper(int stepPin, int dirPin, int enablePin)
//     : stepPin(stepPin),
//       dirPin(dirPin),
//       enablePin(enablePin),
//       stepsPerRevolution(200), // Default value
//       ms1Pin(-1), ms2Pin(-1), ms3Pin(-1) { // Default invalid values for microstepping pins
// }

void Stepper::setMicrostepMode(int mode) {
    microstepMode = mode;

    // Set MS1, MS2, MS3 pin states based on mode
    switch (mode) {
        case 1: // Full step
            digitalWrite(ms1Pin, LOW);
            digitalWrite(ms2Pin, LOW);
            digitalWrite(ms3Pin, LOW);
            break;
        case 2: // Half step
            digitalWrite(ms1Pin, HIGH);
            digitalWrite(ms2Pin, LOW);
            digitalWrite(ms3Pin, LOW);
            break;
        case 4: // Quarter step
            digitalWrite(ms1Pin, LOW);
            digitalWrite(ms2Pin, HIGH);
            digitalWrite(ms3Pin, LOW);
            break;
        case 8: // Eighth step
            digitalWrite(ms1Pin, HIGH);
            digitalWrite(ms2Pin, HIGH);
            digitalWrite(ms3Pin, LOW);
            break;
        case 16: // Sixteenth step
            digitalWrite(ms1Pin, HIGH);
            digitalWrite(ms2Pin, HIGH);
            digitalWrite(ms3Pin, HIGH);
            break;
        default:
            // Default to full step if invalid mode
            microstepMode = 1;
            digitalWrite(ms1Pin, LOW);
            digitalWrite(ms2Pin, LOW);
            digitalWrite(ms3Pin, LOW);
            break;
}
}

// ================== Utility funcitons =================== //

// Get steps per revolution
int Stepper::getStepsPerRevolution() const {
    return stepsPerRevolution * microstepMode;
}

// gets current dir


// =================== Drive functions ================= //
// Move the motor to a specific percentage position (0-100%)
void Stepper::moveToPercentage(float percentage) {
	// Clamp percentage to valid range
	if (percentage < 0.0f) percentage = 0.0f;
	if (percentage > 100.0f) percentage = 100.0f;

	// Calculate the target number of steps
	int targetSteps = (stepsPerRevolution * microstepMode * percentage) / 100;

	// Enable the driver
	digitalWrite(enablePin, LOW);

	// Set direction (e.g., CW for increasing position)
	digitalWrite(dirPin, HIGH);

	// Move the motor
	for (int i = 0; i < targetSteps; ++i) {
		digitalWrite(stepPin, HIGH);
		delayMicroseconds(500); // Adjust for speed
		digitalWrite(stepPin, LOW);
		delayMicroseconds(500);
	}

	// Disable the driver
	digitalWrite(enablePin, HIGH);
}

void Stepper::moveForwardPct(float percentage) {
	// Clamp percentage to valid range
	if (percentage < 0.0f) {
        percentage = 0.0f;
        digitalWrite(enablePin, LOW);
        return;
    }
    
	if (percentage > 100.0f) percentage = 100.0f;

	// Calculate the total number of steps for the given percentage
	int stepsToMove = (stepsPerRevolution * microstepMode * percentage) / 100;

	// Enable the driver
	digitalWrite(enablePin, LOW);

	// Set direction to forward (CW)
	digitalWrite(dirPin, HIGH);

	// Move the motor at a rate determined by the percentage
	// Higher percentages correspond to faster movement
	int delayTime = map(percentage, 0, 100, 2000, 100); // Adjust delay (slower for low %, faster for high %)

	for (int i = 0; i < stepsToMove; ++i) {
		digitalWrite(stepPin, HIGH);
		delayMicroseconds(delayTime); // Speed determined by percentage
		digitalWrite(stepPin, LOW);
		delayMicroseconds(delayTime);
	}

	// Disable the driver
	digitalWrite(enablePin, HIGH);
}

void Stepper::moveStep(int stepDelay) {
	// Enable the driver
	// digitalWrite(enablePin, LOW);

	// Perform a single step
	digitalWrite(stepPin, HIGH);
	delayMicroseconds(stepDelay); // Delay for step speed
	digitalWrite(stepPin, LOW);
	delayMicroseconds(stepDelay);

	// Optionally, disable the driver after the step
	// digitalWrite(enablePin, HIGH);
}
