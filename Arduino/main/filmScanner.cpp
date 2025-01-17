#include <IRremote.h>
#include "buttons.h"
#include <Arduino.h>
#include "FilmScanner.h"


#define PIN_IR   9   // BR -> IR LED
IRsend ir(PIN_IR); // IR object

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
    Serial.println("send");
    for (int i = 0; i < 3; i++) {
        ir.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
        delay(40);
        Serial.println("s");
    }
}

// void filmScanner::moveFrame() {
//     float targetDegrees = frameWidth / mmPerDegree; // [mm / (mm/deg)] = [deg]
//     stepper.rotate(targetDegrees);
// }

void filmScanner::setOutputRatio(float diameter, float ratio) {
    shaftDiameter = diameter;
    // outputRatio = ratio;
    outputRatio = 0.875; // TEMP
    mmPerDegree = (3.1415 * shaftDiameter) / (360.0 * outputRatio);
}

void filmScanner::dynamicPosition(int mapVal = 400, int initialDelay = 16192, int rampTime = 25000, int microstep = 16, int threshold = 2, int window = 5) {
    // Init vars
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
    stepper.setMicrostep(microstep);
    stepper.setRPM(100);

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
        stepper.move(stepsToMove);
        currentPosition = targetPosition;
    }
}

void filmScanner::dynamicMove() {
    poti.setMap(0, 16000);
    stepper.setMicrostep(1);
    stepper.setRPM(1000);
    while (buttonC.state) {
        buttonC.read();
        float delayTime = poti.getMap();
        int dir = (delayTime > 0) ? 1 : -1;
        stepper.move(dir);
        delayMicroseconds(abs(delayTime));
    }
}

void filmScanner::moveFrame() {
    int frameWidth = 300;
    stepper.setMicrostep(16);
    stepper.setRPM(500);
    stepper.move(frameWidth);
    delay(500);
    takePhoto();
    delay(500);
    stepper.move(frameWidth);
    delay(500);
    takePhoto();
    Serial.println("done");
}