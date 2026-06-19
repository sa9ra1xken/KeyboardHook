#pragma once
#include <Arduino.h>
#include <SPI.h>


class Listner {

public:
    Listner(int readPin, int _capsPin, int _codePin);
    void begin();
    void onScan();
    int getScanCode(); 
    int getAsciiCode();
    bool isAllOff(); 

private:

    int _read_pin;
    int _caps_pin;
    int _code_pin;

    uint8_t pressed[8];
    uint8_t sense_d0[8];
    uint8_t sense_d1[8];
    uint8_t sense_d2[8];

    static const char scan2ascii[] PROGMEM;
    bool isUpperCase();
};

int encode8to3(uint8_t value);
