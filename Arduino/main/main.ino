
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

#define TEMP_AMOUNT 10 // amount to change RPM by

float buttonRPM = 64;

#define FUN_A1 buttonRPM = buttonRPM*2; Serial.println(buttonRPM)
#define FUN_A2 buttonRPM = buttonRPM/2; Serial.println(buttonRPM)
#define FUN_A3 
#define FUN_AH 
#define TIME_AH 1000
#define TIME_A 500


#define FUN_B1 stepper.cycleMicrostepMode()
#define FUN_B2 
#define FUN_B3 
#define FUN_BH stepper.cycleMicrostepMode()
#define TIME_BH 500
#define TIME_B 0

#define FUN_C_TOGGLED 
#define FUN_C_WHILEON 
#define FUN_C_ON dynamicPosition()
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

int prevUpdateValue = 0;  // Store the last value at which updateRPM was called
int newRPM2 = 0;
int prevRPM2 = 0;
void dynamicMove() {
	poti.setMap(-50, 50);
	prevRPM2 = newRPM2;
	newRPM2 = poti.getMap();
	// stepper.setMicrostep(8);
	stepper.setRPM(0);
	Serial.println(stepper.getMicrostep());
	unsigned lastUpdate = micros();

	while (buttonC.state) {
		prevRPM2 = newRPM2;
		newRPM2 = int(poti.getMap());

		// Check if the change in RPM is at least 10
		if (abs(newRPM2 - prevUpdateValue) >= 10) {
			updateRPM(newRPM2 / 10);
			prevUpdateValue = newRPM2;  // Update the last update value
		}

		unsigned wait_time_micros = stepper.nextAction();

		buttonA.read();
		buttonB.read();
		buttonC.read();
	}
}



#define WINDOW_SIZE 5  // Number of samples to average
// Moves to current poti defined position.
void dynamicPosition() {
	int mapVal = 400;
	int delayTime = 4096;
	int offset = 0;
	int threshold = 2;  // Threshold to filter small changes (adjust as needed)
	int readings[WINDOW_SIZE] = {0};  // Array to store recent readings
	int index = 0;  // Index for circular buffer
	int sum = 0;    // Sum of readings in the window
	int average = 0; // Averaged potentiometer value

    poti.setMap(-mapVal, mapVal);  // Set the mapping range for the potentiometer
    int currentPosition = 0;  // Variable to track the current stepper position
    int targetPosition = 0;   // Variable to store target position
	// stepper.setRPM((16 / stepper.getMicrostep()) * 10);
	stepper.setRPM(100);
	stepper.setMicrostep(16);

    while (buttonC.state) {  // Loop while the button is pressed
        buttonC.read();  // Update the button state
		buttonA.read();
        int newReading = poti.getMap();  // Get the mapped potentiometer value
		stepper.setRPM(buttonRPM);
        
        // Update moving average
        sum -= readings[index];         // Subtract the oldest reading
        readings[index] = newReading;   // Store new reading
        sum += newReading;              // Add new reading to sum
        index = (index + 1) % WINDOW_SIZE;  // Circular buffer index
        average = sum / WINDOW_SIZE;    // Calculate average

        // Only update the target position if the change exceeds the threshold
        if (abs(average - targetPosition) > threshold) {
            targetPosition = average;
        }

        // Handle constant move at edge of poti
        if (abs(newReading) > mapVal * 0.99) {
			offset = 0; // reset offset count
            if (targetPosition > 1) offset += 1; // start move right
            if (targetPosition < 1) offset -= 1; // start move left
			delayMicroseconds(delayTime); // delay to reduce RPM
        } else {
            offset = 0;
        }

        
        int stepsToMove = targetPosition + offset - currentPosition;  // Calculate steps to move
		
        stepper.move(stepsToMove);  // Move stepper by the calculated steps
        currentPosition = targetPosition;  // Update the current position
    }
}

int prevRPM = 0;
int newRPM = 0;
void updateRPM(int amount) {
	stepper.enable();
	// Serial.println(amount);
	stepper.setRPM(abs(5*amount));
	int dir = 0;
	if (amount > 0) dir = 1;
	if (amount < 0) dir = -1;
	stepper.startMove(1000000 * dir);
}



