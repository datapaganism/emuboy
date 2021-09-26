#pragma once

#include "SDL.h"
#include "bus.hpp"

class RENDERER
{
    public:
        RENDERER();
        ~RENDERER();
        bool isRunning = true;
        SDL_Renderer* renderer = nullptr;
        SDL_Window* window = nullptr;

        void render_frame(BUS *bus);
    private:
        
};