#pragma once

#include <stdint.h>

#define CE      PB1
#define SS      PB2
#define MOSI    PB3
#define MISO    PB4
#define SCK     PB5

#define DUMMY_BYTE 0xFF

void Init_spi();

uint8_t Transfer_spi(uint8_t data);

void SendBuffer_spi(const uint8_t *data, uint16_t length);