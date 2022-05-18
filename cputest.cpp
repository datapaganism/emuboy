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
    bus.set_memory(bus.cpu.registers.pc, 0xf3, MEMORY_ACCESS_TYPE::debug);
    bus.set_memory(bus.cpu.registers.pc+1, 0xfb, MEMORY_ACCESS_TYPE::debug);
    bus.set_memory(bus.cpu.registers.pc+2, 0x00, MEMORY_ACCESS_TYPE::debug);

    bus.set_memory(bus.cpu.registers.pc +3, 0xf3, MEMORY_ACCESS_TYPE::debug);
    bus.set_memory(bus.cpu.registers.pc +4, 0x00, MEMORY_ACCESS_TYPE::debug);

    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();
    bus.cpu.mStepCPU();

    return 0;
}