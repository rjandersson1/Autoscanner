
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
#include <IRremote.hpp>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Buttons.h"
#include "FilmScanner.h"
// #include "libraries/AccelStepper.h"
#include <AccelStepper.h>
// #include "libraries/TMCStepper/TMCStepper.h"
#include <TMCStepper.h>
// #include "libraries/IRremote/IRremote.h"

// ======================== Pindef ============================= //

#define PIN_LED     9
#define PIN_IR      8
#define IR_SEND_PIN PIN_IR // Set the IR sending pin to Pin 9
#define SW_DN       A1
#define SW_UP       A2
#define PIN_BTN_A   A3  
#define PIN_BTN_B   A4  
#define PIN_BTN_C   A5  
#define PIN_POTI    A6  

// Define motor driver pins (TMC2209)
#define PIN_EN     2
#define PIN_STEP   3
#define PIN_DIR    4
#define R_SENSE    0.11f
#define DRIVER_ADDRESS 0b00 // MS1/MS2 set to GND (0b00)
#define PIN_TMC2209_RX 5 // D5 goes to TMC2209 TX (PDN_UART)
#define PIN_TMC2209_TX 6 // D6 goes to TMC2209 RX (PDN_UART)

// ===================== Button Callback Definition =================//

#define FUN_A1 scanner.takePhoto() //scanner.dynamicPosition()
#define FUN_A2 //scanner.calibrate()
#define FUN_A3 
#define FUN_AH //scanner.moveFrame()
#define TIME_AH 1000    // Hold time [ms]
#define TIME_A 0        // Multiclick time [ms]

#define FUN_B1 scanner.dynamicPosition()
#define FUN_B2 //scanner.moveFrame()
#define FUN_B3 //scanner.takePhoto()
#define FUN_BH 
#define TIME_BH 1000    // Hold time [ms]
#define TIME_B 0        // Multiclick time [ms]

#define FUN_C_TOGGLED 
#define FUN_C_WHILEON 
#define FUN_C_WHILEOFF
#define FUN_C_ON digitalWrite(PIN_IR, HIGH); digitalWrite(PIN_LED, HIGH) // Turn on IR LED
#define FUN_C_OFF digitalWrite(PIN_IR, LOW); digitalWrite(PIN_LED, LOW)  // Turn off IR LED

// ====================== Object Definition ======================== //

// Utility
Button buttonA(PIN_BTN_A);
Button buttonB(PIN_BTN_B);
toggleButton buttonC(PIN_BTN_C);
Poti poti(PIN_POTI, 0, 1023);
IRsend irLED(IR_SEND_PIN); // IR_LED V_f = 1.150, R = 200Ohm, I ~ 20mA

// Stepper motor
SoftwareSerial tmc_serial(PIN_TMC2209_RX, PIN_TMC2209_TX); // RX, TX
TMC2209Stepper driver(&tmc_serial, R_SENSE, DRIVER_ADDRESS); // Create TMC2209 driver object
AccelStepper motor(AccelStepper::DRIVER, PIN_STEP, PIN_DIR); // Create stepper motor object

// Film scanner
filmScanner scanner(motor, driver, buttonA, buttonB, buttonC, poti, irLED, PIN_LED); // Create film scanner object

// ======================== Main ============================= //

void setup() {
	Serial.begin(9600);
  initLEDs();
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

void initLEDs() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); 
  pinMode(PIN_IR, OUTPUT);
  digitalWrite(PIN_IR, LOW); 
}
// Initializes button callbacks

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
    pinMode(PIN_EN, OUTPUT); // Set enable pin as output
    digitalWrite(PIN_EN, LOW); // Enable driver (active low)
    // UART config for TMC2209
    tmc_serial.begin(115200);

    driver.begin();
    driver.toff(4);
    driver.blank_time(24);
    driver.rms_current(200);
    driver.microsteps(1);
    driver.pwm_autoscale(true);
    long startTime = millis();

    if (driver.test_connection() == 2) {
      Serial.println("TMC2209 drive VMOT = 0V, please check power supply!");
    }
    else if (driver.test_connection() == 0) {
      Serial.println("TMC2209 driver UART success!");
    }
    else {
      Serial.println("TMC2209 driver UART failed to initialize!");
      while(1);
    }

    // AccelStepper motion setup
    motor.setMaxSpeed(1000);        // reasonable default
    motor.setAcceleration(500);     // ramp speed
    motor.setCurrentPosition(0);    // optional: reset to 0
}