#pragma once
#include <array>
#include <memory>
#include <fstream>

#include "cpu.hpp"
#include "gamepak.hpp"
#include "ppu.hpp"
#include "config.hpp"
#include "dma_controller.hpp"


enum eMemoryAccessType
{
    dma_controller,
    ppu,
    cpu,
    interrupt_handler,
    debug,
};

class BUS
{
private:
    std::string rom_path;
    std::string bios_path;
public:
    BUS() {
        cpu.connectToBus(this);
    };

    CPU cpu;
    
    Byte interrupt_enable_register = 0;

    Byte flat_ram[0xFFFF + 1] = { 0 };


    uint64_t DEBUG_mCycle_counter = 0;

    Byte getMemory(const Word address, enum eMemoryAccessType access_type) {
        if (address == 0xFFFF)
        {
            return interrupt_enable_register;
        }
        return flat_ram[address];
    }
    void setMemory(const Word address, const Byte data, enum eMemoryAccessType access_type)
    {
        if (address == 0xFFFF)
        {
            interrupt_enable_register = data;
            return;
        }
        flat_ram[address] = data;
    }
};

