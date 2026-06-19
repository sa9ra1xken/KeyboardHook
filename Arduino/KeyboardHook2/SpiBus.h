#pragma once
#include <Arduino.h>
#include <SPI.h>

class SpiBus {
public:
    static void begin();

    static void beginTransaction();
    static void endTransaction();

    static uint8_t transfer(uint8_t data);
    static uint16_t transfer16(uint16_t data);

private:
    static SPISettings _settings;
};
