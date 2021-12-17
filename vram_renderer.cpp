#include "vram_renderer.hpp"
#include "config.h"


VRAM_RENDERER::VRAM_RENDERER()
{
    SDL_Init(SDL_INIT_VIDEO);
    this->window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        8 * 15 * RES_SCALING,
        8 * 26 * RES_SCALING, SDL_WINDOW_SHOWN);

    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_TEXTUREACCESS_TARGET);
    SDL_SetRenderDrawColor(this->renderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);
    
    this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 8 * 15, 8 * 26);
    
    SDL_RenderClear(this->renderer);
    SDL_RenderPresent(this->renderer);
}

VRAM_RENDERER::~VRAM_RENDERER()
{
    SDL_DestroyTexture(this->texture);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void VRAM_RENDERER::render_vram_tiles(BUS *bus)
{
    SDL_RenderClear(this->renderer);


    //convert tile data into argb8888 pixels and then feed it into this function
    SDL_UpdateTexture(this->texture, NULL, bus->ppu.framebuffer.get(), 8 * 15 * sizeof(FRAMEBUFFER_PIXEL));
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);        

    SDL_RenderPresent(this->renderer);
}


