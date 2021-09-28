#include "ppu.hpp"
#include "bus.hpp"

#include <bitset>
void PPU::connect_to_bus(BUS* pBus)
{
	this->bus = pBus;
}

void PPU::update_graphics(const int cycles)
{
	// one frame takes 70221 cycles
	// one scanline is 456 cycles 
	
	// the cpu may have modified the registers since the last cycle, it is time to check and update any changes of the lcd stat.
	this->update_lcdstat();

	if (this->lcd_enabled())
	{
		this->cycle_counter += cycles;

		if (this->cycle_counter >= 456)
		{
			this->bus->io[0xFF44 - IOOFFSET]++;
			this->cycle_counter = 0;

			if (this->bus->io[0xFF44 - IOOFFSET] == 144)
			{
				this->bus->cpu.request_interrupt(vblank);
				return;
			}

			if (this->bus->io[0xFF44 - IOOFFSET] > 153)
			{
				this->bus->io[0xFF44 - IOOFFSET] = 0;
				return;
			}

			// emulate ppu of that scanline
		}
		return;
	}
}

void PPU::update_lcdstat()
{

	/*
		LCDSTAT

		00: H-Blank
		01: V-Blank
		10: Searching Sprites Atts
		11: Transfering Data to LCD Driver 
	
		Bit 3: Mode 0 Interupt Enabled
		Bit 4: Mode 1 Interupt Enabled
		Bit 5: Mode 2 Interupt Enabled
	*/
	Byte* lcdstat_register_ptr = &this->bus->io[STAT - IOOFFSET];
	
	Byte* ly_ptr = &this->bus->io[LY - IOOFFSET];

	Byte original_mode = (*lcdstat_register_ptr & 0b00000011);
	bool lyc_ly_flag   = (*lcdstat_register_ptr & 0b00000100);
	bool mode0_irq     = (*lcdstat_register_ptr & 0b00001000);
	bool mode1_irq     = (*lcdstat_register_ptr & 0b00010000);
	bool mode2_irq     = (*lcdstat_register_ptr & 0b00100000);
	bool lyc_ly_irq    = (*lcdstat_register_ptr & 0b01000000);
	

	bool irq_needed = false;

	if (!this->lcd_enabled())
	{
		// reset to the start of the drawing routine, set ly back to 0
		this->cycle_counter = 0;
		*ly_ptr = 0;

		//update register to mode one
		*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000001);

		return;
	}
	
	// if in vblank
	if (*ly_ptr >= 144)
	{
		*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000001);
		irq_needed = mode1_irq;
	}
	// if not in vblank where are in h
	else
	{
		// OAM scan, mode 2
		if (this->cycle_counter <= 80)
		{
			*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000010);
			irq_needed = mode2_irq;
		}
		// pixel transfer, mode 3
		else if (this->cycle_counter <= 172)
		{
			*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000011);
		}
		// h blank, mode 0
		else
		{
			*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000000);
			irq_needed = mode0_irq;
		}


	}

	if (original_mode != (*lcdstat_register_ptr & 0b00000011) && irq_needed)
		this->bus->cpu.request_interrupt(lcdstat);
	
	//time to check LYC = LY

	//set register for equality
	if (*ly_ptr == this->bus->io[LYC - IOOFFSET])
	{
		*lcdstat_register_ptr |= 0b00000100;
		// if interrupt is enabled
		if (lyc_ly_irq)
			this->bus->cpu.request_interrupt(lcdstat);
	}
	else
		*lcdstat_register_ptr &= ~0b00000100;
	
}

bool PPU::lcd_enabled()
{
	Byte temp = this->bus->io[LCDC - IOOFFSET];
	return (this->bus->io[LCDC - IOOFFSET] >> 7);
}

void TILE::consolePrint()
{    
	for (int y = 0; y < 8; y++)
	{
	    for (int x = 0; x < 8; x++)
	    {
	        this->getPixelColour(x, y);
	        std::cout << " ";
	    }
	    std::cout << "\n";
	}
}

void TILE::getPixelColour(int x, int y)
{
	int offset = (0b1 << (7 - x));

	bool bit0 = this->bytes_per_tile[2 * y] & offset;
	bool bit1 = this->bytes_per_tile[(2 * y) + 1] & offset;

	Byte result = (((Byte)bit0 << 1) | (Byte)bit1);

	if (result != 0)
		std::cout<< "";
	std::cout << std::bitset<2>{result};

	//return result;
}

TILE::TILE(BUS* bus, Word address)
{
	for (int i = 0; i < 16; i++)
	{
		this->bytes_per_tile[i] = bus->get_memory(address + i);
	}	
}

TILE::TILE()
{
	this->bytes_per_tile.fill(0x00);
}

PPU::PPU()
{
	/*for (auto& pixel : this->framebuffer)
	{
		pixel = FRAMEBUFFER_PIXEL(0xFF, 0xFF, 0xFF);
	}*/

	
	//int x = 3, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);
	//x = 4, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 6, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);
	//x = 7, y = 2;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 2, y = 3;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 5, y = 3;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 8, y = 3;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 2, y = 4;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 8, y = 4;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 3, y = 5;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 7, y = 5;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 4, y = 6;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 6, y = 6;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);

	//x = 5, y = 7;
	//this->framebuffer[x + (XRES * y)] = FRAMEBUFFER_PIXEL(0x32, 0x32, 0xFF);
};



void FIFO::push(FIFO_pixel pixel)
{
	if (tail_pos <= 16)
	{
		this->queue[tail_pos++] = pixel;
	}
}

FIFO_pixel FIFO::pop()
{
	if (tail_pos > 0)
	{
		FIFO_pixel temp = this->queue[0];
		for (int i = 0; i < this->tail_pos - 1; i++)
		{
			this->queue[i] = this->queue[i + 1];
		}
		memset(&this->queue[this->tail_pos - 1], 0, sizeof(FIFO_pixel));
		this->tail_pos--;

		return temp;
	}
}

FIFO::FIFO()
{
}
