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
    BUS();
    BUS(const std::string rom_path, const std::string bios_path);

    PPU ppu;
    CPU cpu;
    GamePak gamepak;
    DMA_Controller dma_controller;
    //Logger logger;

    bool bios_loaded = false;
    Byte work_ram[0x2000] = { 0, };
    Byte bios[0x100] = { 0, };
    Byte io[0x80] = { 0, };
    Byte high_ram[0x80] = {0,};
    Byte video_ram[0x2000] = { 0, };
    Byte oam_ram[0x100] = { 0, };
    Byte interrupt_enable_register = 0;

    FramebufferPixel framebuffer[XRES * YRES];

    uint64_t DEBUG_mCycle_counter = 0;
   
    void init();
    void biosInit();
    void loadBios(const std::string bios_name);
    void cycleSystemOneFrame();

    Byte getMemory(const Word address, enum eMemoryAccessType access_type);
    void setMemory(const Word address, const Byte data, enum eMemoryAccessType access_type);
    
    Byte joypad_state = 0xFF;
    void setJoypadState(const int button_bit, bool value);
    void pressButton(const int button_bit);
    void depressButton(const int button_bit);
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

    void saveState();
};

