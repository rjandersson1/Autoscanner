// ======================== Changelog =========================== //
//  Date        Version     Comments
//  02.12.24    0.0         Init
//
//
//
//

// ============================= TODO =========================== //
//  - build struct outline
//  - attach buttons
//	- build wiring schematic
//
//
//
//
//
//



// ======================== Libraries =========================== //
#include <Arduino.h>
// #include "A4988.h"
#include "stepper.h"
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
Stepper motor(PIN_STEP, PIN_DIR, PIN_EN, PIN_MS1, PIN_MS2, PIN_MS3, PIN_RST, PIN_SLP, 200); // 200 steps per revolution


// ======================== Main ============================= //


void setup() {
	Serial.begin(9600);
	setupButtons();

}

void loop() {
	readButtons();
	readSerial();
}

// Reads and processes serial inputs
void readSerial() {
	if (~(Serial.available() > 0)) return; // exit if no input to serial
	// Read input as a string to handle edge cases
	String input = Serial.readStringUntil('\n');
	input.trim(); // Remove any trailing whitespace or newline characters

	// Process string with custom function
	processSerial(input);
}

// Processes serial input (change this function)
void processSerial(String input) {

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
	buttonA.onSingleClick([] { Serial.println("[A] S"); });
	buttonA.onHoldStart([] { Serial.println("[A] H"); });
	buttonA.holdTime = 1000;
	
	// Button B
	buttonB.onSingleClick([] { Serial.println("[B] 1"); });
	buttonB.onDoubleClick([] { Serial.println("[B] 2"); });
	buttonB.onTripleClick([] { Serial.println("[B] 3"); });
	buttonB.onWhileHeld([] { Serial.print("[B] "); Serial.println(poti.getPercent()); delay(10); });
	buttonB.holdTime = 500;
	buttonB.multiClickDelay = 300;
	
	// Button C (toggle button)
	buttonC.toggledOn([] { Serial.println("[C] 1"); });
	buttonC.toggledOff([] { Serial.println("[C] 0"); });
	buttonC.whileOn([] { Serial.print("[C] "); Serial.println(poti.getMap()); delay(10); });

	// Poti
}