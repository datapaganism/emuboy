#define _CRT_SECURE_NO_WARNINGS
#include "bus.hpp"
#include "renderer.hpp"

#define main SDL_main
#include "config.h"
#include <iostream>
#include <bitset>
#include <memory>



int main(int argv, char** args)
{

    //BUS bus("./roms/blargg/full.gb", "bios.bin");
    std::unique_ptr<BUS> bus;
    bus = std::make_unique<BUS>("./roms/TETRIS.gb", "bio 5s.bin");
    // BUS bus("./roms/TETRIS.gb", "bios.bin");
    RENDERER renderer;
    
    //bus->ppu.tile.bytes_per_tile.at(0) = 0x02;
    //bus->ppu.tile.bytes_per_tile.at(1) = 0xFF;
    //
    //bus->ppu.tile.bytes_per_tile.at(2) = 0x73;
    //bus->ppu.tile.bytes_per_tile.at(3) = 0x8C;
    //
    //bus->ppu.tile.bytes_per_tile.at(4) = 0x65;
    //bus->ppu.tile.bytes_per_tile.at(5) = 0x9A;

    //bus->ppu.tile.bytes_per_tile.at(6) = 0x48;
    //bus->ppu.tile.bytes_per_tile.at(7) = 0xB6;

    //bus->ppu.tile.bytes_per_tile.at(8) = 0x12;
    //bus->ppu.tile.bytes_per_tile.at(9) = 0xEC;

    //bus->ppu.tile.bytes_per_tile.at(10) = 0x26;
    //bus->ppu.tile.bytes_per_tile.at(11) = 0xD8;

    //bus->ppu.tile.bytes_per_tile.at(12) = 0xCE;
    //bus->ppu.tile.bytes_per_tile.at(13) = 0xB0;

    //bus->ppu.tile.bytes_per_tile.at(14) = 0x60;
    //bus->ppu.tile.bytes_per_tile.at(15) = 0x80;

    //for (int y = 0; y < 8; y++)
    //{
    //    for (int x = 0; x < 8; x++)
    //    {
    //        bus->ppu.tile.getPixelColour(x, y);
    //        std::cout << " ";
    //    }
    //    std::cout << "\n";
    //}


    bus->cpu.DEBUG_printCurrentState();
    
    
    unsigned int ticksNow = 0, ticksPrevious = 0;
    
    double tickDelta = 0;

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

        ticksNow = SDL_GetTicks();
        tickDelta = ticksNow - ticksPrevious;

#ifdef TURBO
        bus->emulate();
#else
        if (tickDelta > VSYNC)
        {
            std::cout << "fps: " << 1000 / tickDelta << std::endl;
            ticksPrevious = ticksNow;

            bus->emulate();
        }
#endif // TURBO

        


        //std::cout << bus->cpu.fetch_decode_execute() << "\n";
        /*bus->cpu.fetch_decode_execute();
        bus->cpu.do_interrupts();*/
    }
   
    return 0;
}
