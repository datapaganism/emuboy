#pragma once

#include "SDL.h"
#include "bus.hpp"

class BG_MAP_RENDERER
{
public:
    BG_MAP_RENDERER();
    ~BG_MAP_RENDERER();
    bool isRunning = true;
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    SDL_Texture* texture = nullptr;

    void render_vram_tiles(BUS* bus);

    void generate_bg_framebuffer(BUS* bus);

    std::unique_ptr<FRAMEBUFFER_PIXEL[]> framebuffer = std::make_unique<FRAMEBUFFER_PIXEL[]>(8 * 32 * 8 * 32);
private:

};