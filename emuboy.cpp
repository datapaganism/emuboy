#define _CRT_SECURE_NO_WARNINGS
#include "bus.hpp"
#include "renderer.hpp"

#define main SDL_main
#include "config.h"
#include <iostream>
#include <bitset>
#include <memory>
#include "joypad_mapping.h"



int main(int argv, char** args)
{

    

    //BUS bus("./roms/blargg/full.gb", "bios.bin");
    std::unique_ptr<BUS> bus;
    bus = std::make_unique<BUS>("./roms/TETRIS.gb", "bios.bin");
    // BUS bus("./roms/TETRIS.gb", "bios.bin");
    RENDERER renderer;

    bus.get()->ppu.fifo_bg.push(FIFO_pixel(01,01,1,1));

    bus.get()->ppu.fifo_bg.pop();
    bus.get()->ppu.fifo_bg.pop();

    bus.get()->ppu.fifo_bg.push(FIFO_pixel(01, 01, 1, 1));

    int i = 1;
    TILE tile0(bus.get(), 0x8000+(16*i));
    tile0.consolePrint();
    
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

    //mapping from sdl physical keys to virtual buttons for gameboy
   
    // the steps of emulation during the while loop
    // 1. modify input registers as SDL polls for an event
    // 2. run cpu, update timers, run graphics and sound chips, and do interrupts
    // 3. renderer displays graphics from emulated state
    // rinse and repeat
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

            case SDL_KEYDOWN:
            {
                int keyPressed = event.key.keysym.sym;
                enum JoypadButtons pressed = keyToEnum(keyPressed);
                if (pressed != UNKNOWN)
                    bus->pressButton(pressed);
               

            } break;
            case SDL_KEYUP:
            {
                int keyPressed = event.key.keysym.sym;
                enum JoypadButtons pressed = keyToEnum(keyPressed);
                if (pressed != UNKNOWN)
                    bus->depressButton(pressed);

            } break;
            };

            
        }

        //SDL_RenderClear(renderer.renderer);
        //if ((bus->io[0] & 0xf))
        //{

        //    SDL_SetRenderDrawColor(renderer.renderer, rand() % 255, rand() % 255, rand() % 255, 255);
        //    SDL_Rect rect;
        //    rect.x = 250;
        //    rect.y = 150;
        //    rect.w = 200;
        //    rect.h = 200;
        //    SDL_RenderFillRect(renderer.renderer, &rect);

        //    SDL_SetRenderDrawColor(renderer.renderer, rand() % 255, rand() % 255, rand() % 255, 255);
        //    SDL_Rect rect1;
        //    rect1.x = 200;
        //    rect1.y = 100;
        //    rect1.w = 200;
        //    rect1.h = 200;
        //    SDL_RenderFillRect(renderer.renderer, &rect1);


        //    SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 0, 255);

        //    
        //}
        //SDL_RenderPresent(renderer.renderer);


        ticksNow = SDL_GetTicks();
        tickDelta = ticksNow - ticksPrevious;





#if TURBO 1
        bus->emulate();
#else
        if (tickDelta > VSYNC)
        {
            std::cout << "fps: " << 1000 / tickDelta << std::endl;
            ticksPrevious = ticksNow;

            bus->emulate();
        }
#endif // TURBO

        SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer.renderer);

        for (int y = 0; y < YRES; y++)
        {
            for (int x = 0; x < XRES; x++)
            {
                //if (display == on)
                //{
                    auto pixel = bus->ppu.framebuffer[x + (XRES * y)];
                    SDL_SetRenderDrawColor(renderer.renderer, pixel.red, pixel.blue, pixel.green, pixel.alpha);
                    SDL_Rect r;
                    r.x = x * RES_SCALING;
                    r.y = y * RES_SCALING;
                    r.w = RES_SCALING;
                    r.h = RES_SCALING;
                    SDL_RenderFillRect(renderer.renderer, &r);
                //}
            }
        }

        SDL_RenderPresent(renderer.renderer);
        


        //std::cout << bus->cpu.fetch_decode_execute() << "\n";
        /*bus->cpu.fetch_decode_execute();
        bus->cpu.do_interrupts();*/
    }
   
    return 0;
}
