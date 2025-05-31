#include <TMCStepper.h>

// Pins
#define EN_PIN     2
#define STEP_PIN   3
#define DIR_PIN    4
#define SW_DN      A1
#define SW_UP      A2
#define BTN_A      A3
#define BTN_B      A4
#define BTN_C      A5
#define POTI       A6
#define R_SENSE    0.11f
#define DRIVER_ADDRESS 0b00

TMC2209Stepper driver(&Serial, R_SENSE, DRIVER_ADDRESS);

const uint16_t microsteps[] = {1, 2, 4, 8, 16};
uint8_t microstep_index = 0;

bool uartActive = true;
bool prev_SW_DN = HIGH;
bool prev_SW_UP = HIGH;
bool prev_BTN_A = HIGH;
bool prev_BTN_B = HIGH;
bool prev_BTN_C = HIGH;

bool constantDrive = false;
uint16_t targetCurrent = 300;
unsigned long lastStepTime = 0;

void setup() {
    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(SW_DN, INPUT_PULLUP);
    pinMode(SW_UP, INPUT_PULLUP);
    pinMode(BTN_A, INPUT_PULLUP);
    pinMode(BTN_B, INPUT_PULLUP);
    pinMode(BTN_C, INPUT_PULLUP);

    digitalWrite(DIR_PIN, HIGH);  // Set default direction
    digitalWrite(EN_PIN, LOW);    // Enable motor

    Serial.begin(115200);
    delay(500);

    driver.begin();
    driver.toff(4);
    driver.blank_time(24);
    driver.rms_current(targetCurrent);
    driver.microsteps(microsteps[microstep_index]);
    driver.pwm_autoscale(true);
}

void loop() {
    bool sw_dn = digitalRead(SW_DN) == LOW;
    bool sw_up = digitalRead(SW_UP) == LOW;
    bool btn_a = digitalRead(BTN_A) == LOW;
    bool btn_b = digitalRead(BTN_B) == LOW;
    bool btn_c = digitalRead(BTN_C) == LOW;

    // --- SW_DN = disable ---
    if (sw_dn) {
        digitalWrite(EN_PIN, HIGH);

    } else if (sw_up) {
        digitalWrite(EN_PIN, LOW);
    } else {
        driver.ihold(16);  // Optional: lower hold current when idle
    }

    // --- BTN_A: Cycle microstepping, apply on SW_UP ---
    if (btn_a && !prev_BTN_A) {
        microstep_index = (microstep_index + 1) % 5;
        if (!uartActive) Serial.begin(9600);
        Serial.print("Selected microstep: 1/");
        Serial.println(microsteps[microstep_index]);
        if (uartActive) Serial.end();
    }

    if (sw_up && !prev_SW_UP) {
        driver.microsteps(microsteps[microstep_index]);
        if (!uartActive) Serial.begin(9600);
        Serial.print("Applied microstep: 1/");
        Serial.println(microsteps[microstep_index]);
        if (uartActive) Serial.end();
    }

    // --- Constant drive movement: 10 deg/s ---
    if (constantDrive) {
        if (sw_dn) {
            constantDrive = false;
            digitalWrite(EN_PIN, HIGH); // Disable motor
            Serial.begin(9600);
            Serial.println("Constant drive stopped (SW_DN).");
        } else {
            float step_deg = 1.8f / microsteps[microstep_index];  // degrees per microstep
            float step_interval_ms = 1000.0f * (step_deg / 10.0f); // for 10°/s

            if (millis() - lastStepTime >= step_interval_ms) {
                digitalWrite(STEP_PIN, HIGH);
                delayMicroseconds(5);
                digitalWrite(STEP_PIN, LOW);
                lastStepTime = millis();
            }
        }
    }
    // --- BTN_B: Start constant drive if SW_UP, else just set current ---
    if (btn_b && !prev_BTN_B) {
        uint16_t current = map(analogRead(POTI), 0, 1023, 0, 600);

        if (!uartActive) Serial.begin(9600);
        Serial.print("Setting current to: ");
        Serial.print(current);
        Serial.println(" mA");
        if (uartActive) Serial.end();

        Serial.end(); delay(100);
        Serial.begin(115200); delay(100);
        driver.rms_current(current);

        if (sw_up) {
            constantDrive = true;
            digitalWrite(EN_PIN, LOW);  // Enable motor
            lastStepTime = millis();
            Serial.end();
        }
    }


    // --- BTN_C: toggle UART/Serial Monitor ---
    if (btn_c && !prev_BTN_C) {
        uartActive = !uartActive;
        Serial.println("SERIAL CLOSING");
        Serial.end();
        delay(100);
        if (!uartActive) {
            Serial.begin(9600);
            delay(100);
            Serial.println("\n.\n.\n.\nSERIAL OPENED");
        } else {
            Serial.begin(115200);
        }
    }

    // Update previous button states
    prev_SW_DN = sw_dn;
    prev_SW_UP = sw_up;
    prev_BTN_A = btn_a;
    prev_BTN_B = btn_b;
    prev_BTN_C = btn_c;

    delay(50);
}