#include "bus.hpp"


BUS::BUS()
{
    this->cpu.connect_to_bus(this);
    this->ppu.connect_to_bus(this);
    this->init();
}

BUS::BUS(const std::string game_name, const std::string bios_name) : BUS::BUS()
{
    this->gamepak = GAMEPAK(game_name);
    this->load_bios(bios_name);
}


void BUS::depressButton(const enum JoypadButtons button)
{
    switch (button)
    {
    case dRight:
    case bA: { this->io.at(0xFF00 - IOOFFSET) |= (0b1 << 0) ; } break;
    
    case dLeft:
    case bB: { this->io.at(0xFF00 - IOOFFSET) |= (0b1 << 1) ; } break;
    
    case dUp:
    case bSelect: { this->io.at(0xFF00 - IOOFFSET) |= (0b1 << 2) ; } break;
    
    case dDown:
    case bStart: { this->io.at(0xFF00 - IOOFFSET) |= (0b1 << 3) ; } break;
    }
}

void BUS::pressButton(const enum JoypadButtons button)
{
    switch (button)
    {
    case dRight:
    case bA: { this->io.at(0xFF00 - IOOFFSET) &= ~(0b1 << 0) ; } break;
    
    case dLeft:
    case bB: { this->io.at(0xFF00 - IOOFFSET) &= ~(0b1 << 1) ; } break;
    
    case dUp:
    case bSelect: { this->io.at(0xFF00 - IOOFFSET) &= ~(0b1 << 2) ; } break;
    
    case dDown:
    case bStart: { this->io.at(0xFF00 - IOOFFSET) &= ~(0b1 << 3) ; } break;
    }

    this->cpu.request_interrupt(joypad);
}


// The gameboy is a memory mapped system, components are mapped
// to indiviudial parts of the addressable memory, we can specify
// what hardware we are reading from using if guards
Byte BUS::get_memory(const Word address)
{
    // boot rom area, or rom bank 0
    if (address <= 0x00ff) //from 0x0000
    {
        // if the bios has never been loaded or if the register at 0xFF50 is set 1 (which is done by the bios program) we need to access the cartridge bank
        if (this->get_memory(0xFF50) == 0x1|| !this->biosLoaded)
            return this->gamepak.rom[address];
        
        return this->bios[address];
    }

    if (address <= 0x3fff) //from 0x0100
    {

        // game rom bank 0
        return this->gamepak.rom[address];
    }

    if (address <= 0x7fff) // from 0x4000
    {
        // game rom bank N

        //banking is not implemented but we will just now read the whole cart
        return this->gamepak.rom[address];
        return 0b0;
    }

    if (address <= 0x97ff) // from 0x8000         
    {
        // Tile Ram region
        //return this->video_ram.at(address - 0x8000);
        return 0b0;
    }

    if (address <= 0x9FFF) // from 0x9800
    {
        // background map region
        return this->video_ram.at(address - 0x8000);
        //return 0b0;
    }

    if (address <= 0xBFFF) // from 0xA000
    {
        // cartridge ram
        return 0b0;
    }

    if (address <= 0xDFFF) // from 0xC000
    {
        // working ram
        return this->work_ram.at(address - 0xC000);
    }

    if (address <= 0xFDFF) // from 0xE000
    {
        // echo ram, it is a copy of the ram above, we will just read memory 2000 addresses above instead
        return this->get_memory(address - 0x2000);
    }

    if (address <= 0xFE9F) // from 0xFE00
    {
        // object attribute memory
        return 0b0;
    }

    if (address <= 0xFEFF) // from 0xFEA0
    {
        // unused part of the map, must retun 0
        return 0;
    }
    if (address <= 0xFF4B) // from 0xFF00
    {
        // i/o registers
        return this->io.at(address - 0xFF00);
    }
    if (address <= 0xFF7F) // from 0xFF4C
    {
        // unused part of the map, just return
        return 0b0;
    }
    if (address <= 0xFFFE) // from 0xFF80
    {
        // high ram area
        return this->high_ram.at(address-0xFF80);
    }
    if (address <= 0xFFFF) // from 0xFFFF, yep
    {
        // interrupt enabled register, cannot be accessed
        return this->interrupt_enable_register;
        return 0b0;
    }

    // temp return
    return 0;
};

void BUS::set_memory(const Word address, const Byte data)
{
    if (address <= 0x00ff)
    {
        // boot rom area, or rom bank 0
        return;
    }

    if (address <= 0x3fff)
    {
        // game rom bank 0
        return;
    }

    if (address <= 0x7fff)
    {
        // game rom bank N
        return;
    }

    if (address <= 0x97ff)
    {

        // Tile Ram region
        this->video_ram.at(address - 0x8000) = data;
        
        return;
    }

    if (address <= 0x9FFF)
    {

        // background map region
        this->video_ram.at(address - 0x8000) = data;
        return;
    }

    if (address <= 0xBFFF)
    {
        // cartridge ram
        return;
    }

    if (address <= 0xDFFF)
    {
        // working ram
        this->work_ram.at(address - 0xC000) = data;
        return;
    }

    if (address <= 0xFDFF)
    {
        // echo ram, it is a copy of the ram above, any calls to this address space will be redirected to address - 2000 spaces above
        return this->set_memory(address - 0x2000, data);
    }

    if (address <= 0xFE9F)
    {
        // object attribute memory
        return;
    }

    if (address <= 0xFEFF)
    {
        // unused part of the map, just return
        return;
    }
    if (address <= 0xFF4B)
    {
        //if changing timers
        if (address == TAC)
        {
            this->io.at(address - 0xFF00) = data;
            this->cpu.update_timerCounter();
            return;
        }
        if (address == DIV)
        {
            this->io.at(address - 0xFF00) = 0;
            return;
        }

        // i/o registers

        // LY register is read only
        if (address == LY)
            return;

        this->io.at(address - 0xFF00) = data;
        return;
    }
    if (address <= 0xFF7F)
    {
        // unused part of the map, just return
        return;
    }
    if (address <= 0xFFFE)
    {
        // high ram area
        this->high_ram.at(address - 0xFF80) = data;
        return;
    }
    if (address <= 0xFFFF)
    {
        // interrupt enabled register, write only data
        this->interrupt_enable_register = data;

        return;
    }
}
void BUS::set_memory_word(const Word address, const Word data)
{
    //store in little endian byte order 
    this->set_memory(address, (data & 0x00ff));
    this->set_memory(address + 1, ((data & 0xff00) >> 8));
}

const Word BUS::get_memory_word_lsbf(const Word address)
{
        return (this->get_memory(address) | (this->get_memory(address + 1) << 8));
}

void BUS::emulate()
{
    int currentCycles = 0;

    while (currentCycles < CYCLES_PER_FRAME)
    {
        int cyclesUsed = this->cpu.fetch_decode_execute();
        currentCycles += cyclesUsed;
        this->cpu.update_timers(cyclesUsed);
        //updategraphics
        currentCycles += this->cpu.do_interrupts();
    }

    //update emulator screen
}




/// <summary>
/// A function to load the bios into memory, as the bios is property of Nintendo, we can not legally release this emulator with the bios,
/// however we can allow users to supply their own bios file, named bios.bin, this function will set the program counter of the CPU to execute the bios program if it exists.  
/// </summary>
/// <param name="bios_name"></param>
void BUS::load_bios(const std::string bios_name)
{
    std::ifstream file(bios_name, std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        std::ifstream::pos_type pos = file.tellg();

        file.seekg(0, std::ios::beg);
        file.read((char*)this->bios.data(), pos);

        //set the program counter of the cpu to execute the bios program
        this->cpu.registers.pc = 0x0000;
        this->biosLoaded = true;
    }
    file.close();

};


// initializes the hardware registers on startup
void BUS::init()
{
    this->set_memory(0xFF00,0xCF); // P1
    this->set_memory(0xFF01,0x00); // SB
    this->set_memory(0xFF02,0x7E); // SC
    this->set_memory(0xFF04,0xAB); // DIV
    this->set_memory(0xFF05,0x00); // TIMA
    this->set_memory(0xFF06,0x00); // TMA
    this->set_memory(0xFF07,0xF8); // TAC
    this->set_memory(0xFF0F,0xE1); // IF
    this->set_memory(0xFF10,0x80); // NR10
    this->set_memory(0xFF11,0xBF); // NR11
    this->set_memory(0xFF12,0xF3); // NR12
    this->set_memory(0xFF13,0xFF); // NR13
    this->set_memory(0xFF14,0xBF); // NR14
    this->set_memory(0xFF16,0x3F); // NR21
    this->set_memory(0xFF17,0x00); // NR22
    this->set_memory(0xFF18,0xFF); // NR23
    this->set_memory(0xFF19,0xBF); // NR24
    this->set_memory(0xFF1A,0x7F); // NR30
    this->set_memory(0xFF1B,0xFF); // NR31
    this->set_memory(0xFF1C,0x9F); // NR32
    this->set_memory(0xFF1D,0xFF); // MR33
    this->set_memory(0xFF1E,0xBF); // MR34
    this->set_memory(0xFF20,0xFF); // NR41
    this->set_memory(0xFF21,0x00); // NR42
    this->set_memory(0xFF22,0x00); // NR44
    this->set_memory(0xFF23,0xBF); // NR50
    this->set_memory(0xFF24,0x77); // NR51
    this->set_memory(0xFF25,0xF3); // LCDC
    this->set_memory(0xFF26,0xF1); // STAT
    this->set_memory(0xFF40,0x91); // SCY
    this->set_memory(0xFF41,0x85); // SCX
    this->set_memory(0xFF42,0x00); // LY
    this->set_memory(0xFF43,0x00); // LYC
    this->set_memory(0xFF44,0x00); // DMA
    this->set_memory(0xFF45,0x00); // BGP
    this->set_memory(0xFF46,0xFF); // 0BP0
    this->set_memory(0xFF47,0xFC); // 0BP1
    this->set_memory(0xFF48,0xFF); // WY
    this->set_memory(0xFF49,0xFF); // WX
    this->set_memory(0xFF4A,0x00); // KEY1
    this->set_memory(0xFF4B,0x00); // VBK
    this->set_memory(0xFF4D,0xFF); // HDMA1
    this->set_memory(0xFF4F,0xFF); // HDMA2
    this->set_memory(0xFF51,0xFF); // HDMA3
    this->set_memory(0xFF52,0xFF); // HDMA4
    this->set_memory(0xFF53,0xFF); // HDMA5
    this->set_memory(0xFF56,0xFF); // RP
    this->set_memory(0xFF68,0xFF); // BCPS
    this->set_memory(0xFF69,0xFF); // BCPD
    this->set_memory(0xFF6A,0xFF); // OCPS
    this->set_memory(0xFF6B,0xFF); // OCPD
    this->set_memory(0xFF70,0xFF); // SVBK
    this->set_memory(0xFFFF,0x00); // IE
}