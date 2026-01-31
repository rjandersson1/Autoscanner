
// ======================== Changelog =========================== //
//  Date        Version     Comments
//  30.01.26    0.0         Cleanup
//
//
//
//

// ============================= TODO =========================== //
//	
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
#include <AccelStepper.h>
#include <TMCStepper.h>

// ======================== Settings =========================== //
#define DEFAULT_CURRENT 1000 // RMS current in mA
#define DEFAULT_MICOSTEP 0
#define DEFAULT_SPEED 200
#define DEFAULT_ACCEL 200

// ======================== Pindef ============================= //

// Define input pins
#define PIN_LED     9
#define PIN_IR      8
#define IR_SEND_PIN PIN_IR // Set the IR sending pin to Pin 9
#define PIN_BTN_A   A3  
#define PIN_POTI    A6  

// Define motor driver pins (TMC2209)
#define PIN_EN     2
#define PIN_STEP   3
#define PIN_DIR    4
#define R_SENSE    0.11f
#define DRIVER_ADDRESS 0b00 // MS1/MS2 set to GND (0b00)
#define PIN_TMC2209_RX 5 // D5 goes to TMC2209 TX (PDN_UART)
#define PIN_TMC2209_TX 6 // D6 goes to TMC2209 RX (PDN_UART)

// Legacy buttons (pins are correct, add for future expansion)
// #define SW_DN       A1
// #define SW_UP       A2
// #define PIN_BTN_B   A4  
// #define PIN_BTN_C   A5  

// ====================== Object Definition ======================== //

Button buttonA(PIN_BTN_A);
Poti poti(PIN_POTI, 0, 1023);
IRsend irLED(IR_SEND_PIN); // IR_LED V_f = 1.150, R = 200Ohm, I ~ 20mA
SoftwareSerial tmc_serial(PIN_TMC2209_RX, PIN_TMC2209_TX); // RX, TX
TMC2209Stepper driver(&tmc_serial, R_SENSE, DRIVER_ADDRESS); // Create TMC2209 driver object
AccelStepper motor(AccelStepper::DRIVER, PIN_STEP, PIN_DIR); // Create stepper motor object
filmScanner scanner(motor, driver, buttonA, poti, irLED, PIN_LED); // Create film scanner object

// ======================== Main ============================= //

void setup() {
	Serial.begin(9600);
  Serial.println("init");
  
  // init LEDS
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); 
  pinMode(PIN_IR, OUTPUT);
  digitalWrite(PIN_IR, LOW);
	initStepper();
}

void loop() {
  scanner.setup();
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
    driver.rms_current(DEFAULT_CURRENT);
    driver.microsteps(DEFAULT_MICOSTEP);
    driver.pwm_autoscale(true);

    if (driver.test_connection() == 2) {
      Serial.println("Driver VMOT = 0V!");
    }
    else if (driver.test_connection() == 0) {
      Serial.println("UART success!");
    }
    else {
      Serial.println("UART failed to init!");
      while(1);
    }

    // AccelStepper motion setup
    motor.setMaxSpeed(DEFAULT_SPEED);        // reasonable default
    motor.setAcceleration(DEFAULT_ACCEL);     // ramp speed
    motor.setCurrentPosition(0);    // optional: reset to 0
}