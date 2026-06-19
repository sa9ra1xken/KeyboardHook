#pragma once
#include <Arduino.h>
#include <SPI.h>

class Printer;

class Listner {

public:
    Listner(Printer* prt, int readPin, int _capsPin, int _codePin);
    void begin();
    void onScan();

    struct ScanCode {
        uint8_t KeyCode;
        bool isCodeMode;
    };

    ScanCode getScanCode(); 

    int getAsciiCode();
    bool isAllOff(); 
    void dumpSwitch();

private:

    Printer* printer;
    int _read_pin;
    int _caps_pin;
    int _code_pin;

    uint8_t sense_d0[8];
    uint8_t sense_d1[8];
    uint8_t sense_d2[8];
    uint8_t pressed[8];
    bool isCodeMode;

    uint8_t code_d0;
    uint8_t code_d1;
    uint8_t code_d2;

    static const char scan2ascii_lower[] PROGMEM;
    static const char scan2ascii_upper[] PROGMEM;
    
    bool isUpperCase();

};

int encode8to3(uint8_t value);
