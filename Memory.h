#pragma once
#include <cstdint>
class Memory
{
public:
	uint8_t byteArray[65536];
	void write(uint16_t addr, uint8_t value);
	uint8_t read(uint16_t addr);
	uint8_t &operator[](uint16_t addr);
	Memory();
	void ReadFileIntoMemoryAt(const char* filename, uint32_t offset);
};

