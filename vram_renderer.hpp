#pragma once

#include "SDL.h"
#include "bus.hpp"

class VRAM_RENDERER
{
    public:
        VRAM_RENDERER();
        ~VRAM_RENDERER();
        bool isRunning = true;
        SDL_Renderer* renderer = nullptr;
        SDL_Window* window = nullptr;
        SDL_Texture* texture = nullptr;

        void render_vram_tiles(BUS *bus);

        std::array<FRAMEBUFFER_PIXEL, 8 * 16 * 8 * 24> generate_vram_framebuffer(BUS* bus);
    private:
        
};