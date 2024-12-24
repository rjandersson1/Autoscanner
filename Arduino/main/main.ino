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
// #inclide <Arduino.h>
// #include "A4988.h"
#include "stepper.h"
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
Stepper motor(STP_STP, STP_DIR, STP_EN, STP_MS1, STP_MS2, STP_MS3, STP_RST, STP_SLP, 200); // stepPin, dirPin, enablePin, ms1Pin, ms2Pin, ms3Pin, reset pin, sleep pin, stepsPerRevolution


// ====================== Button Objects ======================== //
// Poti poti;
Button buttonA(PIN_A);
toggleButton buttonB(PIN_B);
toggleButton buttonC(PIN_C);
Poti poti(PIN_POTI, 0, 1023);


// ======================== Main ============================= //


void setup() {
	Serial.begin(9600);
	setupButtons();

}

int microstepMode = 1;
void loop() {

  buttonA.read();
  buttonB.read();
  buttonC.read();
  poti.read();

  if (Serial.available() > 0) {
        // Read input as a string to handle edge cases
        String input = Serial.readStringUntil('\n');
        input.trim(); // Remove any trailing whitespace or newline characters

        // Convert input to a float
        float speed = input.toFloat();

		// Update motor speed
		motor.setSpeed(speed);

		// Print update
		Serial.print("Set speed to: ");
		Serial.println(speed);
  }

//   delay(10);
}

void setupButtons() {	
	// Set up buttons
	// Button A
	buttonA.onSingleClick([] { motor.moveStep(); });
	buttonA.onDoubleClick([] { motor.moveSteps(10); });
	buttonA.onTripleClick(changeMicrostepping);
	buttonA.onHold([] { motor.moveToDegree(poti.getDegrees()); } );
	// buttonA.onHold([] { Serial.println(poti.getDegrees()); } );
	buttonA.holdTime = 1000;
	// Button B
	buttonB.whileOn([] { motor.printStatus(); });
	
	// buttonB.onHold([] { Serial.println(poti.getAnalog()); }); // lambda function
	// Button C
	buttonC.toggledOn([] { motor.enableMotor(); });
	buttonC.toggledOff([] { motor.disableMotor(); });



	// Setup poti


	// // Set up potentiometer
	// poti.setupPoti(PIN_POTI);
}

void changeMicrostepping() {
	microstepMode = microstepMode * 2;
	if (microstepMode > 16) microstepMode = 1;
	motor.setMicrostepMode(microstepMode);
	Serial.println(microstepMode);
}