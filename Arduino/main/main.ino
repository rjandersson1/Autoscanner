
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

// ===================== Button Callback Definition =================//

#define TEMP_AMOUNT 1 // amount to change RPM by

#define FUN_A1 updateRPM((-TEMP_AMOUNT))
#define FUN_A2 
#define FUN_A3 
#define FUN_AH 
#define TIME_AH 1000
#define TIME_A 0


#define FUN_B1 updateRPM(TEMP_AMOUNT)
#define FUN_B2 
#define FUN_B3 
#define FUN_BH stepper.cycleMicrostepMode()
#define TIME_BH 1000
#define TIME_B 0

#define FUN_C_TOGGLED 
#define FUN_C_WHILEON 
#define FUN_C_ON dynamicMove()
int updateDelay = 100; // [uS]
#define FUN_C_OFF 


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
	Serial.begin(115200);
	setupButtons();
	setupStepper();
	Serial.println("init...");
}

void loop() {
	readButtons();
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
  buttonA.onSingleClick([] { FUN_A1; });
  buttonA.onDoubleClick([] { FUN_A2; });
  buttonA.onTripleClick([] { FUN_A3; });
  buttonA.onHoldStart([] { FUN_AH; });
  buttonA.holdTime = TIME_AH;
  buttonA.multiClickDelay = TIME_A;
  
  // Button B
  buttonB.onSingleClick([] { FUN_B1; });
  buttonB.onDoubleClick([] { FUN_B2; });
  buttonB.onTripleClick([] { FUN_B3; });
  buttonB.onHoldStart([] { FUN_BH; });
  buttonB.holdTime = TIME_BH;
  buttonB.multiClickDelay = TIME_B;

  // Button C
  buttonC.onToggle([] { FUN_C_TOGGLED; });
  buttonC.whileOn([] { FUN_C_WHILEON; });
  buttonC.toggledOn([] { FUN_C_ON; });
  buttonC.toggledOff([] { FUN_C_OFF; });
}

void setupStepper() {
	stepper.begin(RPM, MICROSTEPS);
	stepper.setEnableActiveState(LOW); // Enabled when PIN_EN LOW
	stepper.setSpeedProfile(stepper.LINEAR_SPEED, MOTOR_ACCEL, MOTOR_DECEL); // set acceleration profile
	stepper.setMicrostep(1);
	stepper.enable();
}

void dynamicMove() {
	poti.setMap(0,400);
	stepper.startMove(100000);
	// stepper.setRPM(100);
	Serial.println(stepper.getMicrostep());
	// stepper.setMicrostep(8);
	float prevRPM = 0;
	unsigned lastUpdate = micros();
	while (buttonC.state) {
		unsigned wait_time_micros = stepper.nextAction();
		buttonA.read();
		buttonB.read();
		buttonC.read();
	}
}


// Moves to current poti defined position.
int prevRPM = 0;
void dynamicPosition() {
	poti.setMap(5,200);
	stepper.setMicrostep(16);
	prevRPM = newRPM;
	int newRPM = map(analogRead(buttonC.pin), 0, 1023, 0, 200);

	stepper.setSpeedProfile(stepper.LINEAR_SPEED, 16000, 999999);

	int targetPos = poti.getMap();
	int prevPos = 0;
	int posThreshold = 1;

	stepper.startRotate(targetPos);
	while (buttonC.state) {
		targetPos = poti.getMap();
		if (abs(targetPos - prevPos) > posThreshold) {
			stepper.startRotate(targetPos);
		}
		buttonC.read();
	}
}

// Temp function to read a val from poti.
void changeUpdateDelay() {
	poti.setMap(0,50000);
	updateDelay = round(poti.getMap());
	Serial.print("delay [uS] = ");
	Serial.println(updateDelay);
}


int prevRPM = 0;
int newRPM = 0;
void updateRPM(int amount) {
	stepper.enable();
	prevRPM = newRPM;
	newRPM = prevRPM + amount;
	Serial.println(newRPM);
	stepper.setRPM(newRPM);
	int dir = 0;
	if (amount > 0) dir = 1;
	if (amount < 0) dir = -1;
	stepper.startMove(1000000 * dir);
}
