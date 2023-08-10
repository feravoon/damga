#include "GbCPU.h"
#include "GbPPU.h"
#include "Renderer.h"
#include <stdio.h>
#include <thread>
#include <cmath>
using namespace std::chrono;

std::atomic<bool> paintNow{false};
std::atomic<bool> vblank{false};
GbCPU cpu = GbCPU(); 
GbPPU ppu = GbPPU(&(cpu.memory));


void emulateGb()
{
	
	// Read GameBoy boot rom into memory
	cpu.memory.ReadFileIntoMemoryAt("Tetris.gb", 0x0000);
	cpu.memory.ReadFileIntoMemoryAt("DMG_ROM.bin", 0x0000);

	//cpu.printOutput = true; // for printing debug output to console
	cpu.PC = 0x0000;

	auto now = high_resolution_clock::now();
	auto nextInterrupt = now;
	int cycles = 0;
	int cyclesToRun = 0;
	bool vblank = false;
	//int cycleToRun = 0;
	while(true)
	{
		cycles = 0;
		while ((34917 > cycles) & !vblank){
			//cpu.disassemble_GbCPU_Op();
			cyclesToRun = cpu.processInstruction();
			vblank = ppu.runFor(cyclesToRun);
			cycles += cyclesToRun;
			if(vblank)
				break;
		}
		now = high_resolution_clock::now();
		if ((now>=nextInterrupt) & vblank)
		{
			paintNow = true;
			
			nextInterrupt = now + nanoseconds((int)(1000000000.0f/60.0f));
		}
	}
}

int main()
{
	Renderer renderer = Renderer(3.0f);
	bool close = false;
	std::thread thr(emulateGb);
	SDL_Event event;
	while(!close)
	{
		SDL_PollEvent(&event);
		close = event.type == SDL_QUIT;
		if(paintNow)
		{
			paintNow = false;
			vblank = false;
			renderer.render(ppu.fb);
			//printf("%d\n",3);
		}
	}

	return 0;
}
