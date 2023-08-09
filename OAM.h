#pragma once
#include <cstdint>
class OAM
{
public:
	uint8_t byteArray[160];
	void write(uint16_t addr, uint8_t value);
	uint8_t read(uint16_t addr);
	uint8_t &operator[](uint16_t addr);
	OAM();
};