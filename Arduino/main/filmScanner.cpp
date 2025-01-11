#include "buttons.h"
#include <Arduino.h>
#include "FilmScanner.h"


// Constructor
filmScanner::filmScanner(A4988 &stepper, Button &buttonA, Button &buttonB, toggleButton &buttonC, Poti &poti)
    : stepper(stepper), buttonA(buttonA), buttonB(buttonB), buttonC(buttonC), poti(poti)
{
    setup();
}

void filmScanner::setup() {
    setOutputRatio(10, 0.875);
}

// Sends IR signal 3 times
void filmScanner::takePhoto() {
    // for (int i = 0; i < 3; i++) {
    //     ir.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
    //     delay(40);
    // }
}

void filmScanner::moveFrame() {
    float targetDegrees = frameWidth / mmPerDegree; // [mm / (mm/deg)] = [deg]
    stepper.rotate(targetDegrees);
}

void filmScanner::setOutputRatio(float diameter, float ratio) {
    shaftDiameter = diameter;
    // outputRatio = ratio;
    outputRatio = 0.875; // TEMP
    mmPerDegree = (3.1415 * shaftDiameter) / (360.0 * outputRatio);
}

void filmScanner::dynamicMove() {
    poti.setMap(-50,50);
    int newRPM = 0;
    int oldRPM = 0;
    newRPM = poti.getMap(); // Init first RPM
    stepper.setMicrostep(16);

}
