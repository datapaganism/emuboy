#include <iterator>

#include "bus.hpp"

#include <iostream>
#include <bitset>
#include <sstream>
#include <fstream>


void BUS::cycleSystemOneFrame()
{
    int current_cycles = 0;

    while (current_cycles <= CYCLES_PER_FRAME)
    {

        /*if (cpu.registers.pc == 0x100)
            __debugbreak();*/

        this->cpu.mStepCPU();
        this->cpu.updateTimers();
        
        this->dma_controller.updateDMA(4);

        this->ppu.updateGraphics(4);

        current_cycles++;

        // Serial monitoring
        if ((this->io[0xFF02 - IOOFFSET] & ~0x7E) == 0x81)
        {
            char c = this->io[0xFF01 - IOOFFSET];
            if (c == ' ')
            {
                this->cpu.debug_toggle = true;
                printf("");
            }
            if (c != 0)
            {
                printf("%c", c);
                this->io[0xFF02 - IOOFFSET] = 0x0;
            }
        }
    }
    
}



void BUS::setJoypadState(const enum eJoypadButtons button, bool value)
{
    // joypadstate
    // 0000 0000
    // ACT  DIR
    
    Byte bit = 0;
    switch (button)
    {
    case d_right:    {bit = 0b00000001; break;}
    case d_left:     {bit = 0b00000010; break;}
    case d_up:       {bit = 0b00000100; break;}
    case d_down:     {bit = 0b00001000; break;}
    case b_a:        {bit = 0b00010000; break;}
    case b_b:        {bit = 0b00100000; break;}
    case b_select:   {bit = 0b01000000; break;}
    case b_start:    {bit = 0b10000000; break;}
    default: fprintf(stderr, "Unreachable button press");  exit(-1); break;
    }
    this->joypad_state = (value) ? (this->joypad_state | bit) : (this->joypad_state & ~bit);
}




Byte BUS::DEBUG_ascii_to_hex(char character)
{
    Byte temp = 0x0;
    switch (character)
    {
    case '0': case '1': case '2':case '3':    case '4':    case '5':    case '6':    case '7':    case '8':    case '9': 
    {
        temp = character - '0';
        return temp;
        break;
    }
    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
    {
        temp = (character - 'A') + 0xA;
        return temp;
        break;
    }
    default:
        return 0;
    }
}

// 3E 18 06 FF 90
void BUS::DEBUG_opcode_program(Word address, std::string byteString)
{

    std::vector<Byte> byteArray;

    Word temp = 0x0;

    
    for (auto itr = byteString.begin(); itr != byteString.end(); itr++)
    {
        auto character = *(itr);
    
        if (character == ' ' || itr == byteString.end() - 1)
        {
            // edge case handling but its bad
            if (itr == byteString.end() - 1)
            {
                temp += this->DEBUG_ascii_to_hex(character);
                temp = temp << 4;
            }

            byteArray.push_back(temp >> 4);
            temp = 0x0;
            continue;
        }
        temp += this->DEBUG_ascii_to_hex(character);
        temp = temp << 4;
    }

    int i = 0;
    for (Byte byte : byteArray)
    {
        this->setMemory(address + i, byte, eMemoryAccessType::debug);
        i++;
    }

    this->cpu.registers.pc = address;
}

void BUS::DEBUG_fill_ram(Word address, std::string byteString)
{
#include <iterator>
    std::vector<Byte> byteArray;

    Word temp = 0x0;


    for (auto itr = byteString.begin(); itr != byteString.end(); itr++)
    {
        auto character = *(itr);

        if (character == ' ' || itr == byteString.end() - 1)
        {
            // edge case handling but its bad
            if (itr == byteString.end() - 1)
            {
                temp += this->DEBUG_ascii_to_hex(character);
                temp = temp << 4;
            }

            byteArray.push_back(temp >> 4);
            temp = 0x0;
            continue;
        }
        temp += this->DEBUG_ascii_to_hex(character);
        temp = temp << 4;
    }

    int i = 0;
    for (Byte byte : byteArray)
    {
        this->setMemory(address + i, byte,eMemoryAccessType::debug);
        i++;
    }
}

void BUS::DEBUG_nintendo_logo()
{
    this->DEBUG_fill_ram(0x8000, "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    //this->DEBUG_fill_ram(0x8000, "FF FF 81 81 81 81 81 81 81 81 81 81 81 81 FF FF");
    this->DEBUG_fill_ram(0x8000, "FF FF C3 81 81 A5 99 81 99 81 81 A5 C3 81 FF FF");
    
    this->DEBUG_fill_ram(0x8010, "F0 00 F0 00 FC 00 FC 00 FC 00 FC 00 F3 00 F3 00");
    
    this->DEBUG_fill_ram(0x8020, "3C 00 3C 00 3C 00 3C 00 3C 00 3C 00 3C 00 3C 00");
    
    this->DEBUG_fill_ram(0x8030, "F0 00 F0 00 F0 00 F0 00 00 00 00 00 F3 00 F3 00");
    
    this->DEBUG_fill_ram(0x8040, "00 00 00 00 00 00 00 00 00 00 00 00 CF 00 CF 00");
    
    this->DEBUG_fill_ram(0x8050, "00 00 00 00 0F 00 0F 00 3F 00 3F 00 0F 00 0F 00");
    
    this->DEBUG_fill_ram(0x8060, "00 00 00 00 00 00 00 00 C0 00 C0 00 0F 00 0F 00");
    
    this->DEBUG_fill_ram(0x8070, "00 00 00 00 00 00 00 00 00 00 00 00 F0 00 F0 00");
    
    this->DEBUG_fill_ram(0x8080, "00 00 00 00 00 00 00 00 00 00 00 00 F3 00 F3 00");
    
    this->DEBUG_fill_ram(0x8090, "00 00 00 00 00 00 00 00 00 00 00 00 C0 00 C0 00");
    
    this->DEBUG_fill_ram(0x80A0, "03 00 03 00 03 00 03 00 03 00 03 00 FF 00 FF 00");
    
    this->DEBUG_fill_ram(0x80B0, "C0 00 C0 00 C0 00 C0 00 C0 00 C0 00 C3 00 C3 00");
    
    this->DEBUG_fill_ram(0x80C0, "00 00 00 00 00 00 00 00 00 00 00 00 FC 00 FC 00");
    
    this->DEBUG_fill_ram(0x80D0, "F3 00 F3 00 F0 00 F0 00 F0 00 F0 00 F0 00 F0 00");
    
    this->DEBUG_fill_ram(0x80E0, "3C 00 3C 00 FC 00 FC 00 FC 00 FC 00 3C 00 3C 00");
    
    this->DEBUG_fill_ram(0x80F0, "F3 00 F3 00 F3 00 F3 00 F3 00 F3 00 F3 00 F3 00");
    
    this->DEBUG_fill_ram(0x8100, "F3 00 F3 00 C3 00 C3 00 C3 00 C3 00 C3 00 C3 00");
    
    this->DEBUG_fill_ram(0x8110, "CF 00 CF 00 CF 00 CF 00 CF 00 CF 00 CF 00 CF 00");
    
    this->DEBUG_fill_ram(0x8120, "3C 00 3C 00 3F 00 3F 00 3C 00 3C 00 0F 00 0F 00");
    
    this->DEBUG_fill_ram(0x8130, "3C 00 3C 00 FC 00 FC 00 00 00 00 00 FC 00 FC 00");
    
    this->DEBUG_fill_ram(0x8140, "FC 00 FC 00 F0 00 F0 00 F0 00 F0 00 F0 00 F0 00");
    
    this->DEBUG_fill_ram(0x8150, "F3 00 F3 00 F3 00 F3 00 F3 00 F3 00 F0 00 F0 00");
    
    this->DEBUG_fill_ram(0x8160, "C3 00 C3 00 C3 00 C3 00 C3 00 C3 00 FF 00 FF 00");
    
    this->DEBUG_fill_ram(0x8170, "CF 00 CF 00 CF 00 CF 00 CF 00 CF 00 C3 00 C3 00");
    
    this->DEBUG_fill_ram(0x8180, "0F 00 0F 00 0F 00 0F 00 0F 00 0F 00 FC 00 FC 00");
    
    this->DEBUG_fill_ram(0x8190, "3C 00 42 00 B9 00 A5 00 B9 00 A5 00 42 00 3C 00");

    this->DEBUG_fill_ram(0x9900, "00 00 00 00 01 02 03 04 05 06 07 08 09 0A 0B 0C");
    this->DEBUG_fill_ram(0x9910, "19 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    this->DEBUG_fill_ram(0x9920, "00 00 00 00 0D 0E 0F 10 11 12 13 14 15 16 17 18");

    this->setMemory(LY, 0, eMemoryAccessType::debug);
    this->setMemory(SCY, 0x40, eMemoryAccessType::debug);
    this->setMemory(SCX, 0x20, eMemoryAccessType::debug);
    
    //this->setMemory(SCY, 0x0);
    //this->setMemory(SCX, 0x0);


    this->setMemory(LCDC, 0x91, eMemoryAccessType::debug);
    this->setMemory(STAT, 0b00000011, eMemoryAccessType::debug);
    this->setMemory(0xFF47, 0xE4, eMemoryAccessType::debug);



}

BUS::BUS(const std::string rom_path, const std::string bios_path)
{
    this->cpu.connectToBus(this);
    this->ppu.connectToBus(this);
    this->dma_controller.connectToBus(this);
    this->init();

    this->gamepak = GamePak(rom_path);
    this->loadBios(bios_path);

    //fill framebuffer
    FramebufferPixel blank(GB_PALLETE_00_r, GB_PALLETE_00_g, GB_PALLETE_00_b);
    for (int i = 0; i < XRES * YRES; i++)
        this->framebuffer[i] = blank;
}



void BUS::depressButton(const enum eJoypadButtons button)
{
    this->setJoypadState(button, 1);
}


void BUS::pressButton(const enum eJoypadButtons button)
{
    this->setJoypadState(button, 0);
    this->cpu.requestInterrupt(joypad);
}


Byte BUS::getActionButtonNibble()
{
    return this->joypad_state >> 4;
}

Byte BUS::getDirectionButtonNibble()
{
    return this->joypad_state & 0xF;
}


// The gameboy is a memory mapped system, components are mapped
// to indiviudial parts of the addressable memory, we can specify
// what hardware we are reading from using if guards
Byte BUS::getMemory(const Word address, enum eMemoryAccessType access_type)
{
    switch (access_type)
    {
        /*
        case MEMORY_ACCESS_TYPE::cpu:
        {
            if (this->dma_controller.dma_triggered)
                if (!(0xFF80 >= address && address <= 0xFFFE))
                    return 0x0; //read blocked for non HRAM addresses

            if  (0xFE00 <= address && address <= 0xFE9F) // OAM
                if ((this->ppu.lcdEnabled() && (this->ppu.getPPUState() == 0x3 || this->ppu.getPPUState() == 0x2)))
                    return 0xFF;

            if  (0x8000 <= address && address <= 0x9FFF) // VIDEO RAM
                if ((this->ppu.lcdEnabled() && this->ppu.getPPUState() == 0x3))
                    return 0xFF;

        } break;
        */
     /*   case MEMORY_ACCESS_TYPE::interrupt_handler:
        {
            if (address == 0xFFFF)
                return this->interrupt_enable_register;
        } break;
        default: break;*/
    }

    // boot rom area, or rom bank 0
    if (address <= 0x00FF) //from 0x0000
    {
        // if the bios has never been loaded or if the register at 0xFF50 is set 1 (which is done by the bios program) we need to access the cartridge bank
        if (this->io[(0xFF50) - IOOFFSET] == 0x1|| !bios_loaded)
            return gamepak.getMemory(address);
        
        
        return this->bios[address];
    }

    if (address <= 0x3FFF) //from 0x0100
    {

        // game rom bank 0
        return this->gamepak.getMemory(address);
    }

    if (address <= 0x7FFF) // from 0x4000
    {
        // game rom bank N

        //banking is not implemented but we will just now read the whole cart
        return this->gamepak.getMemory(address);
  //      return 0b0;
    }

    if (address <= 0x97FF) // from 0x8000         
    {       
        return this->video_ram[address - VIDEORAMOFFSET];
    }

    if (address <= 0x9FFF) // from 0x9800
    {
        // background map region
        return this->video_ram[address - VIDEORAMOFFSET];
    }

    if (address <= 0xBFFF) // from 0xA000
    {
        return this->gamepak.getMemory(address);
    }

    if (address <= 0xDFFF) // from 0xC000
    {
        // working ram
        return this->work_ram[address - 0xC000];
    }

    if (address <= 0xFDFF) // from 0xE000
    {
        // echo ram, it is a copy of the ram above, we will just read memory 2000 addresses above instead
        return this->getMemory(address - 0x2000,access_type);
    }

    if (address <= 0xFE9F) // from 0xFE00
    {
        // object attribute memory
        //if ppu in pixel transfer mode
        
        return this->oam_ram[address - OAMOFFSET];
    }

    if (address <= 0xFEFF) // from 0xFEA0
    {
        // unused part of the map, must retun 0
        return 0;
    }
    if (address <= 0xFF4B) // from 0xFF00
    {
        switch (address)
        {
            case 0xFF00:
            {
                Byte requestedJOYP = this->io[0];
                if (requestedJOYP & 0x10) {
                    return 0xD0 | this->getActionButtonNibble();
                }
                if (requestedJOYP & 0x20) {
                    return 0xE0 | this->getDirectionButtonNibble();
                }
                 fprintf(stderr, "cannot return input");  exit(-1);
            }
            //case 0xFF26:// NR52
            //{
            //    break;
            //}
            default: 
                return this->io[address - IOOFFSET];

        }
    }
    if (address <= 0xFF7F) // from 0xFF4C
    {
        // unused part of the map, just return
        return 0b0;
    }
    if (address <= 0xFFFF) // from 0xFF80
    {
        // high ram area
        return this->high_ram[address - HIGHRAMOFFSET];
    }
    
    // temp return
    fprintf(stderr, "getMemory no return");  exit(-1);
};

void BUS::setMemory(const Word address, const Byte data, enum eMemoryAccessType access_type)
{
     /*
    switch (access_type)
    {
    case MEMORY_ACCESS_TYPE::cpu:
    {
        if (this->dma_controller.dma_triggered)
            if (!(0xFF80 >= address && address <= 0xFFFE))
                return; //read blocked for non HRAM addresses

        if (0xFE00 <= address && address <= 0xFE9F) // OAM
            if ((this->ppu.lcdEnabled() && (this->ppu.getPPUState() == 0x3 || this->ppu.getPPUState() == 0x2)))
                return;

        //this breaks the tests
        
        //if (0x8000 <= address && address <= 0x9FFF) // VIDEO RAM
        //    if ((this->ppu.lcdEnabled() && this->ppu.getPPUState() == 0x3))
        //        return;
         
    } break;

    default: break;
    }
    */

    //Set to non-zero to disable boot ROM
    if (address == 0xFF50)
    {
        this->io[0xFF50 - IOOFFSET] = 0x1;
        return;
    }

    if (address <= 0x00ff)
    {
        // boot rom area, or rom bank 0
        if (this->io[(0xFF50) - IOOFFSET] == 0x1 || !this->bios_loaded)
            this->gamepak.setMemory(address,data);
        return;
    }

    if (address <= 0x3fff)
    {
        this->gamepak.setMemory(address, data);
        return;
    }

    if (address <= 0x7fff)
    {
        this->gamepak.setMemory(address, data);
        return;
    }

    if (address <= 0x97ff)
    {
       // Tile Ram region
        this->video_ram[address - VIDEORAMOFFSET] = data;
        return;
    }

    if (address <= 0x9FFF)
    {
        // background map region
        this->video_ram[address - VIDEORAMOFFSET] = data;
        return;
    }

    if (address <= 0xBFFF)
    {
        return this->gamepak.setMemory(address,data);
    }

    if (address <= 0xDFFF)
    {
        // working ram
        /*if (address == 0xC242) {
            auto workramm = this->work_ram.get() + 0x0242;
            workramm = workramm;
        }*/
        this->work_ram[address - 0xC000] = data;
        
        return;
    }

    if (address <= 0xFDFF)
    {
        // echo ram, it is a copy of the ram above, any calls to this address space will be redirected to address - 2000 spaces above
        return this->setMemory(address - 0x2000, data, access_type);
    }

    if (address <= 0xFE9F) // from 0xFE00
    {
       this->oam_ram[address - OAMOFFSET] = data;
       return;
    }

    if (address <= 0xFEFF)
    {
        // unused part of the map, just return
        return;
    }
    if (address <= 0xFF4B) // from ff00
    {
        switch (address)
        {
        case 0xFF00:
            this->io[address - 0xFF00] = data & 0x30; break;              
        case 0xFF02:
            this->io[address - 0xFF00] = (data | 0x7E);
            break;
        case 0xFF04:
            this->io[address - 0xFF00] = 0;
            break;
        case 0xFF07:
            this->io[address - 0xFF00] = (data | 0xF8);
            //this->cpu.update_timerCounter();
            break;
        case 0xFF0F:
            this->io[address - 0xFF00] = (data | 0xE0);
            break;
        case 0xFF10:
            this->io[address - 0xFF00] = (data | 0x80);
            break;
        case 0xFF14:
            this->io[address - 0xFF00] = (data | 0xB8);
            break;
        case 0xFF15:
            this->io[address - 0xFF00] = 0xFF;
            break;
        case 0xFF19:
            this->io[address - 0xFF00] = (data | 0xB8);
            break;
        case 0xFF1A:
            this->io[address - 0xFF00] = (data | 0x7F);
            break;
        case 0xFF1C:
            this->io[address - 0xFF00] = (data | 0x9F);
            break;
        case 0xFF1E:
            this->io[address - 0xFF00] = (data | 0xB8);
            break;
        case 0xFF1F:
            this->io[address - 0xFF00] = 0xFF;
            break;
        case 0xFF20:
            this->io[address - 0xFF00] = (data | 0xC0);
            break;
        case 0xFF23:
            this->io[address - 0xFF00] = (data | 0xBF);
            break;
        case 0xFF26:
            this->io[address - 0xFF00] = (data | 0x70);
            if ((this->io[0xFF26 - IOOFFSET] & 0x8F) == 0)
            {
                //destroy sound registers.
            }
            break;
        case 0xFF41:
            this->io[address - 0xFF00] = (data | 0x80);
            break;
        case 0xFF44: // LY
            break;
        case 0xFF46: // dma
           this->dma_controller.requestDMA(data);
           break;
        case 0xFF50:
            this->io[0xFF50 - IOOFFSET] = 0x1; // disable bootrom
            break;


        default:
            this->io[address - 0xFF00] = data;
        }
        return;
    }
    if (address <= 0xFF7F)
    {
        // unused part of the map, just return
        return;
    }
    if (address <= 0xFFFF)
    {
        // high ram area
        this->high_ram[address - HIGHRAMOFFSET] = data;
        return;
    }

    fprintf(stderr, "set memory fail");  exit(-1);
}

/// <summary>
/// A function to load the bios into memory, as the bios is property of Nintendo, we can not legally release this emulator with the bios,
/// however we can allow users to supply their own bios file, named bios.bin, this function will set the program counter of the CPU to execute the bios program if it exists.  
/// </summary>
/// <param name="bios_name"></param>
void BUS::loadBios(const std::string bios_name)
{
    std::ifstream file(bios_name, std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        std::ifstream::pos_type pos = file.tellg();

        file.seekg(0, std::ios::beg);
        file.read((char*)this->bios.get(), pos);

        //set the program counter of the cpu to execute the bios program
        this->cpu.biosInit();
        this->biosInit();

        this->bios_loaded = true;
    }
    file.close();

};

// initializes the hardware registers on startup
void BUS::init()
{
    this->io[0xFF00 - IOOFFSET] = 0xCF; // P1
    this->io[0xFF01 - IOOFFSET] = 0x00; // SB
    this->io[0xFF02 - IOOFFSET] = 0x7E; // SC
    this->io[0xFF04 - IOOFFSET] = 0xAB; // DIV
    this->io[0xFF05 - IOOFFSET] = 0x00; // TIMA
    this->io[0xFF06 - IOOFFSET] = 0x00; // TMA
    this->io[0xFF07 - IOOFFSET] = 0xF8; // TAC
    this->io[0xFF0F - IOOFFSET] = 0xE1; // IF
    this->io[0xFF10 - IOOFFSET] = 0x80; // NR10
    this->io[0xFF11 - IOOFFSET] = 0xBF; // NR11
    this->io[0xFF12 - IOOFFSET] = 0xF3; // NR12
    this->io[0xFF13 - IOOFFSET] = 0xFF; // NR13
    this->io[0xFF14 - IOOFFSET] = 0xBF; // NR14
    //this->io[0xFF15 - IOOFFSET] = 0xFF; // NR14
    this->io[0xFF16 - IOOFFSET] = 0x3F; // NR21
    this->io[0xFF17 - IOOFFSET] = 0x00; // NR22
    this->io[0xFF18 - IOOFFSET] = 0xFF; // NR23
    this->io[0xFF19 - IOOFFSET] = 0xBF; // NR24
    this->io[0xFF1A - IOOFFSET] = 0x7F; // NR30
    this->io[0xFF1B - IOOFFSET] = 0xFF; // NR31
    this->io[0xFF1C - IOOFFSET] = 0x9F; // NR32
    this->io[0xFF1D - IOOFFSET] = 0xFF; // MR33
    this->io[0xFF1E - IOOFFSET] = 0xBF; // MR34
    //this->io[0xFF1F - IOOFFSET] = 0xFF; // MR34
    this->io[0xFF20 - IOOFFSET] = 0xFF; // NR41
    this->io[0xFF21 - IOOFFSET] = 0x00; // NR42
    this->io[0xFF22 - IOOFFSET] = 0x00; // NR44
    this->io[0xFF23 - IOOFFSET] = 0xBF; // NR50
    this->io[0xFF24 - IOOFFSET] = 0x77; // NR51
    this->io[0xFF25 - IOOFFSET] = 0xF3; // LCDC
    this->io[0xFF26 - IOOFFSET] = 0xF1; // STAT
    this->io[0xFF40 - IOOFFSET] = 0x91; // SCY
    this->io[0xFF41 - IOOFFSET] = 0x85; // SCX
    this->io[0xFF42 - IOOFFSET] = 0x00; // LY
    this->io[0xFF43 - IOOFFSET] = 0x00; // LYC
    this->io[0xFF44 - IOOFFSET] = 0x00; // DMA
    this->io[0xFF45 - IOOFFSET] = 0x00; // BGP
    this->io[0xFF46 - IOOFFSET] = 0xFF; // 0BP0
    this->io[0xFF47 - IOOFFSET] = 0xFC; // 0BP1
    this->io[0xFF48 - IOOFFSET] = 0xFF; // WY
    this->io[0xFF49 - IOOFFSET] = 0xFF; // WX
    this->io[0xFF4A - IOOFFSET] = 0x00; // KEY1
    this->io[0xFF4B - IOOFFSET] = 0x00; // VBK
    this->io[0xFF4D - IOOFFSET] = 0xFF; // HDMA1
    this->io[0xFF4F - IOOFFSET] = 0xFF; // HDMA2
    this->io[0xFF51 - IOOFFSET] = 0xFF; // HDMA3
    this->io[0xFF52 - IOOFFSET] = 0xFF; // HDMA4
    this->io[0xFF53 - IOOFFSET] = 0xFF; // HDMA5
    this->io[0xFF54 - IOOFFSET] = 0xFF; // HDMA5
    this->io[0xFF55 - IOOFFSET] = 0xFF; // HDMA5
    this->io[0xFF56 - IOOFFSET] = 0xFF; // RP
    this->io[0xFF68 - IOOFFSET] = 0xFF; // BCPS
    this->io[0xFF69 - IOOFFSET] = 0xFF; // BCPD
    this->io[0xFF6A - IOOFFSET] = 0xFF; // OCPS
    this->io[0xFF6B - IOOFFSET] = 0xFF; // OCPD
    this->io[0xFF70 - IOOFFSET] = 0xFF; // SVBK

    this->interrupt_enable_register = 0;
}

void BUS::biosInit()
{
    this->io[0xFF00 - IOOFFSET ] = 0xCF; // P1
    this->io[0xFF01 - IOOFFSET ] = 0x00; // SB
    this->io[0xFF02 - IOOFFSET ] = 0x7E; // SC
    this->io[0xFF04 - IOOFFSET ] = 0x00; // DIV
    this->io[0xFF05 - IOOFFSET ] = 0x00; // TIMA
    this->io[0xFF06 - IOOFFSET ] = 0x00; // TMA
    this->io[0xFF07 - IOOFFSET ] = 0xF8; // TAC
    this->io[0xFF0F - IOOFFSET ] = 0xE1; // IF
    this->io[0xFF10 - IOOFFSET ] = 0x80; // NR10
    this->io[0xFF11 - IOOFFSET ] = 0x00; // NR11
    this->io[0xFF12 - IOOFFSET ] = 0x00; // NR12
    this->io[0xFF13 - IOOFFSET ] = 0x00; // NR13
    this->io[0xFF14 - IOOFFSET ] = 0xB8; // NR14
    this->io[0xFF15 - IOOFFSET ] = 0xFF; // NR14
    this->io[0xFF16 - IOOFFSET ] = 0x00; // NR21
    this->io[0xFF17 - IOOFFSET ] = 0x00; // NR22
    this->io[0xFF18 - IOOFFSET ] = 0x00; // NR23
    this->io[0xFF19 - IOOFFSET ] = 0xB8; // NR24
    this->io[0xFF1A - IOOFFSET ] = 0x7F; // NR30
    this->io[0xFF1B - IOOFFSET ] = 0x00; // NR31
    this->io[0xFF1C - IOOFFSET ] = 0x9F; // NR32
    this->io[0xFF1D - IOOFFSET ] = 0x00; // MR33
    this->io[0xFF1E - IOOFFSET ] = 0xB8; // MR34
    this->io[0xFF1F - IOOFFSET ] = 0xFF; // MR34
    this->io[0xFF20 - IOOFFSET ] = 0xC0; // NR41
    this->io[0xFF21 - IOOFFSET ] = 0x00; // NR42
    this->io[0xFF22 - IOOFFSET ] = 0x00; // NR44
    this->io[0xFF23 - IOOFFSET ] = 0xBF; // NR50
    this->io[0xFF24 - IOOFFSET ] = 0x00; // NR51
    this->io[0xFF25 - IOOFFSET ] = 0x00; // LCDC
    this->io[0xFF26 - IOOFFSET ] = 0x70; // STAT
    this->io[0xFF40 - IOOFFSET ] = 0x00; // SCY
    this->io[0xFF41 - IOOFFSET ] = 0x84; // SCX
    this->io[0xFF42 - IOOFFSET ] = 0x00; // LY
    this->io[0xFF43 - IOOFFSET ] = 0x00; // LYC
    this->io[0xFF44 - IOOFFSET ] = 0x00; // DMA
    this->io[0xFF45 - IOOFFSET ] = 0x00; // BGP
    this->io[0xFF46 - IOOFFSET ] = 0xFF; // 0BP0
    this->io[0xFF47 - IOOFFSET ] = 0xFC; // 0BP1
    this->io[0xFF48 - IOOFFSET ] = 0xFF; // WY
    this->io[0xFF49 - IOOFFSET ] = 0xFF; // WX
    this->io[0xFF4A - IOOFFSET ] = 0x00; // KEY1
    this->io[0xFF4B - IOOFFSET ] = 0x00; // VBK
    this->io[0xFF4D - IOOFFSET ] = 0xFF; // HDMA1
    this->io[0xFF4F - IOOFFSET ] = 0x0 ; // HDMA2
    this->io[0xFF51 - IOOFFSET ] = 0xFF; // HDMA3
    this->io[0xFF52 - IOOFFSET ] = 0xFF; // HDMA4
    this->io[0xFF53 - IOOFFSET ] = 0x9F; // HDMA5
    this->io[0xFF54 - IOOFFSET ] = 0xF0; // HDMA5
    this->io[0xFF55 - IOOFFSET ] = 0xFF; // HDMA5
    this->io[0xFF56 - IOOFFSET ] = 0xFF; // RP
    this->io[0xFF68 - IOOFFSET ] = 0xFF; // BCPS
    this->io[0xFF69 - IOOFFSET ] = 0xFF; // BCPD
    this->io[0xFF6A - IOOFFSET ] = 0xFF; // OCPS
    this->io[0xFF6B - IOOFFSET ] = 0xFF; // OCPD
    this->io[0xFF70 - IOOFFSET ] = 0x1 ; // SVBK

    this->interrupt_enable_register = 0;

}