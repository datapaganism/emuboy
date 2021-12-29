#include "bus.hpp"
#include <iterator>

#include "vram_renderer.hpp"
#include "bg_map_renderer.hpp"

void BUS::cycle_system_one_frame()
{
    int currentCycles = 0;

    while (currentCycles <= CYCLES_PER_FRAME)
    {

            int cyclesUsed = this->cpu.fetch_decode_execute();

            this->cpu.update_timers(cyclesUsed);

            cyclesUsed += this->cpu.do_interrupts();

            this->dma_controller.update_dma(cyclesUsed);

            this->ppu.update_graphics(cyclesUsed);

            currentCycles += cyclesUsed;

            // Serial monitoring
            if ((this->io[0xFF02 - IOOFFSET] & ~0x7E) == 0x81)
            {
                char c = this->io[0xFF01 - IOOFFSET];
                printf("%c", c);
                this->io[0xFF02 - IOOFFSET] = 0x0;
            }
    }
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
int BUS::DEBUG_opcode_program(Word address, std::string byteString, int cycles)
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
        this->set_memory(address + i, byte, MEMORY_ACCESS_TYPE::debug);
        i++;
    }

    this->cpu.registers.pc = address;

    int cyclesUsed = 0;
    for (int i = 0; i < cycles; i++)
    {
        cyclesUsed = this->cpu.fetch_decode_execute();
    }
    return cyclesUsed;
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
        this->set_memory(address + i, byte,MEMORY_ACCESS_TYPE::debug);
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

    this->set_memory(LY, 0, MEMORY_ACCESS_TYPE::debug);
    this->set_memory(SCY, 0x40, MEMORY_ACCESS_TYPE::debug);
    this->set_memory(SCX, 0x20, MEMORY_ACCESS_TYPE::debug);
    
    //this->set_memory(SCY, 0x0);
    //this->set_memory(SCX, 0x0);


    this->set_memory(LCDC, 0x91, MEMORY_ACCESS_TYPE::debug);
    this->set_memory(STAT, 0b00000011, MEMORY_ACCESS_TYPE::debug);
    this->set_memory(0xFF47, 0xE4, MEMORY_ACCESS_TYPE::debug);



}

BUS::BUS()
{
    this->cpu.connect_to_bus(this);
    this->ppu.connect_to_bus(this);
    this->dma_controller.connect_to_bus(this);
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
    case bA: { this->io[0xFF00 - IOOFFSET] |= (0b1 << 0) ; } break;
    
    case dLeft:
    case bB: { this->io[0xFF00 - IOOFFSET] |= (0b1 << 1) ; } break;
    
    case dUp:
    case bSelect: { this->io[0xFF00 - IOOFFSET] |= (0b1 << 2) ; } break;
    
    case dDown:
    case bStart: { this->io[0xFF00 - IOOFFSET] |= (0b1 << 3) ; } break;
    default: throw "Unreachable button press"; break;
    }
}

void BUS::pressButton(const enum JoypadButtons button)
{
    switch (button)
    {
    case dRight:
    case bA: { this->io[0xFF00 - IOOFFSET] &= ~(0b1 << 0) ; } break;
    
    case dLeft:
    case bB: { this->io[0xFF00 - IOOFFSET] &= ~(0b1 << 1) ; } break;
    
    case dUp:
    case bSelect: { this->io[0xFF00 - IOOFFSET] &= ~(0b1 << 2) ; } break;
    
    case dDown:
    case bStart: { this->io[0xFF00 - IOOFFSET] &= ~(0b1 << 3) ; } break;
    default: throw "Unreachable button press"; break;
    }

    this->cpu.request_interrupt(joypad);
}


// The gameboy is a memory mapped system, components are mapped
// to indiviudial parts of the addressable memory, we can specify
// what hardware we are reading from using if guards
Byte BUS::get_memory(const Word address, enum MEMORY_ACCESS_TYPE access_type)
{
    switch (access_type)
    {
        case MEMORY_ACCESS_TYPE::cpu:
        {
            if (this->dma_controller.dma_triggered)
                if (!(0xFF80 >= address && address <= 0xFFFE))
                    return 0x0; //read blocked for non HRAM addresses

            if  (0xFE00 <= address && address <= 0xFE9F) // OAM
                if ((this->ppu.lcd_enabled() && (this->ppu.get_ppu_state() == 0x3 || this->ppu.get_ppu_state() == 0x2)))
                    return 0xFF;

            if  (0x8000 <= address && address <= 0x9FFF) // VIDEO RAM
                if ((this->ppu.lcd_enabled() && this->ppu.get_ppu_state() == 0x3))
                    return 0xFF;

        } break;

        case MEMORY_ACCESS_TYPE::interrupt_handler:
        {
            if (address == 0xFFFF)
                return this->interrupt_enable_register;
        } break;
        default: break;
    }

    // boot rom area, or rom bank 0
    if (address <= 0x00FF) //from 0x0000
    {
        // if the bios has never been loaded or if the register at 0xFF50 is set 1 (which is done by the bios program) we need to access the cartridge bank
        if (this->io[(0xFF50) - IOOFFSET] == 0x1|| !this->biosLoaded)
            return this->gamepak.rom[address];
        
        return this->bios[address];
    }

    if (address <= 0x3FFF) //from 0x0100
    {

        // game rom bank 0
        return this->gamepak.rom[address];
    }

    if (address <= 0x7FFF) // from 0x4000
    {
        // game rom bank N

        //banking is not implemented but we will just now read the whole cart
        return this->gamepak.rom[address];
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
        // cartridge ram
        return 0b0;
    }

    if (address <= 0xDFFF) // from 0xC000
    {
        // working ram
        return this->work_ram[address - 0xC000];
    }

    if (address <= 0xFDFF) // from 0xE000
    {
        // echo ram, it is a copy of the ram above, we will just read memory 2000 addresses above instead
        return this->get_memory(address - 0x2000,access_type);
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
        // i/o registers

        if (address == 0xFF26) // NR52
        {

        }

        return this->io[address - 0xFF00];
    }
    if (address <= 0xFF7F) // from 0xFF4C
    {
        // unused part of the map, just return
        return 0b0;
    }
    if (address <= 0xFFFE) // from 0xFF80
    {
        // high ram area
        return this->high_ram[address-0xFF80];
    }
    if (address <= 0xFFFF) // from 0xFFFF, yep
    {
        // interrupt enabled register, cannot be accessed
        //return this->interrupt_enable_register;
        return 0b0;
    }

    // temp return
    return 0;
};

void BUS::set_memory(const Word address, const Byte data, enum MEMORY_ACCESS_TYPE access_type)
{
   /* if (0x9800 <= address && 0x9BFF >= address && this->DEBUG_PC_breakpoint_hit) {
        this->DEBUG_fill_ram(0x8200, "3C 00 42 00 B9 00 A5 00 B9 00 A5 00 42 00 3C 00");
        this->vram_ptr->render_vram_tiles(this);
        this->bg_map_ptr->render_vram_tiles(this);
    }*/
    switch (access_type)
    {
    case MEMORY_ACCESS_TYPE::cpu:
    {
        if (this->dma_controller.dma_triggered)
            if (!(0xFF80 >= address && address <= 0xFFFE))
                return; //read blocked for non HRAM addresses

        if (0xFE00 <= address && address <= 0xFE9F) // OAM
            if ((this->ppu.lcd_enabled() && (this->ppu.get_ppu_state() == 0x3 || this->ppu.get_ppu_state() == 0x2)))
                return;

        //this breaks the tests
        if (0x8000 <= address && address <= 0x9FFF) // VIDEO RAM
            if ((this->ppu.lcd_enabled() && this->ppu.get_ppu_state() == 0x3))
                return;

    } break;

    default: break;
    }

    if (address == 0xFF50)
    {
        this->io[0xFF50 - IOOFFSET] = 0x1;
        return;
    }

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
        // cartridge ram
        return;
    }

    if (address <= 0xDFFF)
    {
        // working ram
        if (address == 0xC242) {
            auto workramm = this->work_ram.get() + 0x0242;
            workramm = workramm;
        }
        this->work_ram[address - 0xC000] = data;
        
        return;
    }

    if (address <= 0xFDFF)
    {
        // echo ram, it is a copy of the ram above, any calls to this address space will be redirected to address - 2000 spaces above
        return this->set_memory(address - 0x2000, data, access_type);
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
            this->io[address - 0xFF00] = (data | 0xCF);
            break;
        case 0xFF02:
            this->io[address - 0xFF00] = (data | 0x7E);
            break;
        case 0xFF04:
            this->io[address - 0xFF00] = 0;
            break;
        case 0xFF07:
            this->io[address - 0xFF00] = (data | 0xF8);
            this->cpu.update_timerCounter();
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
           this->dma_controller.request_dma(data);
           break;
        case 0xFF50:
            this->io[0xFF50 - IOOFFSET] = 0x1; // disable bootrom
            break;


        default:
            this->io[address - 0xFF00] = data;
        }
        return;

        //if (address == 0xFF0F)
        //{
        //    this->io[address - 0xFF00] = (data | 0xE0);
        //    return;
        //}
        //if (address == STAT)
        //{
        //    this->io[address - 0xFF00] = (this->io[address - 0xFF00] & 0x80) | (data & 0x7F);

        //    return;
        //}

        ////if changing timers
        //if (address == TAC)
        //{
        //    this->io[address - 0xFF00] = data;
        //    this->cpu.update_timerCounter();
        //    return;
        //}
        //if (address == DIV)
        //{
        //    this->io[address - 0xFF00] = 0;
        //    return;
        //}

        //// i/o registers

        //if (address == DMA)
        //{
        //    this->dma_controller.request_dma(data);
        //    return;
        //}


        //// LY register gets reset if written to
        //if (address == LY)
        //{
        //    //this->io[LY - IOOFFSET] = 0;
        //    return;
        //}

        //if (address == 0xFF26) // NR52
        //{
        //    this->io[0xFF26 - IOOFFSET] |= (data & 0x80);

        //    if ((this->io[0xFF26 - IOOFFSET] & 0x80) == 0)
        //    {
        //        //destroy sound registers.
        //    }

        //    return;
        //}

        //if (address == 0xFF50)
        //{
        //    this->io[0xFF50 - IOOFFSET] = 0x1;
        //    return;
        //}

        //this->io[address - 0xFF00] = data;
        //return;
    }
    if (address <= 0xFF7F)
    {
        // unused part of the map, just return
        return;
    }
    if (address <= 0xFFFE)
    {
        // high ram area
        this->high_ram[address - 0xFF80] = data;
        return;
    }
    if (address <= 0xFFFF)
    {
        // interrupt enabled register, write only data
        this->interrupt_enable_register = data;

        return;
    }
}
void BUS::set_memory_word(const Word address, const Word data, enum MEMORY_ACCESS_TYPE access_type)
{
    //store in little endian byte order 
    this->set_memory(address, (data & 0x00ff),access_type);
    this->set_memory(address + 1, ((data & 0xff00) >> 8), access_type);
}

const Word BUS::get_memory_word_lsbf(const Word address, enum MEMORY_ACCESS_TYPE access_type)
{
        return (this->get_memory(address, access_type) | (this->get_memory(address + 1, access_type) << 8));
}

Byte BUS::DEBUG_get_memory(const Word address)
{
    return this->get_memory(address, MEMORY_ACCESS_TYPE::debug);
}

void BUS::DEBUG_set_memory(const Word address, const Byte data)
{
    this->set_memory(address, data, MEMORY_ACCESS_TYPE::debug);   
}

void BUS::DEBUG_set_memory_word(const Word address, const Word data)
{
    this->set_memory_word(address, data, MEMORY_ACCESS_TYPE::debug);
}

const Word BUS::DEBUG_get_memory_word_lsbf(const Word address)
{
    return this->get_memory_word_lsbf(address, MEMORY_ACCESS_TYPE::debug);
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
        file.read((char*)this->bios.get(), pos);

        //set the program counter of the cpu to execute the bios program
        this->cpu.bios_init();
        this->bios_init();

        this->biosLoaded = true;
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

void BUS::bios_init()
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