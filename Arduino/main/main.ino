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
// #include "stepper.h"
#include "buttons.h"

// ======================== Pindef ============================= //

#define PIN_MS1  13  // YL -> MS1
#define PIN_EN   12  // GR -> EN
#define PIN_DIR  11  // OR -> DIR
#define PIN_STEP 10  // RD -> STEP
#define PIN_IR   9   // BR -> IR LED
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

// Stepper
#include "A4988.h"
#define MOTOR_STEPS 200 // default steps/rev
#define RPM 1250 // cruise speed RPM
#define MOTOR_ACCEL 16000 // [STEP/s]
#define MOTOR_DECEL 9999999 // [STEP/s]
#define MICROSTEPS 16 // Microstep mode (hardwired)
A4988 stepper(MOTOR_STEPS, PIN_DIR, PIN_STEP, PIN_EN, PIN_MS1, PIN_MS2, PIN_MS3);


// ======================== Main ============================= //
short currentDir = 1;

void setup() {
	Serial.begin(9600);
	setupButtons();
	setupStepper();
	Serial.println("BOOT SUCCESSFUL");
}

void loop() {
	readButtons();
	readSerial();
}

// Reads and processes serial inputs
void readSerial() {
	if (!(Serial.available() > 0)) return; // exit if no input to serial
	// Read input as a string to handle edge cases
	String input = Serial.readStringUntil('\n');
	input.trim(); // Remove any trailing whitespace or newline characters

	// Process string with custom function
	processSerial(input);
}

// Processes serial input (change this function)
void processSerial(String input) {
//   float speed = input.toFloat();
//   stepper.setSpeed(speed);
//   Serial.print("Set speed to: [RPS] ");
//   Serial.println(speed);
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
	// buttonA.onSingleClick([] { stepper.enable(); stepper.rotate(360 * currentDir); });
	// buttonA.onWhileHeld([] { stepper.enable(); stepper.rotate(360 * currentDir); });
	buttonA.onSingleClick([] { stepper.setSpeedProfile(stepper.LINEAR_SPEED, poti.getMap(), 99999999); Serial.print("Accel: "); Serial.println(poti.getMap()); });
	// buttonA.onDoubleClick([] { stepper.setSpeedProfile(stepper.LINEAR_SPEED, poti.getMap(), poti.getMap()); });
	// buttonA.onSingleClick([] { stepper.setRPM(poti.getMap()); Serial.print("RPM: "); Serial.println(poti.getMap()); });
	// buttonA.onTripleClick([] { stepper.cycleMicrostepMode(); Serial.print("microstep: "); Serial.println(stepper.getMicrostep()); });
	buttonA.holdTime = 1000;
	buttonA.multiClickDelay = 0;
	
	// Button B
	buttonB.holdTime = 1000;
	buttonB.multiClickDelay = 0;
	buttonB.onSingleClick([] { stepper.rotate(20*360); });
	// Set RPM from poti
	// buttonB.onSingleClick([] { 
	// 	Serial.print("[B] setRPM: " );
	// 	poti.setMap(1,1000);
	// 	stepper.setRPM(poti.getMap());
	// 	Serial.println(poti.getMap());
	// });
	
	// buttonB.onDoubleClick([] { 
	// 	Serial.print("[B] setAccel: "); 
	// 	poti.setMap(100,10000);
	// 	short accel = short(poti.getMap());
	// 	stepper.setSpeedProfile(stepper.LINEAR_SPEED, accel, accel);
	// 	Serial.println(accel);
	// });

	// buttonB.onTripleClick([] { 
	// 	Serial.print("[B] microstep: "); 
	// 	stepper.cycleMicrostepMode();
	// 	Serial.println(stepper.getMicrostep());
	// });
	// buttonB.onWhileHeld([] { Serial.print("[B] "); Serial.println(poti.getPercent()); delay(10); });
	
	// Button C (toggle button)
	buttonC.whileOn([] { Serial.print("[C] "); Serial.println(poti.getMap()); delay(10); });
	// buttonC.toggledOff([] { stepper.disable(); });
	// buttonC.whileOn([] { Serial.print("[C] "); Serial.println(poti.getMap()); delay(10); });

	// Poti
	poti.setMap(10000,50000); // Set microseconds map

	Serial.println("Buttons initialized successfully");
}

void setupStepper() {
	stepper.begin(RPM, MICROSTEPS);
	stepper.setEnableActiveState(LOW); // Enabled when PIN_EN LOW
	stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL); // set acceleration profile
	stepper.setMicrostep(1);
	stepper.setOutputRatio(80,1);
	stepper.enable();
	Serial.println("Stepper initializedd successfully");
}