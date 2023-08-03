#include "GbCPU.h"
#include <stdio.h>

int main()
{
	GbCPU cpu = GbCPU(); // Define and initialize the CPU

	// Read GameBoy boot rom into memory
	cpu.memory.ReadFileIntoMemoryAt("Tetris.gb", 0x0000);

	cpu.printOutput = false; // for printing debug output to console
	cpu.PC = 0x100;

	int byteSize;
	for (int i = 0; i < 100; i++)
	{
		byteSize = cpu.disassemble_GbCPU_Op();
		printf("\n");
		cpu.PC += byteSize;
		//cpu.printOutput = true; // for printing debug output to console
		//cpu.processInstruction();
	}

	return 0;
}
