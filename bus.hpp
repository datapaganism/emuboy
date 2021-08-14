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
    std::array<Byte, 0x2000> work_ram;
    std::array<Byte, 0x0100> bios;

    BUS();
    BUS(const std::string game_name, const std::string bios_name);
    void init();
    Byte get_memory(Word address);
    void set_memory(Word address,Byte data);

    bool biosPostComplete = false;
    bool biosLoaded = false;

private:
    void load_bios(const std::string bios_name);
    

};