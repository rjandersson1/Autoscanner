// Kick-and-hold solenoid test sketch (revised)
// - Potentiometer sets either HOLD duty, KICK time, or ON time depending on mode
// - Button (D3, INPUT_PULLUP):
//     Short press: start a timed firing sequence (kick -> hold for t_on seconds)
//     Long press (>0.6 s): cycle parameter mode:
//         SET_HOLD_DUTY -> SET_KICK_MS -> SET_ON_S -> SET_HOLD_DUTY ...
// - During firing:
//     - After kick_ms, PWM goes to hold_duty
//     - Sequence ends automatically after t_on seconds
//     - If button is pressed during firing: PWM immediately goes to 0 (safety latch)
//       and stays 0 until the original timer ends (no re-trigger on release)
//
// Notes:
// - Uses only space indentation (no tabs).
// - Serial prints: mode, kick_ms, hold_duty, t_on_ms, firing, safety_latched.

#include <Arduino.h>

#define PIN_POTI A0
#define PIN_PWM  6
#define PIN_BTN  3

enum ParamMode : uint8_t {
    SET_HOLD_DUTY = 0,
    SET_KICK_MS   = 1,
    SET_ON_S      = 2
};

static ParamMode mode = SET_HOLD_DUTY;

// Defaults
static uint16_t kick_ms = 120;         // 0..2000 ms
static uint16_t t_on_ms = 1500;        // 0..5000 ms (total on-window)
static uint8_t hold_duty = 80;         // 0..255
static const uint8_t kick_duty = 255;

// Button / debounce
static bool last_btn_level = true;     // pullup: true=HIGH=not pressed
static uint32_t t_last_change = 0;
static uint32_t t_btn_down = 0;

// Firing state
static bool firing = false;
static bool safety_latched = false;
static uint32_t t_fire_start = 0;

// Serial rate limiting
static uint32_t t_last_print = 0;

static uint16_t readKickMsFromPot() {
    int raw = analogRead(PIN_POTI); // 0..1023
    return (uint16_t)map(raw, 0, 1023, 0, 2000);
}

static uint16_t readOnMsFromPot() {
    int raw = analogRead(PIN_POTI); // 0..1023
    return (uint16_t)map(raw, 0, 1023, 0, 5000);
}

static uint8_t readHoldDutyFromPot() {
    int raw = analogRead(PIN_POTI); // 0..1023
    return (uint8_t)(raw / 4);      // 0..255
}

static const char* modeName(ParamMode m) {
    switch (m) {
        case SET_HOLD_DUTY: return "SET_HOLD_DUTY";
        case SET_KICK_MS:   return "SET_KICK_MS";
        case SET_ON_S:      return "SET_ON_S";
        default:            return "UNKNOWN";
    }
}

static void printStatus(const char* tag) {
    Serial.print(tag);
    Serial.print(" | mode=");
    Serial.print(modeName(mode));
    Serial.print(" kick_ms=");
    Serial.print(kick_ms);
    Serial.print(" hold_duty=");
    Serial.print(hold_duty);
    Serial.print(" t_on_ms=");
    Serial.print(t_on_ms);
    Serial.print(" firing=");
    Serial.print(firing ? "1" : "0");
    Serial.print(" safety_latched=");
    Serial.println(safety_latched ? "1" : "0");
}

static void applyPwm(uint32_t dt_ms) {
    if (!firing) {
        analogWrite(PIN_PWM, 0);
        return;
    }

    if (safety_latched) {
        analogWrite(PIN_PWM, 0);
        return;
    }

    if (dt_ms < kick_ms) {
        analogWrite(PIN_PWM, kick_duty);
    } else {
        analogWrite(PIN_PWM, hold_duty);
    }
}

void setup() {
    Serial.begin(9600);

    pinMode(PIN_POTI, INPUT);
    pinMode(PIN_PWM, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);

    analogWrite(PIN_PWM, 0);

    Serial.println("Boot");
    printStatus("INIT");
}

void loop() {
    uint32_t now = millis();

    // Update whichever parameter is currently selected (always, even during firing)
    if (mode == SET_KICK_MS) {
        kick_ms = readKickMsFromPot();
    } else if (mode == SET_ON_S) {
        t_on_ms = readOnMsFromPot();
    } else {
        hold_duty = readHoldDutyFromPot();
    }

    // If kick_ms > t_on_ms, clamp kick_ms so kick window doesn't exceed total window
    if (kick_ms > t_on_ms) {
        kick_ms = t_on_ms;
    }

    // Debounced button edge detection
    bool btn_level = (digitalRead(PIN_BTN) == HIGH); // true=not pressed
    if (btn_level != last_btn_level) {
        if ((now - t_last_change) > 80) { // debounce
            t_last_change = now;
            last_btn_level = btn_level;

            if (!btn_level) {
                // Pressed
                t_btn_down = now;

                // If already firing, latch safety immediately on press
                if (firing && !safety_latched) {
                    safety_latched = true;
                    analogWrite(PIN_PWM, 0);
                    Serial.println("SAFETY_LATCH | PWM=0 until timer ends");
                }
            } else {
                // Released
                uint32_t press_ms = now - t_btn_down;

                if (press_ms >= 600) {
                    // Long press: cycle parameter mode
                    if (mode == SET_HOLD_DUTY) {
                        mode = SET_KICK_MS;
                    } else if (mode == SET_KICK_MS) {
                        mode = SET_ON_S;
                    } else {
                        mode = SET_HOLD_DUTY;
                    }
                    printStatus("MODE_CYCLE");
                } else {
                    // Short press: start firing only if currently idle (not firing)
                    if (!firing) {
                        firing = true;
                        safety_latched = false;
                        t_fire_start = now;

                        Serial.print("FIRE_START | kick_ms=");
                        Serial.print(kick_ms);
                        Serial.print(" hold_duty=");
                        Serial.print(hold_duty);
                        Serial.print(" t_on_ms=");
                        Serial.println(t_on_ms);

                        // Apply initial kick immediately
                        analogWrite(PIN_PWM, kick_duty);
                    }
                }
            }
        }
    }

    // Firing timing logic
    if (firing) {
        uint32_t dt = now - t_fire_start;

        // End automatically after total on window
        if (dt >= t_on_ms) {
            firing = false;
            safety_latched = false;
            analogWrite(PIN_PWM, 0);
            Serial.println("FIRE_END | PWM=0");
        } else {
            // Continue applying PWM (unless safety latched)
            applyPwm(dt);
        }
    } else {
        analogWrite(PIN_PWM, 0);
    }

    // Periodic status print (2 Hz)
    if ((now - t_last_print) >= 500) {
        t_last_print = now;
        printStatus("STAT");
    }
}