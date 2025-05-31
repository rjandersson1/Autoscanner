// fastest speed: ~800deg/s @ 600mA 1/1
// lowest current: ~75mA


#include <TMCStepper.h>
#include <SoftwareSerial.h>

// ---------------------------------------------------------
// Pin definitions
// ---------------------------------------------------------

// Define motor driver pins (TMC2209)
#define EN_PIN     2
#define STEP_PIN   3
#define DIR_PIN    4
#define R_SENSE    0.11f
#define DRIVER_ADDRESS 0b00 // MS1/MS2 set to GND (0b00)
#define TMC2209_RX 5 // D5 goes to TMC2209 TX (PDN_UART)
#define TMC2209_TX 6 // D6 goes to TMC2209 RX (PDN_UART)

// Define io pins
#define LED_GR       8 // green LED
#define LED_IR       12 // red LED
#define SW_DN      A1
#define SW_UP      A2
#define BTN_A      A3
#define BTN_B      A0
#define BTN_C      A5
#define POTI       A6

// ---------------------------------------------------------
// IO initialization
// ---------------------------------------------------------

// State tracking for downtrigger detection
bool prev_SW_DN = HIGH;
bool prev_SW_UP = LOW;
bool prev_BTN_A = HIGH;
bool prev_BTN_B = HIGH;
bool prev_BTN_C = HIGH;

// ---------------------------------------------------------
// TMC2209 driver initialization
// ---------------------------------------------------------
SoftwareSerial tmc_serial(TMC2209_RX, TMC2209_TX); // RX, TX
TMC2209Stepper driver(&tmc_serial, R_SENSE, DRIVER_ADDRESS);
bool uartActive = true;

// Microstepping settings
const uint16_t microsteps[] = {1, 2, 4, 8, 16};
uint8_t microstep_index = 0;

// Motor control settings
uint16_t targetCurrent = 300;
int maxCurrent = 600; // mA
int maxSpeed = 1100; // deg/sec
float speed = 10.0f; // degrees per second


// ---------------------------------------------------------
// Setup function
// ---------------------------------------------------------
void setup() {
    // Initialize LED pins
    pinMode(LED_GR, OUTPUT);
    pinMode(LED_IR, OUTPUT);

    // Initialize motor driver pins
    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);

    // Initialize input pins with internal pull-up resistors
    pinMode(SW_DN, INPUT_PULLUP);
    pinMode(SW_UP, INPUT_PULLUP);
    pinMode(BTN_A, INPUT_PULLUP);
    pinMode(BTN_B, INPUT_PULLUP);
    pinMode(BTN_C, INPUT_PULLUP);

    // Set default direction and enable motor
    digitalWrite(DIR_PIN, HIGH);  // Set default direction
    digitalWrite(EN_PIN, LOW);    // Enable motor

    // Initialize serial communication
    Serial.begin(9600);           // Debug output
    tmc_serial.begin(115200);     // TMC2209 UART
    delay(500);

    // Initialize TMC2209 driver settings
    driver.begin();
    driver.toff(4);
    driver.blank_time(24);
    driver.rms_current(targetCurrent);
    driver.microsteps(microsteps[microstep_index]);
    driver.pwm_autoscale(true);

    // Quick rotate to check motor direction
    digitalWrite(DIR_PIN, HIGH);  // Set default direction
    for (int i = 0; i < 100; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(2000); // 2ms high
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(2000); // 2ms low
    }

    // Close serial and open 9600
    Serial.end();
    delay(100);
    Serial.begin(9600);
    delay(100);
}

// ---------------------------------------------------------
// Main loop function
// ---------------------------------------------------------
void loop() {
    // Read input states
    bool sw_dn = digitalRead(SW_DN) == LOW;
    bool sw_up = digitalRead(SW_UP) == LOW;
    bool btn_a = digitalRead(BTN_A) == LOW;
    bool btn_b = digitalRead(BTN_B) == LOW;
    bool btn_c = digitalRead(BTN_C) == LOW;

    // --- SW_DN = disable motor, UART, enable IR led, begin serial 9600 ---
    if (sw_dn && !prev_SW_DN) {
        digitalWrite(EN_PIN, HIGH); // Disable motor
        digitalWrite(LED_GR, LOW);  // Turn off green LED
        digitalWrite(LED_IR, HIGH); // Turn on IR LED
        Serial.begin(9600);
        delay(100);
    }

    // --- SW_UP = enable motor, disable IR led, begin serial 115200, start movement cycles---
    if (sw_up && !prev_SW_UP) {
        digitalWrite(EN_PIN, LOW);  // Enable motor
        digitalWrite(LED_GR, HIGH); // Turn on green LED
        digitalWrite(LED_IR, LOW);  // Turn off IR LED

        // Define motor speed
        float deg_per_step = 1.8f;
        float us_per_step = 1000000.0f / (speed * deg_per_step); // 1/(deg/s / deg/step) = s/deg [in us]
        int microstep = microsteps[microstep_index];
        float us_per_ustep = us_per_step / microstep; // us per microstep
        
        // DEBUG: Print info
        Serial.print(speed);
        Serial.print("deg/s // 1/");
        Serial.print(microstep);
        Serial.print(" // ");
        Serial.print((int)us_per_ustep);
        Serial.print("us // ");
        Serial.print(targetCurrent);
        Serial.println("mA");

        // enable motor, set current and microsteps
        driver.rms_current(targetCurrent);
        driver.microsteps(microstep);
        driver.pwm_autoscale(true);

        // Move for 2 seconds
        uint16_t start_time = millis();
        int step_count = 0;
        while (sw_up) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds((int)(us_per_ustep / 2));
            digitalWrite(STEP_PIN, LOW);
            delayMicroseconds((int)(us_per_ustep / 2));
            sw_up = digitalRead(SW_UP) == LOW;
            step_count++;
        }

        uint16_t elapsed_time = millis() - start_time;

        Serial.begin(9600);
        delay(100);

        // Print elapsed time and step count
        // Serial.print(elapsed_time);
        // Serial.print("ms // ");
        // Serial.print(step_count / microstep / deg_per_step);
        // Serial.print("deg // est. ");
        // Serial.print((float)step_count / (elapsed_time / 1000.0f / microstep / deg_per_step));
        // Serial.print("deg/s // @");
        // Serial.print(driver.rms_current());
        // Serial.println("mA");
        // delay(100);
    }
  
    // --- BTN_A: Cycle microstepping, apply on SW_UP ---
    if (btn_a && !prev_BTN_A) {
        handleButtonA();
    }

    // --- BTN_B: Start constant drive if SW_UP, else just set current ---
    if (btn_b && !prev_BTN_B) {
        handleButtonB();
    }

    // --- BTN_C: toggle UART/Serial Monitor ---
    if (btn_c && !prev_BTN_C) {
        handleButtonC();
    }

    // Update previous button states
    prev_SW_DN = sw_dn;
    prev_SW_UP = sw_up;
    prev_BTN_A = btn_a;
    prev_BTN_B = btn_b;
    prev_BTN_C = btn_c;
}

// cycle microsteps
void handleButtonA() {
    microstep_index = (microstep_index + 1) % 5; // cycle through microsteps
    Serial.print("Selected microstep: 1/");
    Serial.println(microsteps[microstep_index]);
}

// set speed from poti
void handleButtonB() {
  Serial.println("Setting speed...");
  int val = analogRead(POTI);
  speed = map(val, 0, 1023, 0, maxSpeed);
  Serial.print("Speed set to: ");
  Serial.print(speed);
  Serial.println("deg/s");
}

// set current from poti
void handleButtonC() {
  Serial.println("Setting current...");
  int val = analogRead(POTI);
  targetCurrent = map(val, 0, 1023, 0, maxCurrent);
  Serial.print("Current set to: ");
  Serial.print(targetCurrent);
  Serial.println("mA");
}