#pragma once
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
// #include "AccelStepper.h"
// #include "IRremote/IRremote.h"
// #include "TMCStepper/TMCStepper.h"
#include <AccelStepper.h>
#include <TMCStepper.h>
// class IRsend;
#include "Buttons.h"

class filmScanner {

public:

    // Constructor
    filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &buttonA, Button &buttonB, toggleButton &buttonC, Poti &poti, IRsend &irLED, int PIN_LED);

    // Reference Objects
    AccelStepper &motor;
    TMC2209Stepper &driver;
    Button &buttonA;
    Button &buttonB;
    toggleButton &buttonC;
    Poti &poti;
    IRsend &irLED;


    // Properties
    long frameWidth = 0; // [steps]
    bool scanning;
    int PIN_LED; // LED pin for feedback
    float multiplier = 1.0;

    // default motor parameters
    float maxAcceleration = 40000; // [steps/s^2]
    float maxSpeed = 1000; // [steps/s]


    // Methods
    void moveFrame(long steps);
    void setup();
    long calibrate();
    void takePhoto();
    void startScan();
    void stopScan();
    void dynamicMove();
    long dynamicPosition();
    void DEBUG_findMaxVelocity();
    void scan135();
    float setScanSpeed();


private:


};