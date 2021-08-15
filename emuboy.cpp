#define _CRT_SECURE_NO_WARNINGS
#include "bus.hpp"
#include "renderer.hpp"

#define main SDL_main
#include "config.h"
#include <iostream>
#include <bitset>


int main(int argv, char** args)
{

    BUS bus("gamepak.b in", "bios.b in");
    RENDERER renderer;

    int testnumber = 0xff;
    int wrAddr = WORKRAMOFFSET;
    bus.cpu.registers.pc = wrAddr;
    bus.set_memory(wrAddr, 0x06);
    bus.set_memory(wrAddr + 1, testnumber);
    bus.cpu.fetch_decode_execute();
    if (bus.cpu.registers.b == testnumber)
        std::cout << "it works";
   

    //bus.set_memory(0xc000, 0xff);
    //bus.set_memory(0xdfff, 0xf0);
    //std::cout << std::hex << (int)bus.work_ram[0] << "\n";
    //std::cout << std::hex << (int)bus.get_memory(0xe000) << "\n";
    //std::cout << std::hex << (int)bus.work_ram[0x2000 - 1] << "\n";



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
    // cpu.registers.a = 0xFF;
    // cpu.registers.b = 0xF0;
    // cpu.registers.f = 0b01000000;

    // cpu.registers.set_word(&cpu.registers.a,&cpu.registers.b, 0x1234);

    // std::cout << (int)cpu.registers.a << "\n";
    // std::cout << (int)cpu.registers.f << "\n";

    // std::cout << std::hex << cpu.registers.get_word(&cpu.registers.a,&cpu.registers.f) << "\n";
    // std::cout << std::hex <<  cpu.registers.get_word(&cpu.registers.b,&cpu.registers.c) << "\n";
    // std::cout << std::hex <<  cpu.registers.get_word(&cpu.registers.d,&cpu.registers.e) << "\n";
    // std::cout << std::hex <<  cpu.registers.get_word(&cpu.registers.h,&cpu.registers.l) << "\n";

    // cpu.registers.set_flag(z,0);
    // std::cout << (int)cpu.registers.get_flag(z) << "\n";
    // cpu.registers.set_flag(z,1);
    // std::cout << (int)cpu.registers.get_flag(z) << "\n";

    // uint32_t a = CYCLES_PER_FRAME;

    // std::cout << a << "\n";


    // cpu.registers.a = 0x8E;
    // std::cout << "reg a " << std::bitset<8>(cpu.registers.a) << "\n";
    // std::cout << "nib a " << std::bitset<4>(cpu.registers.get_nibble(&cpu.registers.a,true)) << "\n";
    // std::cout << "reg a " << std::bitset<8>(cpu.registers.a) << "\n";
    // cpu.registers.set_nibble(&cpu.registers.a,0b1001,false);
    //  std::cout << "reg a " << std::bitset<8>(cpu.registers.a) << "\n";
    // std::cout << "nib a " << std::bitset<4>(cpu.registers.get_nibble(&cpu.registers.a,true)) << "\n";


    return 0;
}
