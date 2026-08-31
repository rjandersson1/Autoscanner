#include "FilmScanner.h"
#include <Arduino.h>

// TODO
// one perf ~185s @ 8uS
// one frame 8 perf
// 1mm = 38.95 steps

//

// Constructor
filmScanner::filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &buttonA, Poti &poti, IRsend &irLED, int PIN_LED)
    : motor(motor), driver(driver), buttonA(buttonA), poti(poti), irLED(irLED), PIN_LED(PIN_LED)
{
    //
}

void filmScanner::setup() {
    poti.initFilter(3); // Initialize filter with window size
    calibrate();
}

// Sends IR signal 3 times
void filmScanner::takePhoto() {
    digitalWrite(PIN_LED, HIGH); // Turn on LED to indicate photo taking
    for (int i = 0; i < 3; i++) {
        irLED.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
        delay(40);
    }
    digitalWrite(PIN_LED, LOW); // Turn off LED after photo taken
}

void filmScanner::scanFrame() {
    delay(exposureTime / 2);
    takePhoto();
    delay(exposureTime / 2);
}

// Moves film scanner position based on potentiometer input. When at limits, it ramps up to continuous movement.
long filmScanner::dynamicPosition() {
    // Init vars
    const long MAP_VAL = 800/2; // Max potentiometer value (+/- steps)
    const int MICROSTEP = 8; // Was 8 originally
    const int DELTA_THRESHOLD = 0.005 * MAP_VAL; // Threshold for change in position to trigger movement
    const long ACCEL_MOVING = 2000; // Acceleration for movement [step/s^2]
    const long SPEED_MOVING = 8000; // Speed for movement [steps/s]
    const long ACCEL_POSITIONING = ACCEL_MOVING * 100;
    const long SPEED_POSITIONING = 8000; // (fine) Speed for positioning [steps/s]
    const long POTI_LIMIT = (MAP_VAL * 99) / 100; // Limit for potentiometer to trigger continuous movement

    // Setup
    poti.setMap(-MAP_VAL, MAP_VAL);
    driver.microsteps(MICROSTEP);
    motor.setMaxSpeed(SPEED_MOVING);
    long stepCount = 0;
    
    // Wait for potentiometer to return to zero (+/- 2)
    // while ((float)abs(poti.getMap()) > (float)(0.01 * MAP_VAL)) {
    //     poti.read();
    //     delay(10); // Delay to avoid flooding
    // }
    // motor.setCurrentPosition(0); 

    // Loop
    long prevPotiReading = poti.getMap(); // Previous potentiometer reading
    while (1) {
        buttonA.read();
        if (buttonA.isPressed) { // Exit loop
            digitalWrite(PIN_LED, LOW);
            delay(150);
            digitalWrite(PIN_LED, HIGH);
            delay(150);
            digitalWrite(PIN_LED, LOW);
            break;
        }

        // Read potentiometer value
        poti.read(); // Read potentiometer value
        long potiReading = poti.getMap(); // Read raw potentiometer value
        long newReading = potiReading + motor.currentPosition();

        // Case 1: Poti at limit --> begin ramp up to continuous movement
        if (abs(potiReading) > POTI_LIMIT) {
            // Set direction
            int moveDir = (potiReading > 0) ? 1 : -1; 

            // Set speed/accel for continuous movement
            motor.setAcceleration(ACCEL_MOVING);
            motor.setMaxSpeed(SPEED_MOVING);

            // Set target position far away to allow continuous movement
            motor.move(moveDir * 10000);

            // Continuously run motor (with ramps) until poti is moved away from limits
            while (abs(poti.getMap()) > POTI_LIMIT) {
                motor.run();
                poti.read();
            }
        }
        
        // Case 2: Poti not at limit --> begin dynamic positioning with poti readings
        else {
            // Compare positions
            long delta = prevPotiReading - potiReading; // Calculate change in position
            
            if (abs(delta) > DELTA_THRESHOLD) {
                motor.setAcceleration(ACCEL_POSITIONING); // Set acceleration for positioning
                motor.setMaxSpeed(SPEED_POSITIONING);
                motor.move(-delta); // Set target position
                while (motor.distanceToGo() != 0) {
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
    // Serial.print("Distance traveled: ");
    // Serial.println(distanceTraveled);
    return distanceTraveled;
}


// Move to start, set zero (A), move to end of frame (A), moves one frame, move to start again (A). Return distance
long filmScanner::calibrate() {
    // Move to start
    dynamicPosition();
    motor.setCurrentPosition(0);

    // Move to end of frame
    long measured = dynamicPosition();

    // Move back to start
    moveFrame(-measured);

    // Compare widths to known distances
    long measuredWidth = abs(measured);
    direction = (measured > 0) - (measured < 0);
    Serial.println(measuredWidth);

    // Set scan mode
    scanMode = setScanMode();

    if (scanMode == 0) {
        scanCount = 1;
        frameWidth = frameWidth_135;
        gutterWidth = gutterWidth_135;
    }
    if (scanMode == 1) {
        frameWidth = measuredWidth;
        gutterWidth = 100;

        // Set scans per frame for 120
        scanCount = setScansPerFrame();
        Serial.print("Scancount: ");
        Serial.println(scanCount);
    }

    // Measure exposure timing
    // setScanTime(); // temp removed
    // exposureTime = 200; // temporary hardcode

    // Begin scan
    scan();
}

// Move Frame in 16uS
void filmScanner::moveFrame(long steps) {
    driver.microsteps(8);
    motor.setMaxSpeed(10000);
    motor.setAcceleration(2000);
    motor.move(steps);
    while (motor.run());
    return;
}

// Read poti and define 135 or 120 based on that
bool filmScanner::setScanMode() {
    Serial.println("0=135, 1=120");
    poti.setMap(0,1.999);
    buttonA.read();
    delay(100);
    while (!buttonA.isPressed) {
        poti.read();
        buttonA.read();
        Serial.println(int(poti.getMap()));
        delay(100);
    }
    return int(poti.getMap());
}

int filmScanner::setScansPerFrame() {
    delay(100);
    buttonA.read();
    Serial.println("Setting scanCount");
    int scans = 0;
    poti.setMap(0,1.99);
    while (!buttonA.isPressed) {
        buttonA.read();
        poti.read();
        Serial.println(int(poti.getMap()));
        delay(100);
    }

    int val = int(poti.getMap());
    if (val == 0) scans = 1;
    if (val == 1) scans = 3;
    if (val == 2) scans = 5;
    if (val == 3) scans = 7;
    
    return scans;
}

// TODO: add pause/restart mode, or recalibrate mode.
void filmScanner::scan() {
    delay(100);
    Serial.println("move to scan start position");
    dynamicPosition();
    delay(100);
    Serial.println("starting scan");
    delay(100);
    buttonA.read();
    if (scanMode == 0) scan135();
    if (scanMode == 1) scan120();
    Serial.println("finished scan");
}


void filmScanner::scan120() {
    while (true) {
        buttonA.read();
        if (buttonA.isPressed) break;

        // Case A
        if (scanCount == 1) {
            scanFrame();
            moveFrame(frameWidth);
        } else {
            // Case B (scanCount in {3,5,7}):
            // Start position: left_sensor == left_frame
            //
            // Total scans per exposure = scanCount + 2
            // Positions (example scanCount=3 => 5 scans):
            // 1) one step left of left edge
            // 2) left edge
            // 3) one step (i.e., +step) from left edge (middle for n=3)
            // 4) right edge
            // 5) one step right of right edge
            //
            // step is defined by evenly spacing scanCount positions across [0, frameWidth]
            // so step = frameWidth / (scanCount - 1)

            long step = (frameWidth / (scanCount - 1)) * direction;

            // Scan 1: one left
            moveFrame(-step);
            buttonA.read();
            if (buttonA.isPressed) break;
            scanFrame();

            // Scan 2: back to left edge
            moveFrame(step);
            buttonA.read();
            if (buttonA.isPressed) break;
            scanFrame();

            // Scans 3..(scanCount+1): step right, scanning each time
            // This includes the right edge on the last iteration.
            for (int i = 0; i < (scanCount - 1); i++) {
                moveFrame(step);
                buttonA.read();
                if (buttonA.isPressed) break;
                scanFrame();
            }
            if (buttonA.isPressed) break;

            // Final scan (scanCount+2): one right past right edge
            moveFrame(step);
            buttonA.read();
            if (buttonA.isPressed) break;
            scanFrame();

            // Move to next
            moveFrame(2*step);
        }

        buttonA.read();
        if (buttonA.isPressed) break;

        dynamicPosition(); // advance to next exposure start
    }
}

// Continuously scan until A is pressed.
void filmScanner::scan135() {
    while (!buttonA.isPressed) {
        buttonA.read();
        scanFrame();
        buttonA.read();
        moveFrame(frameWidth + gutterWidth);
        buttonA.read();
    }
    return;
}

// Take photo, press A when photo completed. Additional time added for buffer.
void filmScanner::setScanTime() {
    // Wait for user to press A
    delay(100);
    buttonA.read();
    Serial.println("Waiting for shutter timing. press A to start");
    while (!buttonA.isPressed) {
        buttonA.read();
    }
    delay(1500);
    // Start a timer and take a photo
    long t0 = millis();
    takePhoto();

    // Wait for A to be pressed
    while (!buttonA.isPressed) {
        buttonA.read();
    }
    long t1 = millis();
    long dt = t1 - t0;
    if (dt > 2000) {
        Serial.print("Retrying. Measured time > 2s: ");
        Serial.println(dt);
        setScanTime(); // retry in case user messed up
    }
    else {
        Serial.print("Measured time: ");
        Serial.println(dt);
        exposureTime = dt * 1.50;
    }

}