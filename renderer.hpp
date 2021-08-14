#pragma once

#include "SDL.h"

class RENDERER
{
    public:
        RENDERER();
        ~RENDERER();
        bool isRunning = true;

    private:
        SDL_Renderer *renderer = nullptr;
        SDL_Window *window = nullptr;
};