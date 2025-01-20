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
void filmScanner::takePhoto(int speed = 1) {
    Serial.println("snap");
    // for (int i = 0; i < 3; i++) {
    //     ir.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
    //     delay(40);
    //     Serial.println("s");
    // }
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_IR, HIGH);
        delay(50);
        digitalWrite(PIN_IR, LOW);
        delay(50);
        Serial.println("s");
    }
    delay(100/speed); // time for shutter to fire
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

void filmScanner::moveFrame(int speed = 1) {
    float degreesToMove = frameWidth / mmPerDegree;
    stepper.setSpeedProfile(stepper.LINEAR_SPEED, 3300*speed, 3300*speed); // set acceleration profile
    stepper.setRPM(200*speed);
    stepper.setMicrostep(16);
    stepper.enable();
    stepper.rotate(degreesToMove);
    stepper.disable();
}

void filmScanner::setFramewidth() {
    stepper.enable(); 
    // Init vars
    int mapVal = 400;                   // Range (steps) of potentiometer
    poti.setMap(-mapVal, mapVal);       // Apply range
    stepper.setMicrostep(16);           // Set microstep (smoothest & finest operation)
    stepper.setRPM(100);                // Set base RPM (excessively slow (<50) --> less responsive)
    int currentPosition = poti.getMap();    // Set initial position
    int targetPosition = currentPosition;   // Set initial position
    int initialDelay = 16192;               // Ramp delay [uS]
    int rampTime = 25000;                   // Ramp update time [uS]
    int threshold = 2;                      // Poti moving average delta threshold
    int window = 5;                         // Moving average window
    int offset = 0;                         // For moving at end of range
    int delayTime = initialDelay;           // Set initial ramp delay
    int readings[window] = {0};             // Init vector collection of past readings
    int index = 0;                          // Index of readings
    int sum = 0;                            // Moving average sum
    int average = currentPosition;          // Moving average
    unsigned long moveStartTime = 0;        // Timestamp of move begin
    bool isMoving = false;                  // Bool of move

    // Update readings vector to current position
    sum = 0; // Reset sum
    for (unsigned int i = 0; i < window; i++) {
        readings[i] = currentPosition;
        sum += currentPosition; // Add currentPosition to sum
    }
    average = sum / window; // Set initial average correctly

    // Step counting
    int stepOffset = 0;

    // Loop
    buttonC.state = 1;
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
        if (abs(average - targetPosition) > threshold) {
            targetPosition = average;
        } 
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
        stepOffset = stepOffset + stepsToMove; // update stepcount
        stepper.move(stepsToMove);
        currentPosition = targetPosition;
    }

    Serial.print("Step offset set to ");
    Serial.println(stepOffset);

    Serial.print("New framewidth: ");
    float fullSteps = stepOffset / stepper.getMicrostep(); // divide by microstep mode to get full steps moved
    float degreeOffset = 360 * (fullSteps / stepper.getSteps()); // divide by steps per revolution to get degrees
    float mmOffset = degreeOffset * mmPerDegree; // calculate mm traveled with mm per degree constant
    frameWidth = frameWidth + mmOffset; //update new framewidth
    Serial.print(degreeOffset);
    Serial.print("deg  ");
    Serial.println(frameWidth);
    stepper.disable();
}

void filmScanner::scanFrame() {
    for (unsigned int i = 0; i < 3; i++) {
        moveFrame(1);
        takePhoto(1);
    }
}

void filmScanner::startScanning() {
    buttonC.state = 1;
    while (buttonC.state) {
        buttonA.read();
        buttonB.read();
        buttonC.read();

        // Add a speed selection here
        poti.setMap(1, 5);
        int speed = poti.getMap();

        moveFrame(speed);
        takePhoto(speed);
    }
}