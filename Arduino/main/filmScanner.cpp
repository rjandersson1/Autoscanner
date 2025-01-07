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
    poti.setMap(0, 500);              // Set potentiometer mapping range
    // while (abs(poti.getMap()) > 10) {
    //     poti.read();
    //     Serial.print("Move poti to 0: ");
    //     Serial.println(poti.getMap());
    // }     // wait until poti moved to 0 before continuing

    stepper.enable();
    const long largeStepCount = 10000; // Arbitrary large step count
    short direction = 1;                 // Initial direction
    short lastDirection = 1;             // To track direction changes
    short rpmThreshold = 1;
    stepper.setMicrostep(1);
    // stepper.setRPM(100);
    // stepper.startMove(largeStepCount);   // Start non-blocking stepper move

    while (buttonC.state) {              // Run while buttonC is active
        poti.read();
        buttonC.read();
        int rpm = poti.getMap();       // Get RPM from potentiometer
        stepper.setRPM(rpm);
        stepper.move(200);
        Serial.println(rpm);
        // stepper.rotate(pos);
        // if (abs(rpm) < 10) {             // Threshold to stop motor
        //     stepper.stop();
        // } else if (rpm < 0) {
        //     direction = -1;              // Set direction to reverse
        //     rpm = abs(rpm);              // Use absolute RPM value
        // } else {
        //     direction = 1;               // Set direction to forward
        // }

        // if (direction != lastDirection) {  // Check if direction changed
        //     // stepper.stop();
        //     stepper.startMove(direction*largeStepCount);
        //     lastDirection = direction;            // Save current direction
        // } 
        // rpm = 2;
        // stepper.setRPM(rpm);             // Update RPM
        // stepper.stop();
        // stepper.startMove(largeStepCount);
        // stepper.move(10);
        // stepper.startMove(largeStepCount);
        // if (rpm > rpmThreshold) {
        //   stepper.enable();
        //   stepper.startMove(largeStepCount);
        // }
        // else {
        //   stepper.disable();
        //   stepper.stop();
        // }
        // stepper.nextAction();            // Execute next step
    }
}
