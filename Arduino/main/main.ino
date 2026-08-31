
// ======================== Changelog =========================== //
//  Date        Version     Comments
//  30.01.26    0.0         Cleanup
//  31.08.26    0.1         New control board pinout, single button, solenoid film clamp
//  31.08.26    0.2         State machine: single/multi shot, measured width + pitch,
//                          rescan on double press, hold-to-reboot
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
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include "Buttons.h"
#include "Solenoid.h"
#include "FilmScanner.h"
#include <AccelStepper.h>
#include <TMCStepper.h>

// ======================== Tunables ========================== //
// Every user-adjustable parameter lives here. These are pushed into the
// scanner / solenoid objects by applyTunables() in setup().

// --- Stepper driver ---
#define MOTOR_CURRENT_MA     1000    // TMC2209 RMS current [mA]
#define TRANSPORT_MICROSTEP  8       // microstepping used for transport + jog

// --- Film transport moves (moveFrame) ---
#define TRANSPORT_SPEED      10000   // transport speed [steps/s]
#define TRANSPORT_ACCEL      2000    // transport accel [steps/s^2]

// --- Multi-shot capture sweep ---
#define SCAN_SPEED           10000   // capture-sweep speed [steps/s]
#define SCAN_SPEED_SLOW      5000    // capture-sweep speed on a rescan [steps/s]

// --- Poti jog positioning (dynamicPosition) ---
#define JOG_RANGE            400     // poti throw -> +/- this many steps of fine travel
#define JOG_SPEED            8000    // jog speed [steps/s]
#define JOG_ACCEL            2000    // jog accel while tracking the poti [steps/s^2]
#define REST_SPEED           1000    // motor max speed restored after a jog [steps/s]
#define REST_ACCEL           40000   // motor accel restored after a jog [steps/s^2]

// --- Timing / delays ---
#define EXPOSURE_TIME        50     // camera exposure wait [ms]
#define SETTLE_TIME          10     // settle before/after film clamp [ms]
#define HOLD_REBOOT_TIME     2000    // button hold that triggers a reboot [ms]

// --- Solenoid film clamp ---
#define SOL_KICK_DUTY        255     // pull-in PWM duty [0-255]
#define SOL_HOLD_DUTY        40      // hold PWM duty [0-255]
#define SOL_KICK_TIME        50     // pull-in duration [ms]

// --- IR shutter ---
#define IR_SHUTTER_REPEATS   3       // shutter command repeats per photo
#define IR_SHUTTER_GAP       40      // delay between repeats [ms]

// ======================== Pindef ============================= //

// Inputs / outputs
#define PIN_IR      4
#define IR_SEND_PIN PIN_IR
#define PIN_SOL     5  // solenoid PWM
#define PIN_BTN     3
#define PIN_POTI    A0

// Motor driver pins (TMC2209)
#define PIN_EN     2
#define PIN_STEP   8
#define PIN_DIR    9
#define R_SENSE    0.11f
#define DRIVER_ADDRESS 0b00 // MS1/MS2 set to GND (0b00)
#define PIN_TMC2209_RX 6 // D6 <- TMC2209 TX (PDN_UART)
#define PIN_TMC2209_TX 7 // D7 -> TMC2209 RX (PDN_UART, through 1k)

// ====================== Object Definition ======================== //

Button button(PIN_BTN);
Poti poti(PIN_POTI, 0, 1023);
Solenoid solenoid(PIN_SOL);
IRsend irLED(IR_SEND_PIN); // IR LED V_f = 1.150, R = 200Ohm, I ~ 20mA
SoftwareSerial tmc_serial(PIN_TMC2209_RX, PIN_TMC2209_TX); // RX, TX
TMC2209Stepper driver(&tmc_serial, R_SENSE, DRIVER_ADDRESS);
AccelStepper motor(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);
filmScanner scanner(motor, driver, button, poti, irLED, solenoid);

// ======================== Main ============================= //

void setup() {
  MCUSR = 0;     // clear reset flags
  wdt_disable(); // drop any watchdog left set by a reboot gesture

	Serial.begin(9600);
  Serial.println("init");

  pinMode(PIN_IR, OUTPUT);
  digitalWrite(PIN_IR, LOW);
	initStepper();
  applyTunables();
  scanner.begin();
}

// Pushes the tunables above into the scanner and solenoid objects.
void applyTunables() {
  scanner.exposureTime = EXPOSURE_TIME;
  scanner.settleTime   = SETTLE_TIME;
  scanner.holdReboot   = HOLD_REBOOT_TIME;

  scanner.transportMicrostep = TRANSPORT_MICROSTEP;
  scanner.transportSpeed     = TRANSPORT_SPEED;
  scanner.transportAccel     = TRANSPORT_ACCEL;

  scanner.scanSpeed     = SCAN_SPEED;
  scanner.scanSpeedSlow = SCAN_SPEED_SLOW;

  scanner.jogRange  = JOG_RANGE;
  scanner.jogSpeed  = JOG_SPEED;
  scanner.jogAccel  = JOG_ACCEL;
  scanner.restSpeed = REST_SPEED;
  scanner.restAccel = REST_ACCEL;

  scanner.irShutterRepeats = IR_SHUTTER_REPEATS;
  scanner.irShutterGap     = IR_SHUTTER_GAP;

  solenoid.kickDuty = SOL_KICK_DUTY;
  solenoid.holdDuty = SOL_HOLD_DUTY;
  solenoid.kickTime = SOL_KICK_TIME;
}

void loop() {
  scanner.run();
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
    driver.rms_current(MOTOR_CURRENT_MA);
    driver.microsteps(TRANSPORT_MICROSTEP);
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
    motor.setMaxSpeed(REST_SPEED);        // reasonable default
    motor.setAcceleration(REST_ACCEL);     // ramp speed
    motor.setCurrentPosition(0);    // optional: reset to 0
}