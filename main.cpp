#include "SDL.h"
#include "WindowManager.hpp"
#include <iostream>
#include <string>

//passed
//06-ld r,r.gb
//09-op r,r.gb
//10-bit ops.gb

//not passed
//11 - op a, (hl).gb
//08-misc instrs.gb
//07-jr,jp,call,ret,rst.gb
//05-op rp.gb
//04-op r,imm.gb
//03 - op sp, hl.gb
//02-interrupts.gb


int main(int argc, char* argv[])
{
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy-v2\\roms\\blargg\\05-op rp.gb");
    std::string bios_path = "C:\\dev\\repos\\emuboy-v2\\bios\\bios.binX";
    
    WindowManager application(rom_path, bios_path);
    application.run();
  
    return 0;
}