#pragma once
#include <array>
#include <memory>

#include "cpu.hpp"
#include "gamepak.hpp"
#include "ppu.hpp"
#include "config.hpp"
#include "joypad_mapping.hpp"
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
public:
    BUS(const std::string rom_path, const std::string bios_path);

    PPU ppu;
    CPU cpu;
    GamePak gamepak;
    DMA_Controller dma_controller;

    bool bios_loaded = false;
    std::unique_ptr<Byte[]> work_ram  = std::make_unique<Byte[]>(0x2000);
    std::unique_ptr<Byte[]> bios      = std::make_unique<Byte[]>(0x100);
    std::unique_ptr<Byte[]> io        = std::make_unique<Byte[]>(0x80);
    std::unique_ptr<Byte[]> high_ram  = std::make_unique<Byte[]>(0x7F);
    std::unique_ptr<Byte[]> video_ram = std::make_unique<Byte[]>(0x2000);
    std::unique_ptr<Byte[]> oam_ram   = std::make_unique<Byte[]>(0x100);
    Byte interrupt_enable_register = 0;

    std::unique_ptr<FramebufferPixel[]> framebuffer = std::make_unique<FramebufferPixel[]>(XRES * YRES);

    uint64_t DEBUG_mCycle_counter = 0;
   
    void init();
    void biosInit();
    void loadBios(const std::string bios_name);
    void cycleSystemOneFrame();

    Byte getMemory(const Word address, enum eMemoryAccessType access_type);
    void setMemory(const Word address, const Byte data, enum eMemoryAccessType access_type);
    
    Byte joypad_state = 0xFF;
    void setJoypadState(const enum eJoypadButtons button, bool value);
    void pressButton(const enum eJoypadButtons button);
    void depressButton(const enum eJoypadButtons button);
    Byte getActionButtonNibble();
    Byte getDirectionButtonNibble();
    
    Byte DEBUG_get_memory(const Word address);
    void DEBUG_set_memory(const Word address, const Byte data);
    Byte DEBUG_ascii_to_hex(char character);
    void DEBUG_opcode_program(Word address, std::string byteString);
    bool DEBUG_PC_breakpoint_hit = false;
    void DEBUG_fill_ram(Word address, std::string byteString);
    void DEBUG_nintendo_logo();
    void DEBUG_print_ASCII_from_serial();
};

