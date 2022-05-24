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

    bool biosLoaded = false;
    std::unique_ptr<Byte[]> work_ram  = std::make_unique<Byte[]>(0x2000);
    std::unique_ptr<Byte[]> bios      = std::make_unique<Byte[]>(0x100);
    std::unique_ptr<Byte[]> io        = std::make_unique<Byte[]>(0x80);
    std::unique_ptr<Byte[]> high_ram  = std::make_unique<Byte[]>(0x7F);
    std::unique_ptr<Byte[]> video_ram = std::make_unique<Byte[]>(0x2000);
    std::unique_ptr<Byte[]> oam_ram   = std::make_unique<Byte[]>(0x100);
    Byte interrupt_enable_register = 0;

    std::unique_ptr<FRAMEBUFFER_PIXEL[]> framebuffer = std::make_unique<FRAMEBUFFER_PIXEL[]>(XRES * YRES);
   
    void init();
    void bios_init();
    void load_bios(const std::string bios_name);
    void cycle_system_one_frame();

    Byte get_memory(const Word address, enum MEMORY_ACCESS_TYPE access_type);
    void set_memory(const Word address, const Byte data, enum MEMORY_ACCESS_TYPE access_type);
    
    Byte joypadState = 0xFF;
    void set_joypadState(const enum JoypadButtons button, bool value);
    void pressButton(const enum JoypadButtons button);
    void depressButton(const enum JoypadButtons button);
    Byte getActionButtonNibble();
    Byte getDirectionButtonNibble();
    
    Byte DEBUG_get_memory(const Word address);
    void DEBUG_set_memory(const Word address, const Byte data);
    Byte DEBUG_ascii_to_hex(char character);
    void DEBUG_opcode_program(Word address, std::string byteString);
    bool DEBUG_PC_breakpoint_hit = false;
    void DEBUG_fill_ram(Word address, std::string byteString);
    void DEBUG_nintendo_logo();
};

