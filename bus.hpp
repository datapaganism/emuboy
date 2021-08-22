#pragma once

#include "cpu.hpp"
#include "gamepak.hpp"
#include "config.h"
#include <array>

class BUS
{
public:

    CPU cpu;
    GAMEPAK gamepak;
    std::array<Byte, 0x2000> work_ram = {0,};
    std::array<Byte, 0x0100> bios = { 0, };
    std::array<Byte, 0x004C> io = { 0, };
    std::array<Byte, 0x007F> high_ram = { 0, };
    Byte interrupt_enable_register = 0;

    BUS();
    BUS(const std::string game_name, const std::string bios_name);
    void init();
    Byte get_memory(const Word address);
    void set_memory(const Word address, const Byte data);
    void set_memory_word(const Word address, const Word data);

    bool biosLoaded = false;

private:
    void load_bios(const std::string bios_name);
    

};