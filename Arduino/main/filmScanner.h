#ifndef FILMSCANNER_H
#define FILMSCANNER_H


#include "A4988.h"      // Stepper motor driver
#include "Buttons.h"    // Button handling


// extern IRsend ir;

class filmScanner {

public:

    // Constructor
    filmScanner(A4988 &stepper, Button &buttonA, Button &buttonB, toggleButton &buttonC, Poti &poti);

    // Reference Objects
    A4988 &stepper;
    Button &buttonA;
    Button &buttonB;
    toggleButton &buttonC;
    Poti &poti;


    // Properties
    float shaftDiameter = 1; // [mm]
    float outputRatio = 0.875; // [-] [TEMP]
    float mmPerDegree = 1; // [mm / deg]
    float frameWidth = 36; // [mm]
    bool scanning;


    // Methods
    void setOutputRatio(float diameter, float ratio); // Sets mechanical properties of shaft for mm calculations
    void moveFrame(int speed = 1);
    void setup();
    void calibrate();
    void takePhoto(int speed = 1);
    void startScan();
    void stopScan();
    void dynamicMove();
    void dynamicPosition(int mapVal = 400, int initialDelay = 16192, int rampTime = 25000, int microstep = 16, int threshold = 2, int window = 5);
    void setFramewidth();
    void scanFrame();

private:


};




#endif