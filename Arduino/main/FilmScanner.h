#pragma once
#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>
#include <AccelStepper.h>
#include <TMCStepper.h>
#include "Buttons.h"
#include "Solenoid.h"

// Flow (one full pass, driven by run() from loop()):
//  1  pick mode        poti <50 single shot, >=50 multi shot        A confirms
//  1b pick shot count  (multi only) poti <50 = 3, >=50 = 5          A confirms
//  2  dynamicPosition to image start                               A confirms
//  3  dynamicPosition to image end -> width known, stay put        A confirms
//  4  dynamicPosition to next image start -> pitch known, then the
//     single return move back to the first frame start             A confirms
//  5  standby: dynamicPosition to first frame start, press A to scan
//  6  scan frame, then move straight on to the next frame start,
//     dynamicPosition to adjust, press A for next
//
// Gestures:
//  single press during a scan   -> stop, back to standby
//  double press during scan/adv -> rescan this frame at half transport speed
//  hold A 3s (any time)         -> reboot, restart from step 1
//  a nudge before confirming the next frame folds into the pitch permanently,
//  unless it exceeds 1.5x frame width (treated as a reload / manual reposition)

class filmScanner {

public:

    filmScanner(AccelStepper &motor, TMC2209Stepper &driver, Button &button, Poti &poti, IRsend &irLED, Solenoid &solenoid);

    // Reference objects
    AccelStepper &motor;
    TMC2209Stepper &driver;
    Button &button;
    Poti &poti;
    IRsend &irLED;
    Solenoid &solenoid;

    // Tunables (set from applyTunables() in main.ino)
    long exposureTime = 650;          // camera exposure wait [ms]
    long settleTime = 150;            // settle before/after clamp [ms]
    unsigned long holdReboot = 3000;  // A hold time that triggers reboot [ms]

    int   transportMicrostep = 8;     // microstepping for transport + jog
    float transportSpeed = 10000;     // transport speed [steps/s]
    float transportAccel = 2000;      // transport accel [steps/s^2]

    float scanSpeed = 10000;          // multi-shot capture-sweep speed [steps/s]
    float scanSpeedSlow = 5000;       // capture-sweep speed on a rescan [steps/s]

    long  jogRange = 400;             // poti throw -> +/- steps of fine travel
    float jogSpeed = 8000;            // jog speed [steps/s]
    float jogAccel = 2000;            // jog accel while tracking the poti [steps/s^2]
    float restSpeed = 1000;           // motor max speed restored after a jog [steps/s]
    float restAccel = 40000;          // motor accel restored after a jog [steps/s^2]

    int irShutterRepeats = 3;         // shutter command repeats per photo
    int irShutterGap = 40;            // delay between repeats [ms]

    void begin();   // one-time init, call from setup()
    void run();     // advances the state machine, call from loop()

private:

    enum State {
        CFG_MODE, CFG_COUNT,
        CAL_IMG_START, CAL_IMG_END, CAL_PITCH,
        STANDBY, SCAN_FRAME, ADVANCE, RESCAN
    };
    enum Event { EV_SINGLE, EV_DOUBLE };

    State state = CFG_MODE;

    bool multiShot = false;
    int  shotCount = 1;      // 1, 3 or 5 (multi adds an overshoot each edge)
    int  direction = 1;      // transport sign, from the width calibration
    long imageWidth = 0;
    long framePitch = 0;
    long frameStart = 0;     // motor position at the current frame start
    bool redoHalfSpeed = false;

    Event pending = EV_SINGLE; // classified press from an interrupted capture

    // State handlers
    void cfgMode();
    void cfgCount();
    void calImageStart();
    void calImageEnd();
    void calPitch();
    void standby();
    void scanFrameState();
    void advance();
    void rescan();

    // Movement (unchanged from the pre-rework firmware)
    long dynamicPosition();                       // poti positioning, returns motor position
    void moveFrame(long steps, long speed = -1);  // blocking relative move (-1 = transportSpeed)

    // Scan cycle
    bool scanImage(bool slow);
    bool frameCapture();
    void takePhoto();

    // Buttons
    void  waitButton();          // config screen: stream poti, wait for press
    Event finishPress();         // after a press edge: release wait + double detect + reboot
    bool  hold(unsigned long ms); // interruptible delay, no motor running
    void  reboot();
};
