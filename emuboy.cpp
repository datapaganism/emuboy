#define _CRT_SECURE_NO_WARNINGS
#include "bus.hpp"
#include "renderer.hpp"

#define main SDL_main
#include "config.h"
#include <iostream>
#include <bitset>
#include <memory>
#include "joypad_mapping.h"
#include <string>



int main(int argv, char** args)
{
    BUS bus("./roms/TETRIS.gb", "bios.bin");
    RENDERER renderer;

    //bus.DEBUG_nintendo_logo();

  /*  for (int i = 0; i < 0x19; i++)
    {
        std::cout << i << "\n";
        TILE tile0(&bus, bus.ppu.get_tile_address_from_number(i, PPU::background));
        tile0.consolePrint();
        std::cout << "\n";
    }
    */
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


        
#if DEBUG 1
        int currentCycles = 0;

        
        int cyclesUsed = bus.cpu.fetch_decode_execute();
        currentCycles += cyclesUsed;
        bus.cpu.update_timers(cyclesUsed);
        bus.ppu.update_graphics(cyclesUsed);
        currentCycles += bus.cpu.do_interrupts();


      if (bus.cpu.registers.pc > 0x5d)
            renderer.render_frame(&bus);
        #else

        // Emulate a single frame's worth of CPU instructions
        if (tickDelta > VSYNC)
        {
            //std::cout << "fps: " << 1000 / tickDelta << std::endl;
            ticksPrevious = ticksNow;

            bus.cycle_system_one_frame();
        }

        /*int currentCycles = 0;

        while (currentCycles < CYCLES_PER_FRAME)
        {
            int cyclesUsed = 1;
            currentCycles += cyclesUsed;
            bus.ppu.update_graphics(cyclesUsed);
            renderer.render_frame(&bus);
        }*/

        //Render framebuffer
        renderer.render_frame(&bus);
        std::string windowTitle("SCX: ");
        windowTitle.append(std::to_string(bus.get_memory(SCX)));
        windowTitle.append(" SCY: ");
        windowTitle.append(std::to_string(bus.get_memory(SCY)));
        windowTitle.append(" fps: ");
        windowTitle.append(std::to_string(tickDelta));
        SDL_SetWindowTitle(renderer.window, windowTitle.data());
        
#endif


    }
   
    return 0;
}
