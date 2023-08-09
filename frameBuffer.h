#pragma once
#include <cstdint>
class frameBuffer
{
public:
	uint8_t byteArray[160*144];
	void write(uint16_t addr, uint8_t value);
	uint8_t read(uint16_t addr);
    void writePixelLoc(uint8_t row, uint8_t column, uint8_t value);
    uint8_t readPixelLoc(uint8_t row, uint8_t column);
	void clear();
	uint8_t &operator[](uint16_t addr);
	frameBuffer();
};