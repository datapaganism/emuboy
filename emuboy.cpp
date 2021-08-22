#define _CRT_SECURE_NO_WARNINGS
#include "bus.hpp"
#include "renderer.hpp"

#define main SDL_main
#include "config.h"
#include <iostream>
#include <bitset>


int main(int argv, char** args)
{

    BUS bus("./roms/blargg/full.gb", "bios.bin");
    RENDERER renderer;


    while (renderer.isRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                renderer.isRunning = false;
                break;
            };
        }

        std::cout << bus.cpu.fetch_decode_execute() << "\n";
    }
   
    return 0;
}
