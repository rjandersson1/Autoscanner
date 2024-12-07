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

// ======================== Pindef ============================= //
#define PIN_A 10   // Button A Signal Pin
#define PIN_B 11   // Button B Signal Pin
#define PIN_C 12   // Button C Signal Pin
#define PIN_POTI A0 // Potentiometer Signal Pin

#define STP_EN 2
#define STP_MS1 3
#define STP_MS2 4
#define STP_MS3 5
#define STP_RST 6
#define STP_SLP 7
#define STP_STP 8
#define STP_DIR 9


// ====================== Motor Object ========================== //
// Stepper motor(STP_STP, STP_DIR, STP_EN, STP_MS1, STP_MS2, STP_MS3, STP_RST, STP_SLP, 200); // stepPin, dirPin, enablePin, ms1Pin, ms2Pin, ms3Pin, reset pin, sleep pin, stepsPerRevolution


// ====================== Button Objects ======================== //
// Poti poti;
Button buttonA(PIN_A), buttonB(PIN_B), buttonC(PIN_C);


// ======================== Main ============================= //


void setup() {
	Serial.begin(9600);

}

int microstepMode = 1;
void loop() {

  buttonA.update();
  buttonB.update();
  buttonC.update();
	// buttonA.print();

	// delay(20);
}

void setupButtons() {	
	// Set up buttons
	buttonA.setup(PIN_A);
	buttonA.debounceDelay = 10;
	buttonA.holdTime = 200;
	buttonB.setup(PIN_B);
	buttonB.debounceDelay = 30;
	buttonB.holdTime = 300;
	buttonC.setup(PIN_C);
	buttonC.debounceDelay = 60;
	buttonC.holdTime = 500;

	// // Set up potentiometer
	// poti.setupPoti(PIN_POTI);
}
