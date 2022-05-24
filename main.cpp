#define _CRT_SECURE_NO_WARNINGS

#include "bus.hpp"
#include "renderer.hpp"
#include "config.h"
#include <iostream>
#include <bitset>
#include <memory>
#include "joypad_mapping.h"
#include <string>
#include <sstream>


#define to_hex_str(hex_val) (static_cast<std::stringstream const&>(std::stringstream() << "0x" << std::hex << hex_val)).str()


#define main SDL_main
int main(int argv, char** args)
{
    //BUS bus("./roms/TETRIS.gb", "bios.bin");
    BUS bus("C:\\dev\\repos\\emuboy-v2\\roms\\blargg\\01-read_timing.gb", "bio s.bin");
    RENDERER renderer;

    /*bus.set_memory(0xFF01,0x30);
    bus.set_memory(0xFF02,0x81);*/

   // SDL_RendererInfo info;
   // SDL_GetRendererInfo(renderer.renderer, &info);
   //std::cout << "Renderer name: " << info.name << std::endl;
   //std::cout << "Texture formats: " << std::endl;
   // for (Uint32 i = 0; i < info.num_texture_formats; i++)
   // {
   //     std::cout << SDL_GetPixelFormatName(info.texture_formats[i]) << std::endl;
   // }

    //bus.DEBUG_nintendo_logo();

  /*  for (int i = 0; i < 0x19; i++)
    {
        std::cout << i << "\n";
        TILE tile0(&bus, bus.ppu.get_tile_address_from_number(i, PPU::background));
        tile0.consolePrint();
        std::cout << "\n";
    }
    */


    //mapping from sdl physical keys to virtual buttons for gameboy
   
    // the steps of emulation during the while loop
    // 1. modify input registers as SDL polls for an event
    // 2. run cpu, update timers, run graphics and sound chips, and do interrupts
    // 3. renderer displays graphics from emulated state
    // rinse and repeat
    unsigned int ticksNow = 0, ticksPrevious = 0;
    double tickDelta = 0;
    while (renderer.isRunning)
    {
        // Process events and inputs
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
                    bus.pressButton(pressed);
            } break;
            case SDL_KEYUP:
            {
                int keyPressed = event.key.keysym.sym;
                enum JoypadButtons pressed = keyToEnum(keyPressed);
                if (pressed != UNKNOWN)
                    bus.pressButton(pressed);
            } break;
            };
        }
        ticksNow = SDL_GetTicks();
        tickDelta = ticksNow - ticksPrevious;


        


        // Emulate a single frame's worth of CPU instructions
        if (tickDelta > 1000/VSYNC)
        {
            //std::cout << "fps: " << 1000 / tickDelta << std::endl;
            ticksPrevious = ticksNow;

            bus.cycle_system_one_frame();
            renderer.render_frame(&bus);

            std::string windowTitle("SCX: ");
            windowTitle.append(std::to_string(bus.get_memory(SCX, MEMORY_ACCESS_TYPE::debug)));
            windowTitle.append(" SCY: ");
            windowTitle.append(std::to_string(bus.get_memory(SCY, MEMORY_ACCESS_TYPE::debug)));
            //windowTitle.append(" LY: ");
            //windowTitle.append(std::to_string(bus.get_memory(LY)));
            windowTitle.append(" fps: ");
            //windowTitle.append(std::to_string(std::pow(tickDelta,-1)));
            windowTitle.append(std::to_string(1000 / tickDelta));

            windowTitle.append(" PC: ");
            windowTitle.append(to_hex_str((bus.cpu.registers.pc)));

            windowTitle.append(" JOY: ");
            windowTitle.append(to_hex_str(bus.get_memory(0xFF00, MEMORY_ACCESS_TYPE::debug)));

            SDL_SetWindowTitle(renderer.window, windowTitle.data());
            

        }

        
    }
   
    return 0;
}
