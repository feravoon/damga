#include "GbCPU.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_timer.h"

class Renderer
{
    public:
        SDL_Window* win;
        SDL_Renderer* rend;
        uint8_t imByteArray[224 * 256 / 8];
        SDL_Texture* tex;
        SDL_Texture* texBG;
        SDL_Rect dest;
        float scale;
        Renderer(float scale);
        void render(Memory cpuMem);
};