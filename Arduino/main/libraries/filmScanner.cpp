#include "FilmScanner.h"
#include <Arduino.h>


// Constructor
filmScanner::filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &buttonA, Button &buttonB, toggleButton &buttonC, Poti &poti, IRsend &ir)
    : motor(motor), driver(driver), buttonA(buttonA), buttonB(buttonB), buttonC(buttonC), poti(poti), ir(ir)
{
    setup();
}

void filmScanner::setup() {
    setOutputRatio(10, 0.875);
}

// Sends IR signal 3 times
void filmScanner::takePhoto() {
    for (int i = 0; i < 3; i++) {
        ir.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
        delay(40);
    }
}

void filmScanner::moveFrame() {
    float targetDegrees = frameWidth / mmPerDegree; // [mm / (mm/deg)] = [deg]
    motor.rotate(targetDegrees);
}

void filmScanner::setOutputRatio(float diameter, float ratio) {
    shaftDiameter = diameter;
    // outputRatio = ratio;
    outputRatio = 0.875; // TEMP
    mmPerDegree = (3.1415 * shaftDiameter) / (360.0 * outputRatio);
}

// Moves film scanner position based on potentiometer input. When at limits, it ramps up to continuous movement.
void filmScanner::dynamicPosition() {
    // Init vars
    int mapVal = 400; // Max potentiometer value (+/- steps)
    int initialDelay = 16192; // Initial delay time in microseconds
    int rampTime = 25000; // Time in microseconds to ramp up speed
    int microstep = 1;
    int threshold = 2; // Threshold for change in position to trigger movement
    int window = 5; // Number of readings for moving average
    int offset = 0;             // for moving
    int delayTime = initialDelay;
    int readings[window] = {0}; // Collection of past readings
    int index = 0;              // index of readings
    int sum = 0;                
    int average = 0;            // moving average
    unsigned long moveStartTime = 0;
    bool isMoving = false;
    int targetPosition = 0;
    int currentPosition = 0;

    // Setup
    poti.setMap(-mapVal, mapVal);
    poti.initFilter(window); // Initialize filter with window size
    driver.microsteps(microstep);
    motor.setMaxSpeed(1000);

    // Loop
    while (buttonC.state) {
        buttonC.read();
        int newReading = poti.getMap();
        
        // Update moving average
        sum -= readings[index]; // erase old reading
        readings[index] = newReading; // update new reading
        sum += newReading; // update sum
        index = (index + 1) % window; // change index
        average = sum / window; // calculate average

        // Update target position if change exceeds threshold (reduced idle jitter)
        if (abs(average - targetPosition) > threshold) targetPosition = average;

        // Check poti to see if at end of range to determine whether to begin move
        if (abs(newReading) > mapVal * 0.99) {
            if (!isMoving) {
                // Start moving
                isMoving = true;
                moveStartTime = micros(); // update timestamp
                delayTime = initialDelay; // reset delay
            }

            // Calculate time elapsed
            unsigned long elapsedTime = micros() - moveStartTime;
            if (elapsedTime > rampTime) {
                // Half the delay to ramp up speed
                delayTime = max(delayTime / 1.05, 1);
                moveStartTime += rampTime;
            }

            offset = (targetPosition > 0) ? 1 : -1; // Determine direction
            delayMicroseconds(delayTime);           // apply delay
            
        } else {
            isMoving = false;
            offset = 0;
        }

        // Move # of steps
        int stepsToMove = targetPosition + offset - currentPosition;
        motor.moveTo(stepsToMove);
        while (motor.distanceToGo() != 0) {
            motor.run();
        }
        currentPosition = targetPosition;
    }
}

// Changes stepper speed based on potentiometer value
void filmScanner::dynamicMove() {
    poti.setMap(0, 16000);
    driver.microsteps(1);
    motor.setMaxSpeed(1000);
    while (buttonC.state) {
        buttonC.read();
        float delayTime = poti.getMap();
        int dir = (delayTime > 0) ? 1 : -1;
        stepper.move(dir);
        delayMicroseconds(abs(delayTime));
    }
}

