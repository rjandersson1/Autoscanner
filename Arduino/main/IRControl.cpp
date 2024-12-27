#include "IRControl.h"
#include <Arduino.h>

// Constructor def

IRControl::IRControl(int pin = 9, int frequency = 40000)
    : pin(pin),
    frequency(frequency)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    setupTimerAt40kHz();
}

void IRControl::on() {
    TCCR1A |= (1 << COM1A0); // Enable 40kHz SqWave on pin 9
}

void IRControl::off() {
    TCCR1A &= ~(1 << COM1A0); // Disable 40kHz SqWave on pin 9
}

void IRControl::sendHeader() {
    // Sony protocol header = 2400us on, 600us off
    on();
    delayMicroseconds(2400);
    off();
    delayMicroseconds(600);
}

void IRControl::sendBit(int bit) {
    if (bit == 1) {
        on();
        delayMicroseconds(1200);
        off();
        delayMicroseconds(600);
    }
    if (bit == 0) {
        on();
        delayMicroseconds(600);
        off();
        delayMicroseconds(600);
    }
}

void IRControl::send1() {
    on();
    delayMicroseconds(1200);
    off();
    delayMicroseconds(600);
}

void IRControl::send0() {
    on();
    delayMicroseconds(600);
    off();
    delayMicroseconds(600);
}

void IRControl::sendEnd() {
    off();
    delay(40); // 40ms delay
}

void IRControl::setupTimerAt40kHz() {
    Serial.println("Setting timer1");
    // Reset Timer1
    TCCR1A = 0; // reset output behaviour
    TCCR1B = 0; // reset timer mode & clock configuration
    TCNT1 = 0; // reset Timer1 counter

    // Set CompareA value for 40kHz (OCR1A = 49)
    OCR1A = calculateTimer1Frequency(40000);

    // Configure Timer1 for CTC mode (toggle pin state)
    // Clear Timer on Compare Match: timer resets to 0 when it matches value in OCR1A
    // WGM12 (waveform generation mode bit 12) enables CTC mode in TCCR1B
    TCCR1B |= (1 << WGM12); 

    // Enable toggle mode on OC1A (Pin 9)
    // COM1A0 (Compare Match Output Mode bit 0 for OC1A): toggles pin state each compare match
    TCCR1A |= (1 << COM1A0);

    // Start Timer1 with a prescaler of 8
    // Sets clock select bit 1 to prescaler 8
    TCCR1B |= (1 << CS11);
}

// Calculates OCR1A Compare value given a frequency. result must be an INT. Change prescaling precision if not
int IRControl::calculateTimer1Frequency(int frequency) {
    int clockSpeed = 16000000; // 16MHz (arduino uno)
    int prescaler = 8; // preset for 40kHz 
    int result = clockSpeed/(prescaler*frequency) - 1;
    return result;
}

void IRControl::sendCommand(uint32_t CMD) {
    Serial.println("Sending CMD");
    setupTimerAt40kHz();
    // Repeat command 3 times
    for (int j = 0; j < 3; j++) {
        sendHeader(); // Send header
        for (int i = 0; i < 20; i++) {
            sendBit(CMD & 1); // Send least significant bit
            CMD >>= 1; // shift to next bit
        }
        sendEnd(); // Send stop frame
    }
    Serial.println("Finished CMD");
}

uint32_t IRControl::readHexFromSerial() {
    Serial.println("Processing hex command...");
    // Read the input as a string
    String input = Serial.readStringUntil('\n'); // Read until a newline character

    // Check if the input starts with "0x" or "0X"
    if (input.startsWith("0x") || input.startsWith("0X")) {
        input = input.substring(2); // Remove the "0x" prefix
    }

    // Convert the remaining string to a uint32_t
    char inputBuffer[input.length() + 1];
    input.toCharArray(inputBuffer, input.length() + 1);
    return strtoul(inputBuffer, NULL, 16); // Convert string to uint32_t (base 16)
    Serial.println("Done!");
}