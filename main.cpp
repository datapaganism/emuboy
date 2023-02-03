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
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy\\roms\\blargg\\halt_bug.gb");
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy\\roms\\blargg\\02-interrupts.gb");
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "D:\\afk\\Downloads\\bgbw64\\bgbtest.gb");

    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\metasprites.gb");
<<<<<<< Updated upstream
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\Super Mario Land.gb");
   // std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\empty.gb");
=======
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\TETRIS.gb");
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\Users\\afk\\Downloads\\Nintendo - Game Boy\\Catrap (USA).gb");
    // std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\empty.gb");
>>>>>>> Stashed changes
    std::string bios_path = "..\\..\\..\\bios\\bios.bin";
    
    WindowManager application(rom_path, bios_path);

    application.run();

    return 0;
}
