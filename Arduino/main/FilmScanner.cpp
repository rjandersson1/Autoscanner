#include "FilmScanner.h"
#include <Arduino.h>
#include <avr/wdt.h>

// 1mm = 38.95 steps @ 1/8 microstep

filmScanner::filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &button, Poti &poti, IRsend &irLED, Solenoid &solenoid)
    : motor(motor), driver(driver), button(button), poti(poti), irLED(irLED), solenoid(solenoid)
{
}

void filmScanner::begin() {
    poti.initFilter(16);
    state = CFG_MODE;
}

void filmScanner::run() {
    switch (state) {
        case CFG_MODE:      cfgMode();        break;
        case CFG_COUNT:     cfgCount();       break;
        case CAL_IMG_START: calImageStart();  break;
        case CAL_IMG_END:   calImageEnd();    break;
        case CAL_PITCH:     calPitch();       break;
        case STANDBY:       standby();        break;
        case SCAN_FRAME:    scanFrameState(); break;
        case ADVANCE:       advance();        break;
        case RESCAN:        rescan();         break;
    }
}

// ============================ States ============================ //

void filmScanner::cfgMode() {
    Serial.println(F("mode: poti <50 single, >=50 multi"));
    poti.setMap(0, 100);
    waitButton();
    multiShot = (poti.getMap() >= 50);
    shotCount = 1;
    state = multiShot ? CFG_COUNT : CAL_IMG_START;
}

void filmScanner::cfgCount() {
    Serial.println(F("shots: poti <50 -> 3, >=50 -> 5"));
    poti.setMap(0, 100);
    waitButton();
    shotCount = (poti.getMap() >= 50) ? 5 : 3;
    state = CAL_IMG_START;
}

void filmScanner::calImageStart() {
    Serial.println(F("move to image start, press A"));
    dynamicPosition();
    finishPress();
    motor.setCurrentPosition(0);
    state = CAL_IMG_END;
}

void filmScanner::calImageEnd() {
    Serial.println(F("move to image end, press A"));
    long measured = dynamicPosition();
    finishPress();
    imageWidth = labs(measured);
    direction = (measured >= 0) ? 1 : -1;
    // Stay at the image end; the pitch step continues straight from here and
    // does the single return move back to the first frame start.
    Serial.print(F("width ")); Serial.println(imageWidth);
    state = CAL_PITCH;
}

void filmScanner::calPitch() {
    Serial.println(F("move to next image start, press A"));
    // The origin is still the first frame start (set in calImageStart, never
    // reset since), so dynamicPosition() reports the pitch directly.
    framePitch = dynamicPosition();
    finishPress();
    moveFrame(-framePitch); // back to first frame start
    motor.setCurrentPosition(0);
    frameStart = 0;
    Serial.print(F("pitch ")); Serial.println(framePitch);
    state = STANDBY;
}

void filmScanner::standby() {
    Serial.println(F("standby: move to frame start, press A to scan"));
    dynamicPosition();
    finishPress();
    frameStart = motor.currentPosition();
    redoHalfSpeed = false;
    state = SCAN_FRAME;
}

void filmScanner::scanFrameState() {
    Serial.println(F("scanning"));
    if (scanImage(redoHalfSpeed)) {
        redoHalfSpeed = false;
        state = ADVANCE;
        return;
    }
    solenoid.release();
    if (pending == EV_DOUBLE) {
        moveFrame(frameStart - motor.currentPosition());
        state = RESCAN;
    } else {
        Serial.println(F("stopped"));
        state = STANDBY;
    }
}

void filmScanner::advance() {
    // The scan sweep leaves us wherever the last capture landed (past the end
    // edge in multi shot); go straight to the next frame start, no detour
    // back through the current frame start.
    long nextStart = frameStart + framePitch;
    moveFrame(nextStart - motor.currentPosition());
    frameStart = nextStart;

    Serial.println(F("adjust if needed, press A (double = rescan)"));
    long adjusted = dynamicPosition();
    if (finishPress() == EV_DOUBLE) {
        moveFrame(frameStart - motor.currentPosition());
        state = RESCAN;
        return;
    }
    // A small nudge is drift correction and folds into the pitch permanently.
    // A large one (> 1.5x frame width) is a deliberate reposition -- e.g. new
    // film loaded -- so move there but leave the pitch alone.
    long nudge = adjusted - frameStart;
    if (labs(nudge) <= (imageWidth * 3) / 2) {
        framePitch += nudge; // cumulative pitch correction
    } else {
        Serial.println(F("large reposition, pitch unchanged"));
    }
    frameStart = adjusted;
    state = SCAN_FRAME;
}

void filmScanner::rescan() {
    Serial.println(F("rescan: reposition, press A to redo"));
    dynamicPosition();
    finishPress();
    frameStart = motor.currentPosition();
    redoHalfSpeed = true;
    state = SCAN_FRAME;
}

// ========================== Scan cycle ========================= //

// One frame. Single shot = one capture at the start edge. Multi shot =
// shotCount evenly spaced captures across the image plus one overshoot
// past each edge (shotCount + 2 total). Leaves the motor past the end edge;
// advance() moves straight on from there to the next frame.
// The button is checked between moves, not during them.
bool filmScanner::scanImage(bool slow) {
    long speed = slow ? scanSpeedSlow : scanSpeed;

    if (shotCount <= 1) {
        return frameCapture();
    }

    long step = (imageWidth / (shotCount - 1)) * direction;

    moveFrame(-step, speed);                  // overshoot before start edge
    if (!frameCapture()) return false;

    moveFrame(step, speed);                   // start edge
    if (!frameCapture()) return false;

    for (int i = 0; i < shotCount - 1; i++) { // interior points up to end edge
        moveFrame(step, speed);
        if (!frameCapture()) return false;
    }

    moveFrame(step, speed);                   // overshoot past end edge
    if (!frameCapture()) return false;

    return true;
}

// Settle, clamp film, expose, release. Returns false if the button is pressed.
bool filmScanner::frameCapture() {
    if (!hold(settleTime)) return false;
    solenoid.engage();
    if (!hold(settleTime)) { solenoid.release(); return false; }
    takePhoto();
    if (!hold(exposureTime)) { solenoid.release(); return false; }
    solenoid.release();
    return true;
}

// Sends IR shutter command 3 times
void filmScanner::takePhoto() {
    for (int i = 0; i < irShutterRepeats; i++) {
        irLED.sendSony(0xB4B8F, 20); // Sony 12-bit command
        delay(irShutterGap);
    }
}

// =========================== Movement ========================= //
// Unchanged from the pre-rework firmware. Motion loops stay tight (no
// button reads inside) so stepping is smooth and responsive.

// Moves position based on potentiometer input. At the limits it ramps up to
// continuous movement. Returns the motor position when A is pressed.
long filmScanner::dynamicPosition() {
    const long MAP_VAL = jogRange;
    const int  MICROSTEP = transportMicrostep;
    const int  DELTA_THRESHOLD = 0.005 * MAP_VAL;
    const long ACCEL_MOVING = jogAccel;
    const long SPEED_MOVING = jogSpeed;
    const long ACCEL_POSITIONING = ACCEL_MOVING * 100;
    const long SPEED_POSITIONING = jogSpeed;
    const long POTI_LIMIT = (MAP_VAL * 99) / 100;

    poti.setMap(-MAP_VAL, MAP_VAL);
    driver.microsteps(MICROSTEP);
    motor.setMaxSpeed(SPEED_MOVING);

    long prevPotiReading = poti.getMap();
    while (1) {
        button.read();
        if (button.isPressed) break;

        poti.read();
        long potiReading = poti.getMap();

        // Case 1: poti at limit -> ramp up to continuous movement
        if (abs(potiReading) > POTI_LIMIT) {
            int moveDir = (potiReading > 0) ? 1 : -1;
            motor.setAcceleration(ACCEL_MOVING);
            motor.setMaxSpeed(SPEED_MOVING);
            motor.move(moveDir * 10000);
            while (abs(poti.getMap()) > POTI_LIMIT) {
                motor.run();
                poti.read();
            }
        }
        // Case 2: poti not at limit -> track poti readings
        else {
            long delta = prevPotiReading - potiReading;
            if (abs(delta) > DELTA_THRESHOLD) {
                motor.setAcceleration(ACCEL_POSITIONING);
                motor.setMaxSpeed(SPEED_POSITIONING);
                motor.move(-delta);
                while (motor.distanceToGo() != 0) {
                    motor.run();
                }
                prevPotiReading = poti.getMap();
            }
        }
    }

    motor.setMaxSpeed(restSpeed);
    motor.setAcceleration(restAccel);
    return motor.currentPosition();
}

// Blocking relative move at the transport microstepping.
void filmScanner::moveFrame(long steps, long speed) {
    if (speed < 0) speed = transportSpeed;
    driver.microsteps(transportMicrostep);
    motor.setMaxSpeed(speed);
    motor.setAcceleration(transportAccel);
    motor.move(steps);
    while (motor.run());
}

// =========================== Buttons ========================= //

// Config screen: stream the poti value and wait for a press.
void filmScanner::waitButton() {
    while (button.isPressed) button.read();
    unsigned long last = 0;
    while (true) {
        button.read();
        if (button.isPressed) { finishPress(); return; }
        poti.read();
        if (millis() - last > 1000) {
            last = millis();
            Serial.println((long)poti.getMap());
        }
    }
}

// Called right after a press edge. Waits for release, then a short window for
// a second tap. Reboots if a tap is held for holdReboot.
filmScanner::Event filmScanner::finishPress() {
    const unsigned long gap = 250;

    unsigned long down = millis();
    while (button.isPressed) {
        button.read();
        if (millis() - down >= holdReboot) reboot();
    }

    unsigned long up = millis();
    while (millis() - up < gap) {
        button.read();
        if (button.isPressed) {
            down = millis();
            while (button.isPressed) {
                button.read();
                if (millis() - down >= holdReboot) reboot();
            }
            return EV_DOUBLE;
        }
    }
    return EV_SINGLE;
}

// Interruptible delay (no motor running). Returns false if A is pressed;
// `pending` holds the classified press.
bool filmScanner::hold(unsigned long ms) {
    unsigned long t0 = millis();
    while (millis() - t0 < ms) {
        button.read();
        if (button.isPressed) { pending = finishPress(); return false; }
    }
    return true;
}

void filmScanner::reboot() {
    solenoid.release();
    Serial.println(F("reboot"));
    delay(500);
    Serial.flush();
    wdt_enable(WDTO_15MS);
    while (true) {}
}
