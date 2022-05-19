#include "cpu.hpp"
#include "bus.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string rom_path = ((argv[1] != NULL) ? argv[1] : "C:\\dev\\repos\\emuboy-v2\\roms\\blargg\\03-op sp,hl.gb");
    std::string bios_path = "C:\\dev\\repos\\emuboy-v2\\bios\\bios.binX";

    BUS bus(rom_path, bios_path);
    bus.cpu.registers.pc = 0xC000;
    bus.DEBUG_opcode_program(0xC000, "31 12 43 08 00 C0");

    Byte temp = 0;
    while(1)
    {
        bus.cpu.mStepCPU();
        temp = bus.get_memory(0xC000, MEMORY_ACCESS_TYPE::debug);
    }
    return 0;
}