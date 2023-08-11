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

void handleInterrupts()
{
	uint8_t statRegister = cpu.memory.read(0xff41);
	uint8_t IE = cpu.memory.read(0xffff);
	// sort according to priority
	if(cpu.IME){
		if(((statRegister>>6) & 0x01) & ((IE>>6) & 0x01)) // LY = LCY Int.
			cpu.generateInterrupt(1);

		if(((statRegister>>5) & 0x01) & ((IE>>5) & 0x01)) // OAM Int.
			cpu.generateInterrupt(1);

		if(((statRegister>>4) & 0x01) & ((IE>>4) & 0x01)) // V Blank Int.
		{
			cpu.generateInterrupt(1);
			cpu.generateInterrupt(0);
		}

		if(((statRegister>>3) & 0x01) & ((IE>>3) & 0x01)) // H Blank Int.
			cpu.generateInterrupt(1);
	}
}

void emulateGb()
{
	// Read GameBoy boot rom into memory
	cpu.memory.ReadFileIntoMemoryAt("Tetris.gb", 0x0000);
	//cpu.memory.ReadFileIntoMemoryAt("DMG_ROM.bin", 0x0000);

	cpu.printOutput = false; // for printing debug output to console
	cpu.PC = 0x0100;

	auto now = high_resolution_clock::now();
	auto nextPaint = now;
	int cycles = 0;
	int cyclesToRun = 0;
	int viter = 0;
	//int cycleToRun = 0;
	while(true)
	{
		cycles = 0;
		while ((cycles < 34917) & (!vblank.load())){
			if(false)
			{
				cpu.disassemble_GbCPU_Op();
				cpu.printOutput = true;
			}
			else
				cpu.printOutput = false;
				
			cyclesToRun = cpu.processInstruction();
			vblank.store(ppu.runFor(cyclesToRun));
			handleInterrupts();
			cycles += cyclesToRun;
			viter++;
			//printf("%d\n",viter);
			if(vblank)
				break;
		}
		now = high_resolution_clock::now();
		if ((now>=nextPaint) & vblank.load())
		{
			paintNow.store(true);
			nextPaint = now + nanoseconds((int)(1000000000.0f/60.0f));
		}
	}
}

int main()
{
	Renderer renderer = Renderer(3.0f);
	bool close = false;
	std::thread thr(emulateGb);
	SDL_Event event;
	uint16_t memStart = 0x8000;
	while(!close)
	{
		SDL_PollEvent(&event);
		close = event.type == SDL_QUIT;
		if(paintNow.load())
		{
			//std::copy(std::begin(cpu.memory.byteArray) + 0x8000, std::begin(cpu.memory.byteArray) + 0x8000 + 160*144, std::begin(ppu.fb.byteArray));
			
			uint16_t tileInd, tileByteInd, fpX, fpY, fbAddr;
			uint8_t tileByte0,tileByte1,pixelValue,lsbBit,msbBit;
			for(int a=0;a<256*16;a+=2)
			{
				//cpu.memory.write(a+memStart,0b11111111);
				//cpu.memory.write(a+memStart+1,0b11111111);
				tileInd = a/16;
				tileByteInd = a%16;
				tileByte0 = cpu.memory.read(a+memStart);
				tileByte1 = cpu.memory.read(a+memStart+1);
				fpX = 8*(tileInd%16);
				fpY = 8*(tileInd/16) + tileByteInd/2;

				for(int i=0; i<8; i++)
				{
					fbAddr = 160*fpY + fpX + i;
					lsbBit = ((tileByte0>>(7-i))&0x01);
					msbBit = ((tileByte1>>(7-i))&0x01);
					pixelValue = (msbBit<<1) | lsbBit;
					ppu.fb.write(fbAddr, 85*(3-pixelValue));
				}

			}
			renderer.render(ppu.fb);
			paintNow.store(false);
			vblank.store(false);
			//printf("%d\n",rand());
		}
	}
	return 0;
}
