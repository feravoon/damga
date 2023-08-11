#pragma once
#include "Memory.h"
#include "OAM.h"
#include "frameBuffer.h"

class GbPPU
{
private:
    int cyclesInState;  
    uint8_t lineNumber, horPixelNumber;
    void setLY();
    void setStatRegister(uint8_t interruptSources);
    void dmaTransfer(uint8_t baseAddr);
    OAM oam;
    void drawLine();
    bool lineDrawn;

public:
    enum PPUState{
        H_BLANK,
	    V_BLANK,
	    OAM_SEARCH,
	    PIXEL_TRANSFER
    };
    PPUState state;
    Memory* memory;
    frameBuffer fb;
    GbPPU(Memory* memory);
    bool runFor(int cycles);
};