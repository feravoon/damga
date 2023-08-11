#include "GbPPU.h"
#include "stdio.h"

bool GbPPU::runFor(int cycles)
{
    bool hBlankStart = false;
    bool vBlankStart = false;
    bool oamStart = false;
    bool LyLycStart = false;
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
            hBlankStart = true;
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
                vBlankStart = true;
                //printf("VBLANK\n");
            }
            else
            {
                state = OAM_SEARCH;
                oamStart = true;
            }
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
                oamStart = true;
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
    
    uint8_t storedStat = memory->read(0xff41);
    uint8_t LYC = memory->read(0xff45);
    uint8_t LY = memory->read(0xff44);
    LyLycStart = (LY==LYC) && ((storedStat>>2)&0x01);
    uint8_t interruptSources = ((LyLycStart ? 1 : 0) << 6) | ((oamStart ? 1 : 0) << 5) | ((vBlankStart ? 1 : 0) << 4) | ((hBlankStart ? 1 : 0) << 3);
    setStatRegister(interruptSources);

    if(memory->read(0xff46) != 0x00)
    {
        dmaTransfer(memory->read(0xff46));
        memory->write(0xff46, 0x00);
    }
    return vBlankStart;
}

void GbPPU::setLY()
{
    memory->write(0xff44, lineNumber);
}

void GbPPU::setStatRegister(uint8_t interruptSources)
{
    uint8_t LYC = memory->read(0xff45);
    uint8_t LY = memory->read(0xff44);
    uint8_t storedStat = interruptSources | ((LYC==LY)<<2) | (uint8_t)state;
    memory->write(0xff41,storedStat);
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
    //printf("yapiyoruz bu sporu\n");
    uint16_t baseAddr16 = (baseAddr<<8);
    
    for(int i=0; i<160; i++)
        memory->write(i, memory->read(baseAddr16+i));
}

void GbPPU::drawLine()
{
    
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

    //uint8_t SCX = memory->read(0xff42);
    //uint8_t SCY = memory->read(0xff43);
    
    for(int i=0; i<160; i++)
    {
        uint8_t tileAddr = memory->read(BG_tilemap_addr + (lineNumber*160+i)/20 );
        uint8_t tileRow = memory->read(BG_Win_tiledata_addr + tileAddr + (lineNumber*160+i)%20  );
        fb.writePixelLoc(lineNumber,i,(tileRow>>(i%40)) & 0b00000011);
    }
}
