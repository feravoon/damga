#include "Renderer.h"
#include <iostream>
#include <SDL2/SDL_ttf.h>

uint8_t bitReverseTable[] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

uint8_t bitReverse(uint8_t toReverse)
{
	return bitReverseTable[toReverse];
}

Renderer::Renderer(float scale)
{
	this->scale = scale;
	
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }

    // creates a window
    this->win = SDL_CreateWindow("DaMGa", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*this->scale, 144*this->scale, SDL_WINDOW_ALLOW_HIGHDPI);
 
    // triggers the program that controls your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
 
    // creates a renderer to render our images
    this->rend = SDL_CreateRenderer(win, -1, render_flags);

    SDL_ShowCursor(SDL_DISABLE);
}

void pixel_and(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	Uint32 * const target_pixel = (Uint32 *) ((Uint8 *)surface->pixels + y*surface->pitch + x*surface->format->BytesPerPixel);
	*target_pixel &= pixel;	
}

void black_to_transparent(SDL_Surface *surface, int x, int y)
{
	Uint32 * const target_pixel = (Uint32 *) ((Uint8 *)surface->pixels + y*surface->pitch + x*surface->format->BytesPerPixel);
	if (*target_pixel == 0xff000000)
		*target_pixel = 0x00000000;	
}

void Renderer::render(frameBuffer fb)
{
	// Copy the image from emulated memory
    std::copy(std::begin(fb.byteArray), std::begin(fb.byteArray) + 160 * 144, std::begin(this->imByteArray));

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(this->imByteArray,160,144,8,160/8,SDL_PIXELFORMAT_INDEX1MSB);
	
	// Convert surface pixel format to RGBA32
	//surface = SDL_ConvertSurfaceFormat(surface,SDL_PIXELFORMAT_RGBA32,0);
	
    this->tex = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    // set the background color
    SDL_SetRenderDrawColor(rend,0,0,0,SDL_ALPHA_OPAQUE);

    // clears the screen
    SDL_RenderClear(rend);

    // Game frame texture coordinates
    dest.w = 160*2*this->scale;
    dest.h = 144*2*this->scale;
    dest.x = 0*this->scale;
    dest.y = 0*this->scale;
    
	SDL_RenderCopy(rend,this->tex,NULL,&dest);

    SDL_DestroyTexture(this->tex);
    SDL_RenderPresent(rend);
}