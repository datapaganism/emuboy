#pragma once


#include "cpu.hpp"
#include "gamepak.hpp"
#include "ppu.hpp"
#include "config.h"
#include <array>
#include "joypad_mapping.h"


//forward declaration of JoypadButtons mapping
//enum JoypadButtons : int;



class BUS
{
public:

    PPU ppu;
    CPU cpu;
    GAMEPAK gamepak;
    std::array<Byte, 0x2000> work_ram = {0,};
    std::array<Byte, 0x0100> bios = { 0, };
    std::array<Byte, 0x004C> io = { 0, };
    std::array<Byte, 0x007F> high_ram = { 0, };
    std::array<Byte, 0x2000> video_ram = { 0, };
    Byte interrupt_enable_register = 0;
    
    Byte DEBUG_ascii_to_hex(char character);
    /// <summary>
    /// replaces area of memory with opcode string, useful for writing quick debugging programs
    /// </summary>
    /// <param name="address"></param>
    /// <param name="byteString"></param>
    int DEBUG_opcode_program(Word address, std::string byteString, int cycles);


    BUS();
    BUS(const std::string game_name, const std::string bios_name);
    void init();
    Byte get_memory(const Word address);
    void set_memory(const Word address, const Byte data);
    void set_memory_word(const Word address, const Word data);

    const Word get_memory_word_lsbf(const Word address);
    
    
    /// <summary>
    /// Cycles the emulated system state by one frame (~70,221 cycles)
    /// </summary>
    void cycle_system_one_frame();
    

    bool biosLoaded = false;

    void pressButton(const enum JoypadButtons button);
    void depressButton(const enum JoypadButtons button);



private:
    void load_bios(const std::string bios_name);
    

};