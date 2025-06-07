#include "FilmScanner.h"
#include <Arduino.h>


// Constructor
filmScanner::filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &buttonA, Button &buttonB, toggleButton &buttonC, Poti &poti, IRsend &irLED)
    : motor(motor), driver(driver), buttonA(buttonA), buttonB(buttonB), buttonC(buttonC), poti(poti), irLED(irLED)
{
    setup();
}

void filmScanner::setup() {
    // TODO: add setup code here if needed
}

// Sends IR signal 3 times
void filmScanner::takePhoto() {
    for (int i = 0; i < 3; i++) {
        irLED.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
        delay(40);
    }
}

void filmScanner::moveFrame() {
    motor.setMaxSpeed(maxSpeed); // default max speed [steps/s]
    motor.setAcceleration(maxAcceleration); // default max acceleration [steps/s^2]
    motor.move(frameWidth); // Move motor by frame width [steps]
    while (motor.distanceToGo() != 0) {
        motor.run(); // Run motor to reach target position
    }
    motor.stop();
}


// Moves film scanner position based on potentiometer input. When at limits, it ramps up to continuous movement.
long filmScanner::dynamicPosition() {
    // Init vars
    const int MAP_VAL = 400; // Max potentiometer value (+/- steps)
    const int MICROSTEP = 1;
    const int DELTA_THRESHOLD = 2; // Threshold for change in position to trigger movement
    const int FILTER_WINDOW = 5; // Number of readings for moving average
    const int ACCEL_MOVING = 5; // Acceleration for movement [step/s^2]
    const int SPEED_MOVING = 50; // Speed for movement [steps/s]
    const int SPEED_POSITIONING = 100; // (fine) Speed for positioning [steps/s]
    const int POTI_LIMIT = (MAP_VAL * 99) / 100; // Limit for potentiometer to trigger continuous movement

    // Setup
    poti.setMap(-MAP_VAL, MAP_VAL);
    poti.initFilter(FILTER_WINDOW); // Initialize filter with window size
    driver.microsteps(MICROSTEP);
    motor.setMaxSpeed(1000);
    
    // Wait for potentiometer to return to zero
    while (poti.getMap() != 0) {
        poti.read();
    }
    
    // Set initial position to zero
    motor.setCurrentPosition(0); 

    // Loop
    while (1) {
        buttonA.read();
        if (buttonA.isPressed) {
            break;  // Exit loop if button A is pressed
        }
        // Read potentiometer value
        int newReading = poti.getMap();

        // Case 1: Poti at limit --> begin ramp up to continuous movement
        if (abs(newReading) > POTI_LIMIT) {
            // Set direction
            int moveDir = (newReading > 0) ? 1 : -1; 

            // Set speed/accel for continuous movement
            motor.setAcceleration(ACCEL_MOVING);
            motor.setSpeed(SPEED_MOVING);

            // Set target position far away to allow continuous movement
            motor.move(moveDir * 100000);

            // Continuously run motor (with ramps) until poti is moved away from limits
            while (abs(poti.getMap()) > POTI_LIMIT) {
                motor.run();
            }
            // motor.stop(); // optional: decelerate motor after reaching limit

            // Reset motor speed to default
            motor.setSpeed(SPEED_POSITIONING); // Set speed for positioning
        }
        
        // Case 2: Poti not at limit --> begin dynamic positioning with poti readings
        else {
            // Compare positions
            long currentPos = motor.currentPosition();
            long delta = (long)newReading - currentPos; // Calculate change in position

            if (abs(delta) > DELTA_THRESHOLD) {
                if (delta > 0) motor.setSpeed(SPEED_POSITIONING); // Set speed for positive movement
                if (delta < 0) motor.setSpeed(-SPEED_POSITIONING); // Set speed for negative movement

                // Run motor to new position
                motor.move(delta); // Set target position
                while (motor.distanceToGo() != 0) {
                    motor.runSpeed(); // Run motor to reach target position
                }
            }
        }
    }
    // Reset motor parameters to default
    motor.setMaxSpeed(maxSpeed); // default max speed [steps/s]
    motor.setAcceleration(maxAcceleration); // default max acceleration [steps/s^2]

    // Once finished positioning, return distance traveled
    long distanceTraveled = motor.currentPosition(); // Calculate distance traveled based on initial reading
    Serial.print("Distance traveled: ");
    Serial.println(distanceTraveled);
    return distanceTraveled;
}

// Changes stepper speed based on potentiometer value
void filmScanner::DEBUG_findMaxVelocity() {
    poti.setMap(0, 16000);
    driver.microsteps(1);
    while (1) {
        buttonA.read();
        if (buttonA.isPressed) {
            break;  // Exit loop if button A is pressed
        }

        // Read potentiometer value
        int newReading = poti.getMap();

        Serial.println("Speed:" + String(newReading) + "[step/s]" + 
                       " Acceleration:" + String(maxAcceleration) + "[step/s^2]");
        
        // Move 400 steps at set speed or until 3 seconds has passed
        motor.move(400);
        motor.setSpeed(newReading); // Set speed based on potentiometer reading
        motor.setAcceleration(maxAcceleration); // Set acceleration
        long startTime = millis(); // Start time for movement
        while (motor.distanceToGo() != 0 && millis() - startTime < 3000) {
            motor.run(); // Run motor at set speed
        }
        motor.stop(); // Stop motor after movement
        delay(1000);
    }
}

long filmScanner::calibrate() {
    // Position scanner to frame start
    dynamicPosition();

    // Position scanner to next frame (or end of frame)
    frameWidth = dynamicPosition(); // [steps]

    // TODO: if medium format (i.e. frameWide > 36mm), use a button to set how many scans to make. Also use dynamicPosition() to set distance to next frame start

    // if frameWidth > 135 --> set as 120 --> set scan count
    // gutterWidth = dynamicPosition(); // distance from end of frame 0 to start of frame 1[steps] 
    return frameWidth;
}