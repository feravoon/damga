#include "GbCPU.h"
#include <stdio.h>

int main()
{
	GbCPU cpu = GbCPU(); // Define and initialize the CPU

	// Read GameBoy boot rom into memory
	cpu.memory.ReadFileIntoMemoryAt("DMG_ROM.bin", 0x0000);

	cpu.printOutput = false; // for printing debug output to console
	cpu.PC = 0x00;

	int byteSize;
	for (int i = 0; i < 100; i++)
	{
		byteSize = cpu.disassemble_GbCPU_Op();
		cpu.printOutput = true; // for printing debug output to console
		cpu.processInstruction();
	}

	return 0;
}
