
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
#include <SoftwareSerial.h>
#include "libraries/buttons.h"
#include "libraries/filmScanner.h"
#include "libraries/TMC/TMCStepper.h"
#include "libraries/IRremote/IRremote.h"

// ======================== Pindef ============================= //

#define PIN_IR      12
#define PIN_LED     8
#define IR_SEND_PIN PIN_IR // Set the IR sending pin to Pin 9
#define PIN_BTN_A   A3  
#define PIN_BTN_B   A0  
#define PIN_BTN_C   A5  
#define PIN_POTI    A6  
#define SW_DN       A1
#define SW_UP       A2

// Define motor driver pins (TMC2209)
#define PIN_EN     2
#define PIN_STEP   3
#define PIN_DIR    4
#define R_SENSE    0.11f
#define DRIVER_ADDRESS 0b00 // MS1/MS2 set to GND (0b00)
#define PIN_TMC2209_RX 5 // D5 goes to TMC2209 TX (PDN_UART)
#define PIN_TMC2209_TX 6 // D6 goes to TMC2209 RX (PDN_UART)

// ===================== Button Callback Definition =================//

#define FUN_A1 scanner.takePhoto() // Take photo
#define FUN_A2 
#define FUN_A3 
#define FUN_AH 
#define TIME_AH 1000    // Hold time [ms]
#define TIME_A 0        // Multiclick time [ms]

#define FUN_B1 
#define FUN_B2 
#define FUN_B3 
#define FUN_BH 
#define TIME_BH 1000    // Hold time [ms]
#define TIME_B 0        // Multiclick time [ms]

#define FUN_C_TOGGLED 
#define FUN_C_WHILEON 
#define FUN_C_WHILEOFF
#define FUN_C_ON 
#define FUN_C_OFF 

// ====================== Object Definition ======================== //

// Utility
Button buttonA(PIN_BTN_A);
Button buttonB(PIN_BTN_B);
toggleButton buttonC(PIN_BTN_C);
Poti poti(PIN_POTI, 0, 1023);
IRsend ir(IR_SEND_PIN);

// Stepper motor
SoftwareSerial tmc_serial(PIN_TMC2209_RX, PIN_TMC2209_TX); // RX, TX
TMC2209Stepper stepper(&tmc_serial, R_SENSE, DRIVER_ADDRESS);

// Film scanner
filmScanner scanner(stepper, buttonA, buttonB, buttonC, poti, ir);

// ======================== Main ============================= //

void setup() {
	Serial.begin(9600);
	initButtons();
	initStepper();
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

void initButtons() {	
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

// Initializes the TMC2209 stepper driver
void initStepper() {
  tmc_serial.begin(115200); // Initialize UART communication
  stepper.begin();
  stepper.toff(4);
  stepper.blank_time(24);
  stepper.rms_current(100);
  stepper.microsteps(1);
  stepper.pwm_autoscale(true);
}