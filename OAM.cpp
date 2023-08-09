#include "OAM.h"

void OAM::write(uint16_t addr, uint8_t value)
{
	byteArray[addr] = value;
}

uint8_t OAM::read(uint16_t addr)
{
	return byteArray[addr];
}


uint8_t &OAM::operator [] (uint16_t addr)
{
	return byteArray[addr];
}

OAM::OAM() : byteArray()
{
}