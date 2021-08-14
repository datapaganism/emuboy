#include "renderer.hpp"
#include "config.h"


RENDERER::RENDERER()
{
    SDL_Init(SDL_INIT_VIDEO);
    this->window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        XRES * RES_SCALING,
        YRES * RES_SCALING, SDL_WINDOW_SHOWN);

    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_TEXTUREACCESS_TARGET);
    SDL_SetRenderDrawColor(this->renderer,0,0,0,0);
    SDL_RenderClear(this->renderer);
    SDL_RenderPresent(this->renderer);
}

RENDERER::~RENDERER()
{
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}