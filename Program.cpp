#include "GbCPU.h"
#include "GbPPU.h"
#include "Renderer.h"
#include <stdio.h>
#include <thread>
using namespace std::chrono;

std::atomic<bool> paintNow{false};
GbCPU cpu = GbCPU(); 
GbPPU ppu = GbPPU(&(cpu.memory));
Renderer renderer = Renderer(3.0f);

void emulateGb()
{
	
	// Read GameBoy boot rom into memory
	cpu.memory.ReadFileIntoMemoryAt("Tetris.gb", 0x0000);
	cpu.memory.ReadFileIntoMemoryAt("DMG_ROM.bin", 0x0000);

	cpu.printOutput = false; // for printing debug output to console
	cpu.PC = 0x0000;

	auto now = high_resolution_clock::now();
	auto nextInterrupt = now;
	int cycles = 0;
	int cycleToRun = 0;
	while(true)
	{
		cycles = 0;
		while (2*34917 > cycles){
			cycles += cpu.processInstruction();
		}
		now = high_resolution_clock::now();
		if (now>=nextInterrupt)
		{
			paintNow = true;
			nextInterrupt = now + nanoseconds((int)(1000000000.0f/60.0f));
		}
	}
}

int main()
{
	
	bool close = false;
	std::thread thr(emulateGb);
	Memory m = Memory();
	while(!close)
	{
		if(paintNow)
		{
			paintNow = false;
			renderer.render(m);
			//printf("%d\n",rand());
		}
	}

	return 0;
}
