#pragma once
#include <Arduino.h>
#include "SpiBus.h"

class SpiDevice {
public:
    explicit SpiDevice(int csPin) : _cs(csPin) {}

    virtual void begin() {
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);
    }

protected:
    void select() {
        SpiBus::beginTransaction();
        digitalWrite(_cs, LOW);
    }

    void deselect() {
        digitalWrite(_cs, HIGH);
        SpiBus::endTransaction();
    }

    uint8_t transfer(uint8_t data) {
        return SpiBus::transfer(data);
    }

    uint8_t transfer16(uint16_t data) {
        return SpiBus::transfer16(data);
    }

private:
    int _cs;
};