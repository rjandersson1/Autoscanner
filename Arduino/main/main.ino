
// ======================== Changelog =========================== //
//  Date        Version     Comments
//  02.12.24    0.0         Init
//
//
//
//

// ============================= TODO =========================== //
//	- build AutoScanner methods within stepper.h
//
//
//
//
//



// ======================== Libraries =========================== //
#include <Arduino.h>
#include <IRremote.h>
// #include "stepper.h"
#include "buttons.h"
#include "filmScanner.h"

// ======================== Pindef ============================= //

#define PIN_MS1  13  // YL -> MS1
#define PIN_EN   12  // GR -> EN
#define PIN_DIR  11  // OR -> DIR
#define PIN_STEP 10  // RD -> STEP
#define PIN_IR   9   // BR -> IR LED
#define IR_SEND_PIN PIN_IR // Set the IR sending pin to Pin 9
#define PIN_SLP  8   // BR -> SLP
#define PIN_RST  7   // BK -> RST
#define PIN_MS3  6   // WH -> MS3
#define PIN_MS2  5   // GY -> MS2
#define PIN_BTN_A 4  // GY -> BUTTON A
#define PIN_BTN_B 3  // WH -> BUTTON B
#define PIN_BTN_C 2  // BK -> BUTTON C
#define PIN_POTI A0  // RD -> POTI

// ====================== Object Definition ======================== //

// Utility
Button buttonA(PIN_BTN_A);
Button buttonB(PIN_BTN_B);
toggleButton buttonC(PIN_BTN_C);
Poti poti(PIN_POTI, 0, 1023);
IRsend ir(IR_SEND_PIN); // IR object
Timer timer(); // timer object

// Stepper
#include "A4988.h"
#define MOTOR_STEPS 200 // default steps/rev
#define RPM 1250 // cruise speed RPM
#define MOTOR_ACCEL 16000 // [STEP/s]
#define MOTOR_DECEL 9999999 // [STEP/s]
#define MICROSTEPS 16 // Microstep mode (hardwired)
A4988 stepper(MOTOR_STEPS, PIN_DIR, PIN_STEP, PIN_EN, PIN_MS1, PIN_MS2, PIN_MS3);

// Film scanner
filmScanner scanner(stepper, buttonA, buttonB, buttonC, poti);

// ======================== Main ============================= //

void setup() {
	Serial.begin(9600);
	setupButtons();
	setupStepper();
	Serial.println("init...");
}

void loop() {
	readButtons();
	readSerial();
}

// Reads button states
void readButtons() {
	buttonA.read();
	buttonB.read();
	buttonC.read();
	poti.read();
}

void setupButtons() {	
	// Button A
	// buttonA.onSingleClick([] { stepper.disable(); });
	// buttonA.onHoldStart([] { stepper.disable(); });
	buttonA.onSingleClick(testMillis);
	buttonA.holdTime = 500;
	buttonA.multiClickDelay = 0;
	
	// Button B
	buttonB.holdTime = 1000;
	buttonB.multiClickDelay = 0;
	// buttonB.onSingleClick([] { stepper.cycleMicrostepMode(); });
	buttonB.onSingleClick(testMicros);
	

	// Button C (toggle button)
	buttonC.toggledOn([] { scanner.dynamicMove(); });

	// Poti
	// poti.setMap(10000,50000); // Set microseconds map
}

void setupStepper() {
	stepper.begin(RPM, MICROSTEPS);
	stepper.setEnableActiveState(LOW); // Enabled when PIN_EN LOW
	stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL); // set acceleration profile
	stepper.setMicrostep(1);
	stepper.enable();
}

void testMillis() {
	timer.startMillis();
	timer.endMillis();
}

void testMicros() {
	timer.startMicros();
	timer.endMicros();
}