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
    //std::unique_ptr<BUS> bus;
    //bus = std::make_unique<BUS>("./roms/TETRIS.gb", "bios.bin");
    BUS bus("./roms/TETRIS.gb", "bios.bin");
    RENDERER renderer;

    // testing fifo
    //bus.ppu.fifo_bg.push(FIFO_pixel(01,01,1,1));

    //bus.ppu.fifo_bg.pop();
    //bus.ppu.fifo_bg.pop();

    ////bus.ppu.fifo_bg.push(FIFO_pixel(01, 01, 1, 1));

    //bus.video_ram[0x8010 - 0x8000] = 0xF0;
    //bus.video_ram[0x8011 - 0x8000] = 0x00;
    //bus.video_ram[0x8012 - 0x8000] = 0xF0;
    //bus.video_ram[0x8013 - 0x8000] = 0x00;
    //bus.video_ram[0x8014 - 0x8000] = 0xFC;
    //bus.video_ram[0x8015 - 0x8000] = 0x00;
    //bus.video_ram[0x8016 - 0x8000] = 0xFC;
    //bus.video_ram[0x8017 - 0x8000] = 0x00;
    //bus.video_ram[0x8018 - 0x8000] = 0xFC;
    //bus.video_ram[0x8019 - 0x8000] = 0x00;
    //bus.video_ram[0x801a - 0x8000] = 0xFC;
    //bus.video_ram[0x801b - 0x8000] = 0x00;
    //bus.video_ram[0x801c - 0x8000] = 0xF3;
    //bus.video_ram[0x801d - 0x8000] = 0x00;
    //bus.video_ram[0x801e - 0x8000] = 0xF3;
    //bus.video_ram[0x801f - 0x8000] = 0x00;

    //int i = 0;
    //TILE tile0(&bus, 0x8010+(16*i));
    //tile0.consolePrint();
    //
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


    bus.cpu.DEBUG_printCurrentState();
    
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


        //Render framebuffer

        renderer.render_frame(&bus);
#endif


    }
   
    return 0;
}
