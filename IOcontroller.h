#pragma once
#include <cstdint>
class IOcontroller
{
public:
	uint8_t I[256];
	uint8_t O[256];
	uint8_t newOutput[256];
	IOcontroller();
	~IOcontroller();
};

