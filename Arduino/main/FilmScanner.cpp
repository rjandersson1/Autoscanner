#include "FilmScanner.h"
#include <Arduino.h>


// Constructor
filmScanner::filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &buttonA, Button &buttonB, toggleButton &buttonC, Poti &poti, IRsend &irLED, int PIN_LED)
    : motor(motor), driver(driver), buttonA(buttonA), buttonB(buttonB), buttonC(buttonC), poti(poti), irLED(irLED), PIN_LED(PIN_LED)
{
    setup();
}

void filmScanner::setup() {
    // TODO: add setup code here if needed
}

// Sends IR signal 3 times
void filmScanner::takePhoto() {
    digitalWrite(PIN_LED, HIGH); // Turn on LED to indicate photo taking
    for (int i = 0; i < 3; i++) {
        irLED.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
        delay(40);
    }
    digitalWrite(PIN_LED, LOW); // Turn off LED after photo taken
    Serial.println("Photo taken.");
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
    digitalWrite(PIN_LED, HIGH); // Turn on LED to indicate positioning
    // Init vars
    const long MAP_VAL = 400; // Max potentiometer value (+/- steps)
    const int MICROSTEP = 2;
    const int DELTA_THRESHOLD = 2; // Threshold for change in position to trigger movement
    const int FILTER_WINDOW = 1; // Number of readings for moving average
    const int ACCEL_MOVING = 1000; // Acceleration for movement [step/s^2]
    const int SPEED_MOVING = 1000; // Speed for movement [steps/s]
    const int SPEED_POSITIONING = 1000; // (fine) Speed for positioning [steps/s]
    const long POTI_LIMIT = (MAP_VAL * 99) / 100; // Limit for potentiometer to trigger continuous movement

    // Setup
    poti.setMap(-MAP_VAL, MAP_VAL);
    poti.initFilter(FILTER_WINDOW); // Initialize filter with window size
    driver.microsteps(MICROSTEP);
    motor.setMaxSpeed(1000);
    
    // Wait for potentiometer to return to zero (+/- 2)
    Serial.println("Waiting for potentiometer to return to zero...");
    while (poti.getMap() > 2 || poti.getMap() < -2) {
        poti.read();
        Serial.print("Current potentiometer value: ");
        Serial.println(poti.getMap());
        delay(10); // Delay to avoid flooding the serial output
    }
    delay(1000);
    digitalWrite(PIN_LED, LOW); // Turn off LED to indicate ready
    
    // Set initial position to zero
    motor.setCurrentPosition(0); 

    // Loop
    Serial.println("Starting dynamic positioning. Press button A to exit.");
    buttonA.read(); // Read button state before entering loop
    while (!buttonA.isPressed) {
        buttonA.read();
        if (buttonA.isPressed) {
            Serial.println("Button A pressed. Exiting dynamic positioning.");
            break;  // Exit loop if button A is pressed
        }
        // // Read potentiometer value
        int newReading = poti.getMap();
        // Serial.println(newReading);

        // Case 1: Poti at limit --> begin ramp up to continuous movement
        if (abs(newReading) > POTI_LIMIT) {
            // Serial.println("Poti at limit. Starting continuous movement...");
            // Serial.println(POTI_LIMIT);
            // Serial.print("New reading: ");
            // Serial.println(newReading);

            // Set direction
            int moveDir = (newReading > 0) ? 1 : -1; 

            // Set speed/accel for continuous movement
            motor.setAcceleration(ACCEL_MOVING);
            motor.setSpeed(SPEED_MOVING);

            // Set target position far away to allow continuous movement
            motor.move(moveDir * 10000);

            // Continuously run motor (with ramps) until poti is moved away from limits
            while (abs(poti.getMap()) > POTI_LIMIT) {
                motor.run();
            }
            // motor.move(0);
            motor.run();
            // motor.stop(); // optional: decelerate motor after reaching limit

            // Reset motor speed to default
            // motor.setSpeed(SPEED_POSITIONING); // Set speed for positioning
        }
        
        // Case 2: Poti not at limit --> begin dynamic positioning with poti readings
        else {
            
            // Compare positions
            long currentPos = motor.currentPosition();
            long delta = (long)newReading - currentPos; // Calculate change in position

            if (abs(delta) > DELTA_THRESHOLD) {
                // Serial.println("Poti not at limit. Starting dynamic positioning...");
                if (delta > 0) motor.setMaxSpeed(SPEED_POSITIONING); // Set speed for positive movement
                if (delta < 0) motor.setMaxSpeed(-SPEED_POSITIONING); // Set speed for negative movement

                // Run motor to new position
                motor.move(delta); // Set target position
                motor.setAcceleration(40000); // Set acceleration for positioning
                while (motor.distanceToGo() != 0) {
                    motor.run(); // Run motor to reach target position
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
    Serial.println("DEBUG: Finding max velocity. Press button A to exit.");
    delay(3000);
    poti.setMap(0, 16000);
    int microStepSetting = 1;
    driver.microsteps(microStepSetting); // Set microstepping
    motor.setMaxSpeed(1000); // Set initial max speed
    motor.setAcceleration(50); // Set initial acceleration
    motor.move(200);
    while (motor.distanceToGo() != 0) {
        motor.run(); // Run motor to reach target position
    }
    int direction = 1;
    while (1) {
        // if (buttonA.isPressed) {
        //     break;  // Exit loop if button A is pressed
        // }

        // Read potentiometer value
        int newReading = poti.getMap()+ 150;

        Serial.println("Speed:" + String(newReading) + "[step/s]" + 
                       " Acceleration:" + String(maxAcceleration) + "[step/s^2]" + microStepSetting + "x microstepping");
        
        // Move 400 steps at set speed or until 3 seconds has passed
        motor.setMaxSpeed(1000); // Set speed based on potentiometer reading
        motor.setAcceleration(newReading); // Set acceleration
        motor.move(2 * 200 * microStepSetting * direction);
        long startTime = millis(); // Start time for movement
        while (motor.distanceToGo() != 0) {
            motor.run(); // Run motor at set speed
        }
        // while (motor.run());
        // motor.stop(); // Stop motor after movement
        buttonA.read();
        long timerStart = millis(); // Reset timer
        Serial.println("reading...");
        while (millis() - timerStart < 500 && !buttonA.isPressed) {
            buttonA.read();
            if (buttonA.isPressed) {
                if (microStepSetting == 16) {
                    microStepSetting = 1;
                }
                else {
                    microStepSetting = microStepSetting * 2;
                }
                driver.microsteps(microStepSetting); // Set microstepping
            }
        }
        direction = direction * -1;
        Serial.println("Moving...");
    }
}

long filmScanner::calibrate() {
    // Position scanner to frame start
    dynamicPosition();
    takePhoto();

    // Position scanner to next frame (or end of frame)
    frameWidth = dynamicPosition(); // [steps]

    takePhoto();

    // TODO: if medium format (i.e. frameWide > 36mm), use a button to set how many scans to make. Also use dynamicPosition() to set distance to next frame start

    // if frameWidth > 135 --> set as 120 --> set scan count
    // gutterWidth = dynamicPosition(); // distance from end of frame 0 to start of frame 1[steps] 
    Serial.println(frameWidth);
    return frameWidth;
}