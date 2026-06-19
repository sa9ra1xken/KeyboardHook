#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <avr/pgmspace.h>

class Printer
{
public:

    explicit Printer(
        uint8_t scanInterruptNum,    
        int _codePin,
        int loadPin, 
        int _pringingPin,
        int feedingPin, 
        int waitingSOP, 
        int waitingSOF, 
        int waitingEOA
    );
 
    void begin();
    void type(uint8_t index);
    void PushAndRelease(uint16_t data);
    void Wait4SOP();
    void Wait4SOF();
    void Print(uint8_t data);
    void PrintAccent(uint8_t data);
    void ResetCodeMode();
    void LoadSwitch(uint16_t data);
  
private:

    uint8_t scan_intr_num;
    bool was_CR;

    int load_pin;
    int _printing_pin;
    //int printing_pin;
    int feeding_pin;
    int _code_pin;

    int waiting_EOA_indicator;
    int waiting_SOP_indicator;
    int waiting_SOF_indicator; 

    static const uint8_t ascii_to_scancode_map[] PROGMEM;
  
    void PrintCRLF();
    void Wait4EOP();
    void Wait4EOF();
    uint8_t decode3to8(int value);
    uint16_t decode(uint8_t value);
};