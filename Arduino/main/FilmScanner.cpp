#include "FilmScanner.h"
#include <Arduino.h>

// TODO
// Fix microstepping at 1:1 bug
// Improve smootheness/responsiveness of dynamicPosition()
// Fix bug where after moving at limit in dynamicPosition(), when moving away from limit, the motor returns to where it began, instead of moving to the new position.


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

void filmScanner::moveFrame(long steps) {
    motor.setMaxSpeed(5000); // default max speed [steps/s]
    motor.setAcceleration(16000); // default max acceleration [steps/s^2]
    driver.microsteps(4); // Set microstepping to 1/16
    motor.move(steps/4); // Move motor by frame width [steps]

    while (motor.isRunning() != 0) {
        motor.run(); // Run motor to reach target position
    }
}


// Moves film scanner position based on potentiometer input. When at limits, it ramps up to continuous movement.
long filmScanner::dynamicPosition() {
    digitalWrite(PIN_LED, HIGH); // Turn on LED to indicate positioning
    // Init vars
    const long MAP_VAL = 800; // Max potentiometer value (+/- steps)
    const int MICROSTEP = 16;
    const int DELTA_THRESHOLD = 0.005 * MAP_VAL; // Threshold for change in position to trigger movement
    const int FILTER_WINDOW = 1; // Number of readings for moving average
    const long ACCEL_MOVING = 8000; // Acceleration for movement [step/s^2]
    const long SPEED_MOVING = 8000; // Speed for movement [steps/s]
    const long ACCEL_POSITIONING = ACCEL_MOVING * 100;
    const long SPEED_POSITIONING = 8000; // (fine) Speed for positioning [steps/s]
    const long POTI_LIMIT = (MAP_VAL * 99) / 100; // Limit for potentiometer to trigger continuous movement

    // Setup
    poti.setMap(-MAP_VAL, MAP_VAL);
    poti.initFilter(FILTER_WINDOW); // Initialize filter with window size
    driver.microsteps(MICROSTEP);
    motor.setMaxSpeed(SPEED_MOVING);
    long stepCount = 0;
    
    digitalWrite(PIN_LED, LOW); // Turn off LED to indicate ready
    // delay(1000);
    
    // Set initial position to zero
    motor.setCurrentPosition(0); 

    // Loop
    // Serial.println("Starting dynamic positioning. Press button A to exit.");
    digitalWrite(PIN_LED, HIGH);
    long prevPotiReading = 0; // Previous potentiometer reading
    while (1) {
        buttonA.read();
        if (buttonA.isPressed) {
            // Serial.println("Button A pressed. Exiting dynamic positioning.");
            digitalWrite(PIN_LED, LOW); // Turn off LED
            delay(150);
            digitalWrite(PIN_LED, HIGH); // Turn on LED to indicate exit
            delay(150);
            digitalWrite(PIN_LED, LOW); // Turn off LED
            break;  // Exit loop if button A is pressed
        }
        // // Read potentiometer value
        // prevPotiReading = poti.getMap(); // Get old potentiometer reading
        poti.read(); // Read potentiometer value
        long potiReading = poti.getMap(); // Read raw potentiometer value
        long newReading = potiReading + motor.currentPosition();
        // Serial.println(newReading);

        // Case 1: Poti at limit --> begin ramp up to continuous movement
        if (abs(potiReading) > POTI_LIMIT) {
            // Set direction
            int moveDir = (potiReading > 0) ? 1 : -1; 

            // Set speed/accel for continuous movement
            motor.setAcceleration(ACCEL_MOVING);
            motor.setMaxSpeed(SPEED_MOVING);

            // Set target position far away to allow continuous movement
            motor.move(moveDir * 40000);

            // Continuously run motor (with ramps) until poti is moved away from limits
            while (abs(poti.getMap()) > POTI_LIMIT) {
                motor.run();
                poti.read();
            }
        }
        
        // Case 2: Poti not at limit --> begin dynamic positioning with poti readings
        else {
            // Compare positions

            long currentPos = motor.currentPosition();
            long delta = prevPotiReading - potiReading; // Calculate change in position
            // Serial.println(String(currentPos) + " [] " + String(motor.targetPosition()) + " [] " + String(delta));
            
            if (abs(delta) > DELTA_THRESHOLD) {
                // Serial.println("[!] " + delta);
                motor.setAcceleration(ACCEL_POSITIONING); // Set acceleration for positioning
                motor.setMaxSpeed(SPEED_POSITIONING);
                // Run motor to new position
                motor.move(-delta); // Set target position
                while (motor.isRunning() != 0) {
                    motor.run(); // Run motor to reach target position
                }
                prevPotiReading = poti.getMap(); // Get old potentiometer reading
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
        if (buttonA.isPressed) {
            break;  // Exit loop if button A is pressed
        }

        // Read potentiometer value
        int newReading = poti.getMap()+ 150;
        
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
    // takePhoto();

    // Position scanner to next frame (or end of frame)
    frameWidth = dynamicPosition(); // [steps]

    // takePhoto();

    // TODO: if medium format (i.e. frameWide > 36mm), use a button to set how many scans to make. Also use dynamicPosition() to set distance to next frame start

    // if frameWidth > 135 --> set as 120 --> set scan count
    // gutterWidth = dynamicPosition(); // distance from end of frame 0 to start of frame 1[steps] 
    Serial.println(frameWidth);
    return frameWidth;
}

void filmScanner::scan135() {
    // Scan 135 film
    Serial.println("Scanning 135 film...");

    Serial.println("Calibrate film width... press A to select calibration.)");

    frameWidth = calibrate(); // Use calibrate() to set frame width
    moveFrame(frameWidth);
    delay(250);
    moveFrame(-frameWidth);
    // Press A to take photo, press B to move to next frame, press C to stop scanning.
    // Use poti for fine adjustments after moving to next frame.
    Serial.println("Press A to take photo, B to move to next frame, C to stop scanning. Poti for fine adjustments.");
    while (1) {
        buttonA.read();
        // delay(100);
        buttonB.read();
        // delay(100);
        buttonC.read();
        // delay(100);
        poti.read();

        if (buttonA.isPressed) {
            Serial.println("Taking photo...");
            takePhoto(); // Take photo
            delay(250); // Delay to avoid multiple photos
        }
        if (buttonB.isPressed) {
            Serial.println("Moving to next frame...");
            moveFrame(frameWidth); // Move to next frame
            delay(250); // Delay to avoid multiple moves
            dynamicPosition();
            delay(250);
            Serial.println("Taking photo...");
            takePhoto();
            delay(250);
        }
        if (buttonC.state) {
            buttonC.state = 0;
            // Serial.println("Stopping scan.");
            // buttonC.state = 0;
            // delay(250);
            // break; // Stop scanning
            Serial.println("Calibrating...");
            long before = frameWidth;
            long after = calibrate(); // Recalibrate frame width

            // Print before/after and difference
            Serial.print("Before: ");
            Serial.print(before);
            Serial.print(" After: ");
            Serial.print(after);
            Serial.print(" Difference: ");
            Serial.println(after - before);
            frameWidth = after; // Update frame width
            delay(250);
        }
    }
}