#include "SDL.h"
#include "WindowManager.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy-v2\\roms\\blargg\\03-op sp,hl.gb");
    std::string bios_path = "C:\\dev\\repos\\emuboy-v2\\bios\\bios.bin";
    
    WindowManager application(rom_path, bios_path);
    application.run();
  
    return 0;
}