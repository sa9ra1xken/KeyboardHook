#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <avr/pgmspace.h>

class Typer
{
public:

    explicit Typer(
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
    void Print(uint16_t data);
    void PrintAccent(uint16_t data);
    void LoadSwitch(uint16_t data);
  
private:

    bool was_CR;

    int load_pin;
    int _printing_pin;
    //int printing_pin;
    int feeding_pin;

    int waiting_EOA_indicator;
    int waiting_SOP_indicator;
    int waiting_SOF_indicator; 

    static const uint16_t key_map_carriage[] PROGMEM;
    static const uint8_t key_map_carriage_size;
    static const uint8_t key_map_carriage_top;

    static const uint16_t key_map_numeric[] PROGMEM;
    static const uint8_t key_map_numeric_size;
    static const uint8_t key_map_numeric_top;
  
    static const uint16_t key_map_alpha[] PROGMEM;
    static const uint8_t key_map_alpha_size;
    static const uint8_t key_map_alpha_top;
  
    static const uint16_t key_map_ALPHA[] PROGMEM;
    static const uint8_t key_map_ALPHA_size;
    static const uint8_t key_map_ALPHA_top;
  
    
    void LineFeed();
    
    void Wait4EOP();
    void Wait4EOF();

    uint8_t decode3to8(int value);
};