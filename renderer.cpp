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
    SDL_SetRenderDrawColor(this->renderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);
    
    this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, XRES, YRES);
    
    SDL_RenderClear(this->renderer);
    SDL_RenderPresent(this->renderer);
}

RENDERER::~RENDERER()
{
    SDL_DestroyTexture(this->texture);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void RENDERER::render_frame(BUS *bus)
{
    SDL_RenderClear(this->renderer);

    if (bus->ppu.lcd_enabled())
    {
        /*for (int y = 0; y < YRES; y++)
        {
            for (int x = 0; x < XRES; x++)
            {
                    auto pixel = bus->ppu.framebuffer[x + (XRES * y)];
                    SDL_SetRenderDrawColor(this->renderer, pixel.red, pixel.green, pixel.blue, pixel.alpha);
                    SDL_Rect r;
                    r.x = x * RES_SCALING;
                    r.y = y * RES_SCALING;
                    r.w = RES_SCALING;
                    r.h = RES_SCALING;
                    SDL_RenderFillRect(this->renderer, &r);
            }
        }*/

        void* mPixels;
        int mPitch;

        mPixels = bus->ppu.framebuffer.get();

        //SDL_LockTexture(this->texture, NULL, (void**)&mPixels, &mPitch);

       
        SDL_UpdateTexture(this->texture, NULL, bus->ppu.framebuffer.get(), XRES * sizeof(FRAMEBUFFER_PIXEL));
        SDL_UnlockTexture(this->texture);
        SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);        
    }

    SDL_RenderPresent(this->renderer);
}
