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
//
//
//
//
//
//
//



// ======================== Libraries =========================== //
// #inclide <Arduino.h>
// #include "A4988.h"
// #include "stepper.h"
#include "buttons.h"
#include "IRControl.h"

// ======================== Pindef ============================= //
#define PIN_A 2   // Button A Signal Pin
#define PIN_B 3   // Button B Signal Pin
// #define PIN_C 12   // Button C Signal Pin
// #define PIN_POTI A0 // Potentiometer Signal Pin

// #define STP_EN 2
// #define STP_MS1 3
// #define STP_MS2 4
// #define STP_MS3 5
// #define STP_RST 6
// #define STP_SLP 7
// #define STP_STP 8
// #define STP_DIR 9


// ====================== Motor Object ========================== //
// Stepper motor(STP_STP, STP_DIR, STP_EN, STP_MS1, STP_MS2, STP_MS3, STP_RST, STP_SLP, 200); // stepPin, dirPin, enablePin, ms1Pin, ms2Pin, ms3Pin, reset pin, sleep pin, stepsPerRevolution


// ====================== Objects ======================== //
Button buttonA(PIN_A), buttonB(PIN_B);
IRControl A7R(9, 40000);


// ======================== Main ============================= //


void setup() {
	Serial.begin(9600);
	setupButtons();

}

utint32_t command = 0;
void loop() {

  buttonA.read();
  buttonB.read();

  if (Serial.available()) {
	command = A7R.readHexFromSerial();
  }

	// delay(20);
}

void setupButtons() {	
	// Set up buttons
	buttonA.onSingleClick
	buttonA.onPress(printPress);
	buttonA.onRelease(printRelease);
	buttonA.onHold(printHold);

	buttonA.onSingleClick(printSingle);
	buttonA.onDoubleClick(printDouble);
	buttonA.onTripleClick(printTriple);

	// // Set up potentiometer
	// poti.setupPoti(PIN_POTI);
}

void printPress() {
	// Serial.println("pr");
}

void printRelease() {
	// Serial.println("rl");
}

void printHold() {
	// Serial.println("hl");
}

void printSingle() {
	Serial.println("1c");
}

void printDouble() {
	Serial.println("2c");
}

void printTriple() {
	Serial.println("3c");
}