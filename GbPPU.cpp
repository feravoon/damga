#include "GbPPU.h"

bool GbPPU::runFor(int cycles)
{
    bool paintNow = false;
    cyclesInState += cycles;
    switch (state)
    {
    case OAM_SEARCH:
        if(cyclesInState>20)
        {
            state = PIXEL_TRANSFER;
            cyclesInState %= 20;
        }
        break;
    case PIXEL_TRANSFER:
        if(!lineDrawn)
        {
            // draw the scanline
            if(lineNumber==0)
                fb.clear();
            drawLine();
            lineDrawn = true;
        }
        if(cyclesInState>43)
        {
            state = H_BLANK;
            cyclesInState %= 43;
        }
        break;
    case H_BLANK:
        lineDrawn = false;
        if(cyclesInState>51)
        {
            if(lineNumber==143)
            {
                state = V_BLANK;
                paintNow = true;
            }
            else
                state = OAM_SEARCH;

            cyclesInState %= 51;
            lineNumber++;
            lineDrawn = false;
        }
        break;
    case V_BLANK:
        if(cyclesInState>114)
        {
            if(lineNumber==153)
            {
                state = OAM_SEARCH;
                lineNumber = 0;
            }
            else
            {
                lineNumber++;
                lineDrawn = false;
            }
            cyclesInState %= 114;
        }
        break;
    
    default:
        break;
    }
    
    setLY();
    setStatRegister();

    if(memory->read(0xff46) != 0x00)
    {
        dmaTransfer(memory->read(0xff46));
        memory->write(0xff46, 0x00);
    }
    return paintNow;
}

void GbPPU::setLY()
{
    memory->write(0xff44, lineNumber);
}

void GbPPU::setStatRegister()
{
    uint8_t storedStat = memory->read(0xff41);
    uint8_t LYC = memory->read(0xff45);
    uint8_t LY = memory->read(0xff44);
    storedStat = (storedStat & 0xf0) & (((LYC==LY)<<2) | (uint8_t)state);
    memory->write(0xff45,storedStat);
}

GbPPU::GbPPU(Memory* memory)
{
    cyclesInState = 0;
    lineNumber = 0;
    horPixelNumber = 0;
    state = OAM_SEARCH;
    lineDrawn = false;
    this->memory = memory;
    oam = OAM();
    fb = frameBuffer();
}

void GbPPU::dmaTransfer(uint8_t baseAddr)
{
    uint16_t baseAddr16 = (baseAddr<<8);
    
    for(int i=0; i<160; i++)
        oam.write(i, memory->read(baseAddr16+i));
}

void GbPPU::drawLine()
{
    /*
    uint8_t LCDC = memory->read(0xff40); // LCD Control Register
    uint16_t BG_tilemap_addr, BG_Win_tiledata_addr;
    switch (LCDC>>3 & 0x01)
    {
    case 0: BG_tilemap_addr = 0x9800; break;
    case 1: BG_tilemap_addr = 0x9C00; break;
    }

    switch (LCDC>>4 & 0x01)
    {
    case 0: BG_Win_tiledata_addr = 0x8800; break;
    case 1: BG_Win_tiledata_addr = 0x8000; break;
    }

    uint8_t SCX = memory->read(0xff42);
    uint8_t SCY = memory->read(0xff43);
    */
    for(int i=0; i<160; i++)
    {
        //uint8_t tileRow = memory->read(BG_tilemap_addr + lineNumber*80 + i)
        fb.writePixelLoc(lineNumber,i,i);
    }

}
