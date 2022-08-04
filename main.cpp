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

// Classes / structs -> PascalCase
// Functions -> camelCase
// variables / object instances -> under_score
// enums -> ePascalCase::under_score
// arrays of enums -> ePascalCase_all_optional[]
// debug -> DEBUG_under_score



int main(int argc, char* argv[])
{
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy\\roms\\blargg\\halt_bug.gb");
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy\\roms\\blargg\\02-interrupts.gb");
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "D:\\afk\\Downloads\\bgbw64\\bgbtest.gb");
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\TETRIS.gb");
    //std::string rom_path = ((argv[1] != NULL) ? argv[1] : "..\\..\\..\\roms\\blargg\\02-interrupts.gb");
    std::string bios_path = "..\\..\\..\\bios\\bios.bin";
    
    WindowManager application(rom_path, bios_path);
    application.run();
  
    return 0;
}
