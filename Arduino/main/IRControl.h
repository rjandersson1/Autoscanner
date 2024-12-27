#ifndef IRCONTROL_H
#define IRCONTROL_H
#include <Arduino.h>

class IRControl {
public:
    // Constructor
    IRControl(int pin, int frequency);
    

    // Properties
    int pin;
    int frequency;

    // Sony A7RII Commands (20-bit IR codes)
    uint32_t CMD_takePhoto = 0xB4B8F; // Shutter | take photo
    uint32_t CMD_VID = 0x12B8F;      // Start | stop video recording
    uint32_t CMD_DISP = 0x28B8F;     // Display change cycles round
    uint32_t CMD_Menu = 0x1CB8F;     // Enter menu | leave menu
    uint32_t CMD_MenuU = 0x5CB8F;    // Menu up
    uint32_t CMD_MenuD = 0xDCB8F;    // Menu down
    uint32_t CMD_MenuR = 0xFCB8F;    // Menu right
    uint32_t CMD_MenuL = 0x7CB8F;    // Menu left
    uint32_t CMD_OK = 0x9CB91;       // Menu OK
    uint32_t CMD_ZoomIn = 0x52B8F;   // Zoom in
    uint32_t CMD_ZoomOut = 0xD2B8F;  // Zoom out

    // Methods
    void on();
    void off();

    // Sender commands
    void sendHeader();
    void send1();
    void send0();
    void sendBit(int bit);
    void sendEnd();
    void sendCommand(uint32_t CMD);

    void setupTimerAt40kHz();
    int calculateTimer1Frequency(int frequency);

    uint32_t readHexFromSerial();

private:

};



#endif // IRCONTROL_H