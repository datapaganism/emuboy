#include "SDL.h"
#include "WindowManager.hpp"
#include <iostream>

//passed
// 06-ld r,r.gb
// 09-op r,r.gb
// 10-bit ops.gb
// 04-op r,imm.gb
// 05-op rp.gb
// 11 - op a, (hl).gb
// 08-misc instrs.gb
// 07-jr,jp,call,ret,rst.gb
// 03-op sp,hl.gb
// 01-special.gb
//02-interrupts.gb

//not passed


// Naming Convention

// macros / consts -> UPPERCASE
// classes / structs -> PascalCase
// functions -> camelCase
// variables / object instances -> under_score
// enums -> ePascalCase::under_score
// arrays of enums -> ePascalCase_all[]
// debug -> DEBUG_under_score

/*
    Prior to implementing this naming convention, there wasn't a defined convention in place, as developement progressed over time, naming styles started to diverge and began to get problatic as it was getting confusing to understand
    I settled on the defined convention after reading different styles online. I did not find any I agreed with and therefore I decided to create my own.
    I wanted a good seperation that makes it easy to understand what type of code we are looking at, through trial and error I selected what I believe worked best, however this now caused a great issue.
    All of my previous work now needs to be refactored to this new standard, Given that I use Visual Studio 2022 as my IDE, I was atleast given two tools that would make the transition easier.
    Firstly there is a dedicate Rename feature, this allows you to select text and select a new name and this is updated on all of the references in the code base.
    And I also used the manual method of Find All and Replace, this was used changing variable names used in functions en masse.
    After confirming that everything still builds and runs, I can now continue developing in a consistant manner.

    I also decided to include a comment of the naming convention, in my main.cpp file so anyone starting to read my code can get familiar with how it will be layed out.
*/


int main(int argc, char* argv[])
{
    
    /*if (argc == 1)
    {
        std::cout << "Usage:\nemuboy.exe <PATH TO ROM> <PATH TO BOOTROM (Optional)>\n\n";
        std::cout << "Controls:\nArrow Keys = D-Pad\nB, N = Start, Select\nA, S = a, b\n\n";
        std::cout << "Extra:\n2, 3 = VRAM Viewer, BG Map Viewer\n[, ] = change address range of BG MAP viewer\n8 = change colour palette\nP = pause\nSPACE = toggle fast forward\n+, - = scaling\n";
        std::cout << "\nSaves are stored in the same directory as the the ROM with a .sav extension\n";

        return 0;
    }*/
    
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\blargg\\full.gb");
    std::string bootrom_path = ((argv[2] != NULL) ? argv[2] : "..\\..\\..\\bios\\bios.bin");

    //rom_path = "..\\..\\..\\roms\\blargg\\instr_timing.gb";
    //rom_path = "..\\..\\..\\roms\\blargg\\halt_bug.gb";
    //rom_path = "..\\..\\..\\roms\\blargg\\interrupt_time.gb";
    //rom_path = "..\\..\\..\\roms\\mts\\emulator-only\\mbc1\\ram_256kb.gb";
    //rom_path = "D:\\afk\\Downloads\\bgbw64\\bgbtest.gb";
    //rom_path = "..\\..\\..\\roms\\metasprites.gb";
    //rom_path = "..\\..\\..\\roms\\empty.gb";
    rom_path = "..\\..\\..\\roms\\Super Mario Land.gb";
    //rom_path = "..\\..\\..\\roms\\Prince of Persia.gb";
    //rom_path = "..\\..\\..\\roms\\DR.MARIO.gb";
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Metroid II - Return of Samus (World).gb";
    //rom_path = "..\\..\\..\\roms\\POKEMON YELLOW.gbc";
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Donkey Kong (World) (Rev A) (SGB Enhanced).gb";
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Legend of Zelda, The - Link's Awakening (USA, Europe).gb";
    //rom_path = "C:\\Users\\afk\\Downloads\\Pokemon - Gold Version (USA, Europe) (SGB Enhanced).gbc";
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Pocket Monsters - Midori (Japan) (SGB Enhanced).gb";
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Balloon Kid (USA, Europe).gb";
    //rom_path =  "..\\..\\..\\roms\\empty.gb";
    
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Wave Race (USA, Europe).gb";
    //rom_path = "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy Color\\Dragon Quest Monsters 2 - Maruta no Fushigi na Kagi - Ruka no Tabidachi (Japan) (SGB Enhanced).gbc";
    //rom_path = "C:\\Users\\afk\\Downloads\\dmg-acid2.gb";

    if (argv[1] != NULL)
        rom_path = argv[1];

    if (argv[2] != NULL)
        bootrom_path = argv[2];
    
    WindowManager application(rom_path, bootrom_path);

    application.run();

    return 0;
}
