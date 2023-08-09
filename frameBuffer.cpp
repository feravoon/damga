#include "frameBuffer.h"

void frameBuffer::write(uint16_t addr, uint8_t value)
{
	byteArray[addr] = value;
}

uint8_t frameBuffer::read(uint16_t addr)
{
	return byteArray[addr];
}

void frameBuffer::writePixelLoc(uint8_t row, uint8_t column, uint8_t value)
{
    uint16_t addr = row*160 + column;
	byteArray[addr] = value;
}

uint8_t frameBuffer::readPixelLoc(uint8_t row, uint8_t column)
{
    uint16_t addr = row*160 + column;
	return byteArray[addr];
}

void frameBuffer::clear()
{
	for(int i=0; i<160*144; i++)
		byteArray[i] = 0;
}

uint8_t &frameBuffer::operator [] (uint16_t addr)
{
	return byteArray[addr];
}

frameBuffer::frameBuffer() : byteArray()
{
}