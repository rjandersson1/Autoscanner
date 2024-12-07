#ifndef STEPPER_H
#define STEPPER_H


/* Microstepping Configuration Table for Stepper Motor Drivers
 -----------------------------------------------------------
 MS1  | MS2  | MS3  | Microstepping Mode   | Step Size
 -----------------------------------------------------------
 LOW  | LOW  | LOW  | Full Step            | 1.8° per step
 HIGH | LOW  | LOW  | Half Step            | 0.9° per step
 LOW  | HIGH | LOW  | Quarter Step         | 0.45° per step
 HIGH | HIGH | LOW  | Eighth Step          | 0.225° per step
 HIGH | HIGH | HIGH | Sixteenth Step       | 0.1125° per step
 (For DRV8825: 1/32 stepping is possible)
*/

/*Properties:
- int stepPin          // Pin connected to the step signal
- int dirPin           // Pin connected to the direction control
- int enablePin        // Pin to enable/disable the motor driver
- int stepsPerRevolution // Number of steps for a full revolution
- float stepAngle      // Angle per step in degrees
- bool isClockwise     // Current rotation direction
- bool isEnabled       // Whether the motor is enabled
- long currentStep     // Current step position
- long targetStep      // Target step position for movement
- int stepDelay        // Delay between steps for speed control
- unsigned long lastStepTime // Timestamp of the last step
*/

/*Functions:

Initialization:
- void initialize()    // Sets up pins and default states

Motor Control:
- void enableMotor()          // Enables the motor driver
- void disableMotor()         // Disables the motor driver
- void setDirection(bool clockwise) // Sets the rotation direction
- void moveSteps(long steps)  // Moves the motor a specified number of steps
- void moveToPosition(long position) // Moves the motor to an absolute position

Speed Control:
- void setSpeedRPM(float rpm) // Configures the motor speed in RPM
- void setStepDelay(int delayMicroseconds) // Sets the delay between steps directly

Microstepping (Optional):
- void configureMicrostepping(int microstepMode) // Configures the microstepping mode (e.g., 1/2, 1/4, 1/16)

Homing:
- void doHoming(int limitSwitchPin) // Moves the motor until a limit switch is triggered

Getters and Utilities:
- long getCurrentStep()       // Returns the current step count
- float getCurrentAngle()     // Returns the current angle in degrees
- bool isMotorEnabled()       // Returns the motor’s enabled state
- void printState()           // Prints the motor’s state for debugging

Constructor:
- Stepper(int stepPin, int dirPin, int enablePin, int stepsPerRevolution)
    // Basic constructor for motor setup
- Stepper(int stepPin, int dirPin, int enablePin, int ms1Pin, int ms2Pin, int ms3Pin, int stepsPerRevolution)
    // Constructor with microstepping configuration
*/



// Put struct definition here
class Stepper {

private:

    // Motor control pins
    int stepPin;     // Pin connected to the step signal
    int dirPin;      // Pin connected to the direction control
    int enablePin;   // Pin to enable/disable the motor driver
    int resetPin;
    int sleepPin;


    // Microstepping control pins
    int ms1Pin;      // Microstepping pin 1
    int ms2Pin;      // Microstepping pin 2
    int ms3Pin;      // Microstepping pin 3

    // Motor States
    bool isEnabled;         // Current enabled/disabled state
    // bool isMoving;
    int direction;       // Current rotation direction (0 for stationary)
    long currentStep;       // Current step count position
    long targetStep;        // Target step position for movement
    float stepAngle;        // Step angle in degrees
    


    // Motor specifications and state
    // float stepAngle;        // Step angle in degrees (360 / stepsPerRevolution)

    // Timing and speed
    // int stepDelay;          // Delay between steps in microseconds (speed control)
    // unsigned long lastStepTime; // Last step timestamp for precise timing





public:
    int stepsPerRevolution; // Number of steps for a full revolution
    int microstepMode;         // microstepping mode (1,2,4,8...)
    // Constructor  declaration (stepPin, dirPin, enablePin, stepsPerRevolution)
    Stepper(int stepPin, int dirPin, int enablePin, int ms1Pin, int ms2Pin, int ms3Pin, int resetPin, int sleepPin, int stepsPerRevolution);
    // Stepper(int stepPin, int dirPin, int enablePin); // Overloaded constructor

    // Method declarations

    // Set microstepping mode (1, 2, 4, 8, 16)
    void setMicrostepMode(int mode);


    // void initalize();
    // void enableMotor();
    // void disableMotor();
    int getStepsPerRevolution() const;
    void moveForwardPct(float percentage);
    void moveStep(int stepDelay);

    // Method to move the motor to a specific percentage around 360°
	void moveToPercentage(float percentage);

};




#endif



// Notes:
// Header file: (simple defs)
// void foo() { return food; }

// Stepper.cpp (outside defs --> complex fucntions

// void Stepper::foo() {
//     code here
// }



// Using this-> is necessary when there is a name conflict between 
// a class member variable and a function parameter or local variable.
// It explicitly refers to the object’s member variable. Points to current object.