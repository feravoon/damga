#include "Memory.h"
#include <cstdio>

void Memory::write(uint16_t addr, uint8_t value)
{
	byteArray[addr] = value;
}

uint8_t Memory::read(uint16_t addr)
{
	return byteArray[addr];
}


uint8_t &Memory::operator [] (uint16_t addr)
{
	return byteArray[addr];
}

Memory::Memory() : byteArray()
{
}

void Memory::ReadFileIntoMemoryAt(const char* filename, uint32_t offset)
{
	FILE* f = fopen(filename, "rb");
	if (f == NULL)
	{
		printf("error: Couldn't open %s\n", filename);
		return;
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	uint8_t* buffer = &byteArray[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
}