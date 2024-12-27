#include "stepper.h"
#include <Arduino.h>

// Define Functions here


// Constructor implementation
Stepper::Stepper(int stepPin, int dirPin, int enablePin, int ms1Pin, int ms2Pin, int ms3Pin, int resetPin, int sleepPin, int stepsPerRevolution)
    : stepPin(stepPin),
      dirPin(dirPin),
      enablePin(enablePin),
      ms1Pin(ms1Pin),
      ms2Pin(ms2Pin),
      ms3Pin(ms3Pin),
      resetPin(resetPin),
      sleepPin(sleepPin),
      fullStepsPerRevolution(stepsPerRevolution),
      microstepMode(1),
      isEnabled(false),
      direction(1),
      currentStep(0)
{
    setup();
}

void Stepper::setup() {
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    pinMode(ms1Pin, OUTPUT);
    pinMode(ms2Pin, OUTPUT);
    pinMode(ms3Pin, OUTPUT);
    pinMode(resetPin, OUTPUT);
    pinMode(sleepPin, OUTPUT);
    digitalWrite(enablePin, HIGH); // Disable driver
    digitalWrite(resetPin, HIGH); // Ensure reset is not active
    digitalWrite(sleepPin, HIGH); // Keep awake

    stepsPerRevolution = fullStepsPerRevolution * microstepMode;
    stepAngle = 360.0f / stepsPerRevolution;
}

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
        default: // Default to full step if invalid mode
            microstepMode = 1;
            digitalWrite(ms1Pin, LOW);
            digitalWrite(ms2Pin, LOW);
            digitalWrite(ms3Pin, LOW);
            break;
    }

    // Use floating-point arithmetic for calculations
    stepsPerRevolution = microstepMode * fullStepsPerRevolution;
    degreesPerStep = 360.0 / float(stepsPerRevolution);
    stepsPerDegree = 1.0 / degreesPerStep;
    Serial.print("[X] "); Serial.println(microstepMode);
}

void Stepper::cycleMicrostepMode() {
    if (microstepMode == 1) setMicrostepMode(2);
    else if (microstepMode == 2) setMicrostepMode(4);
    else if (microstepMode == 4) setMicrostepMode(8);
    else if (microstepMode == 8) setMicrostepMode(16);
    else if (microstepMode == 16) setMicrostepMode(1);
}

void Stepper::swapDirection() {
    direction = !direction;
    setDirection();
}

// Enable the motor driver
void Stepper::enableMotor() {
    digitalWrite(enablePin, LOW); // Active LOW
    isEnabled = true;
}

// Disable the motor driver
void Stepper::disableMotor() {
    digitalWrite(enablePin, HIGH); // Active LOW
    isEnabled = false;
}

// Set the direction of rotation
void Stepper::setDirection() {
    digitalWrite(dirPin, direction);
}

void Stepper::setTargetDirection(int target) {
    if (currentStep < target) direction = 1;
    if (currentStep > target) direction = -1;
    if (currentStep == target) direction = 0;
    setDirection();
}

// Reset the motor driver
void Stepper::resetMotor() {
    digitalWrite(resetPin, LOW);  // Trigger reset
    delay(10);
    digitalWrite(resetPin, HIGH); // Release reset
}

// Put the motor driver to sleep
void Stepper::sleepMotor() {
    digitalWrite(sleepPin, LOW);  // Sleep mode
}

// Get the current step count
long Stepper::getCurrentStep() const {
    return currentStep;
}

// Set the current step count
void Stepper::setCurrentStep(long step) {
    currentStep = step;
}

void Stepper::zero() {
    currentStep = 0;
}

// Set motor stepDelay based on revolution per second (RPS)
void Stepper::setSpeed(float RPS) {
    if (RPS <= 0.0) {
        stepDelay = 0; // Handle invalid or zero speed
        Serial.println("Invalid RPS: Must be greater than 0.");
        return;
    }

    // Ensure stepsPerRevolution is valid
    if (stepsPerRevolution <= 0) {
        Serial.print(stepsPerRevolution);
        Serial.println("Invalid stepsPerRevolution: Must be greater than 0.");
        stepDelay = 0;
        return;
    }

    // Debugging: Print RPS and stepsPerRevolution
    Serial.print("RPS: "); Serial.println(RPS, 6);
    Serial.print("stepsPerRevolution: "); Serial.println(stepsPerRevolution);

    // Explicitly use floating-point arithmetic
    float stepsPerMicrosecond = (RPS * float(stepsPerRevolution)) / 1e6;

    // Debugging: Print stepsPerMicrosecond
    Serial.print("stepsPerMicrosecond: "); Serial.println(stepsPerMicrosecond, 6);

    if (stepsPerMicrosecond > 0.0) {
        // Calculate step delay in microseconds
        float delayPerStep = 1.0 / (2.0 * stepsPerMicrosecond); // Divide by 2 for HIGH and LOW cycle
        stepDelay = int(round(delayPerStep));

        // Debugging: Print stepDelay
        Serial.print("Calculated stepDelay (uS): "); Serial.println(stepDelay);
    } else {
        Serial.println("Calculated stepsPerMicrosecond is zero or invalid.");
        stepDelay = 0;
    }
}

// Moves a single step
void Stepper::moveStep() {
    if (!isEnabled) return;
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepDelay, LOW);
    delayMicroseconds(stepDelay);
    currentStep += direction;
}

void Stepper::step(float dt) {
    digitalWrite(stepPin, HIGH);
    delay(dt/2.0);
    digitalWrite(stepPin, LOW);
    delay(dt/2.0);
}

void Stepper::moveSteps(int steps) {
    if (!isEnabled) return;
    targetStep = currentStep + steps;
    while (targetStep != currentStep) {
        moveStep();
    }
}

void Stepper::moveToDegree(float degree) {
    // [step] = [deg]*[step/deg]
    targetStep = int(round(float(stepsPerDegree) * degree));
    setTargetDirection(targetStep);

    enableMotor();
    while (targetStep != currentStep) {
        moveStep();
    }
    disableMotor();
}

// Debugging: Print motor status
void Stepper::printStatus() {
    Serial.print("Current Step: ");
    Serial.println(currentStep);
    Serial.print("Target Step: ");
    Serial.println(targetStep);
    Serial.print("Delay [uS]: ");
    Serial.println(stepDelay);
    Serial.print("Direction: ");
    Serial.println(direction);
    Serial.print("Enabled: ");
    Serial.println(isEnabled ? "Yes" : "No");
    Serial.println();
}

// 
void Stepper::moveSmoothed(float targetAngle) {
    // x(t) = 0.5 * a * t**2 + v * t + x0
    // x(t) = 0.5at^2 + vt + x0
    float dx = degreesPerStep; // distance per step (constant) [deg]
    float dt = 0; // time step
    // float x = targetAngle; // end position [deg]
    float v = 0; // start from zero velocity [deg/s]
    float a = acceleration; // [deg/s^2]
    float startTime = millis()/1000.0; // movement start timestmap [s]
    float t = 0; // time since start [s]
    setTargetDirection(round(targetAngle*stepsPerDegree)); // Set direction

    while (currentStep != targetStep && isEnabled) { 
        t = millis() / 1000.0 - startTime; // time since begin
        v = a * t; // update current velocity
        dt = dx/v; // estimate timestep

        step(dt*1000.0); // Move single step

        currentStep += direction; // Update current position
    }
}







