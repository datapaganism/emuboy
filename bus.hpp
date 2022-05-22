#pragma once
#include <array>
#include <memory>

#include "cpu.hpp"
#include "gamepak.hpp"
#include "ppu.hpp"
#include "config.hpp"
#include "joypad_mapping.hpp"
#include "dma_controller.hpp"



enum MEMORY_ACCESS_TYPE
{
    dma_controller,
    ppu,
    cpu,
    interrupt_handler,
    debug,
};

class BUS
{
public:
    BUS(const std::string rom_path, const std::string bios_path);

    
    PPU ppu;
    CPU cpu;
    GAMEPAK gamepak;
    DMA_CONTROLLER dma_controller;

    std::unique_ptr<Byte[]> work_ram  = std::make_unique<Byte[]>(0x2000);
    std::unique_ptr<Byte[]> bios      = std::make_unique<Byte[]>(0x100);
    std::unique_ptr<Byte[]> io        = std::make_unique<Byte[]>(0x80);
    std::unique_ptr<Byte[]> high_ram  = std::make_unique<Byte[]>(0x7F);
    std::unique_ptr<Byte[]> video_ram = std::make_unique<Byte[]>(0x2000);
    std::unique_ptr<Byte[]> oam_ram   = std::make_unique<Byte[]>(0x100);
    Byte interrupt_enable_register = 0;

    Byte directionButtonsState = 0xFF;
    Byte actionButtonsState = 0xFF;
    


    
    std::unique_ptr<FRAMEBUFFER_PIXEL[]> framebuffer = std::make_unique<FRAMEBUFFER_PIXEL[]>(XRES * YRES);
    

    Byte DEBUG_ascii_to_hex(char character);
    // replaces area of memory with opcode string, useful for writing quick debugging programs
    void DEBUG_opcode_program(Word address, std::string byteString);
    bool DEBUG_PC_breakpoint_hit = false;
    Byte* ptr = &this->work_ram[0xdff0 - WORKRAMOFFSET];
   
    void init();
    void bios_init();
    Byte get_memory(const Word address, enum MEMORY_ACCESS_TYPE access_type);
    void set_memory(const Word address, const Byte data, enum MEMORY_ACCESS_TYPE access_type);
    void set_memory_word(const Word address, const Word data, enum MEMORY_ACCESS_TYPE access_type);

    const Word get_memory_word_lsbf(const Word address, enum MEMORY_ACCESS_TYPE access_type);
    
    Byte DEBUG_get_memory(const Word address);
    void DEBUG_set_memory(const Word address, const Byte data);
    void DEBUG_set_memory_word(const Word address, const Word data);
    const Word DEBUG_get_memory_word_lsbf(const Word address);

    
    /// <summary>
    /// Cycles the emulated system state by one frame (~70,221 cycles)
    /// </summary>
    void cycle_system_one_frame();
    
    bool biosLoaded = false;

    void pressButton(const enum JoypadButtons button);
    void depressButton(const enum JoypadButtons button);

    void DEBUG_fill_ram(Word address, std::string byteString);
    void DEBUG_nintendo_logo();

    void load_bios(const std::string bios_name);
    

};

