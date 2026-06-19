#include "SpiBus.h"

SPISettings SpiBus::_settings(1000000, LSBFIRST, SPI_MODE0);

void SpiBus::begin() {
    SPI.begin();
}

void SpiBus::beginTransaction() {
    SPI.beginTransaction(_settings);
}

void SpiBus::endTransaction() {
    SPI.endTransaction();
}

uint8_t SpiBus::transfer(uint8_t data) {
    return SPI.transfer(data);
}

uint16_t SpiBus::transfer16(uint16_t data) {
    return SPI.transfer16(data);
}
