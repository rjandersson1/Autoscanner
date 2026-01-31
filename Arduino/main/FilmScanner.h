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
    filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &buttonA, Poti &poti, IRsend &irLED, int PIN_LED);

    // Reference Objects
    AccelStepper &motor;
    TMC2209Stepper &driver;
    Button &buttonA;
    Poti &poti;
    IRsend &irLED;


    // Properties
    float stepsPerMm = 38.95; 
    long frameWidth = 0;
    long frameWidth_135 = 1402; // [steps @ 8uS]
    long frameWidth_645 = 1616;
    long frameWidth_66   = 2181;
    long frameWidth_67   = 2727;
    long gutterWidth_135 = 78;
    long gutterWidth = 0;
    int scanCount = 1;
    bool scanMode = 0; // 0 = 135, 1 = 120
    long exposureTime = 0;
    int direction = 1; // + for right, - for left (looking at motor side)

    int PIN_LED; // LED pin for feedback

    // default motor parameters
    float maxAcceleration = 40000; // [steps/s^2]
    float maxSpeed = 1000; // [steps/s]


    // Methods
    void setup();
    void takePhoto();
    void scanFrame();
    long dynamicPosition();
    long calibrate();
    void moveFrame(long steps);
    bool setScanMode();
    int setScansPerFrame();
    void scan();
    void setScanTime();
    void scan135();
    void scan120();



private:


};