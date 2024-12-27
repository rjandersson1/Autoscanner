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
#define IR_SEND_PIN 9 // Set the IR sending pin to Pin 9
// #inclide <Arduino.h>
// #include "A4988.h"
// #include "stepper.h"
#include "buttons.h"
#include "IRControl.h"
#include <IRremote.h>

// ======================== Pindef ============================= //
#define PIN_A 3   // Button A Signal Pin
#define PIN_B 2   // Button B Signal Pin
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

IRsend irsend;
int pin_LED = 9;

// ======================== Main ============================= //



void setup() {
	Serial.begin(9600);
	setupButtons();
	// pinMode(pin_LED, OUTPUT);
  // irsend.begin(); // Initialize the IR sender

}

uint32_t command = 0;
void loop() {

  buttonA.read();
  buttonB.read();

  if (Serial.available()) {
	command = A7R.readHexFromSerial();
	Serial.println(command);
  }

	// delay(20);
}

void setupButtons() {	
	// Set up buttons
	// Button A
	// buttonA.onSingleClick([] { A7R.sendCommand(command); });
  buttonA.onSingleClick([] { sendShutter(); });
	// buttonA.onSingleClick([] { Serial.println("clickA"); });
	// buttonB.onSingleClick([] { Serial.println("clickB"); });
	buttonB.onSingleClick([] { digitalWrite(pin_LED, HIGH); });
	buttonB.onDoubleClick([] { digitalWrite(pin_LED, LOW); });
}

void sendShutter() {
  Serial.println("Sending CMD");
  for (int i = 0; i < 3; i++) {
    irsend.sendSony(0xB4B8F, 20); // Send Sony 12-bit command
    delay(40);
  }
  
}

