#include "ppu.hpp"
#include "bus.hpp"
#include <bitset>

// ppu pushes for 4 pixels per NOP
// https://www.reddit.com/r/EmuDev/comments/8uahbc/dmg_bgb_lcd_timings_and_cnt/e1iooum/


PPU::PPU()
{
	this->fifo_bg.connectToPPU(this);
	this->fifo_bg.fetcher.connectToPPU(this);
	this->fifo_sprite.connectToPPU(this);
	this->fifo_sprite.fetcher.connectToPPU(this);

}

// called during BUS consturctor 
void PPU::connectToBus(BUS* pBus)
{
	this->bus = pBus;
	this->setRegisters();
}

Byte PPU::getMemory(const Word address)
{
	return this->bus->getMemory(address, eMemoryAccessType::ppu);
}

void PPU::setMemory(const Word address, const Byte data)
{
	this->bus->setMemory(address, data, eMemoryAccessType::ppu);
}

void PPU::setRegisters()
{
	this->registers.ly = &this->bus->io[LY - IOOFFSET];
	this->registers.lyc = &this->bus->io[LYC - IOOFFSET];
	this->registers.wx = &this->bus->io[WX - IOOFFSET];
	this->registers.wy = &this->bus->io[WY - IOOFFSET];
	this->registers.scx = &this->bus->io[SCX - IOOFFSET];
	this->registers.scy = &this->bus->io[SCY - IOOFFSET];
	this->registers.stat = &this->bus->io[STAT - IOOFFSET];
	this->registers.lcdc = &this->bus->io[LCDC - IOOFFSET];
}

void PPU::updateGraphics(const int cycles)
{
	// one frame takes 70221 cycles
	// one scanline is 456 cycles 

	// the cpu may have modified the registers since the last cycle, it is time to check and update any changes of the lcd stat.
	//this->update_lcdstat();


		//this->bus->setMemory(SCX, 0x00);
		//this->bus->setMemory(SCY, 0x00);
		//this->bus->io[LY - IOOFFSET] = 0x00;
//		this->debug_register_set = false;

	/*this->bus->setMemory(0x8000, 0xC2);
	this->bus->setMemory(0x8001, 0x7F);
	this->bus->setMemory(0x8002, 0xBD);
	this->bus->setMemory(0x8003, 0xC3);
	this->bus->setMemory(0x8004, 0xDB);
	this->bus->setMemory(0x8005, 0x24);
	this->bus->setMemory(0x8006, 0xA5);
	this->bus->setMemory(0x8007, 0x5A);
	this->bus->setMemory(0x8008, 0xA5);
	this->bus->setMemory(0x8009, 0x5A);
	this->bus->setMemory(0x800A, 0xDB);
	this->bus->setMemory(0x800B, 0x24);
	this->bus->setMemory(0x800C, 0xBD);
	this->bus->setMemory(0x800D, 0xC3);
	this->bus->setMemory(0x800E, 0xFF);
	this->bus->setMemory(0x800F, 0x42);*/

	if (this->lcdEnabled())
	{

		this->cycle_counter += cycles;

		//switch (this->bus->io[STAT - IOOFFSET] & 0b00000011)
		switch (*registers.stat & 0b00000011)
		{
		case 0: // h blank
		{
			if (this->cycle_counter >= (456 / 4))
			{
				this->newScanline();
			}
		} break;

		case 1: // v blank
		{
			if (this->cycle_counter >= (456 / 4))
			{
				this->newScanline();
			}
		} break;

		case 2: // oam search
		{
			// do stuff
			if (*registers.wy == *registers.ly)
				this->window_wy_triggered = true;

			if (this->cycle_counter >= (80 / 4))
			{
				this->updateState(3);
			}
		} break;

		case 3: // graphics transfer
		{
			// update bg/win fetcher and fifo
			this->fifo_bg.fetcher.updateFetcher(cycles);
			this->fifo_bg.updateFIFO(cycles);

			if (this->scanline_x >= 160)
				this->updateState(0);

			//if (this->scanline_x >= 8)
				//this->newScanline();
		} break;

		default: fprintf(stderr, "Unreachable PPU STAT");  exit(-1); break;
		}

		return;
	}

	// reset to the start of the drawing routine, set ly back to 0
	this->cycle_counter = 0;
	*registers.ly = 0;

	//update register to mode one
	this->updateState(0);

	return;





	//if (this->lcdEnabled())
	//{
	//	this->cycle_counter += cycles;

	//	if (this->cycle_counter >= 80)
	//	{
	//		this->fifo_bg.fetcher.updateFetcher(cycles);
	//		this->fifo_bg.updateFIFO(cycles);
	//	}

	//	if (this->cycle_counter >= 456)
	//	{
	//		this->bus->io[0xFF44 - IOOFFSET]++;
	//		this->cycle_counter = 0;

	//		if (this->bus->io[0xFF44 - IOOFFSET] == 144)
	//		{
	//			this->bus->cpu.request_interrupt(vblank);
	//			return;
	//		}

	//		if (this->bus->io[0xFF44 - IOOFFSET] > 153)
	//		{
	//			this->bus->io[0xFF44 - IOOFFSET] = 0;
	//			return;
	//		}

	//		//this->render_scanline();
	//	}
	//	return;
	//}
}

//void PPU::update_lcdstat()
//{
//
//	/*
//		LCDSTAT
//
//		00: H-Blank
//		01: V-Blank
//		10: Searching Sprites Atts
//		11: Transfering Data to LCD Driver 
//	
//		Bit 3: Mode 0 Interupt Enabled
//		Bit 4: Mode 1 Interupt Enabled
//		Bit 5: Mode 2 Interupt Enabled
//	*/
//	Byte* lcdstat_register_ptr = &this->bus->io[STAT - IOOFFSET];
//	
//	Byte* ly_ptr = &this->bus->io[LY - IOOFFSET];
//
//	Byte original_mode = (*lcdstat_register_ptr & 0b00000011);
//	bool lyc_ly_flag   = (*lcdstat_register_ptr & 0b00000100);
//	bool mode0_irq     = (*lcdstat_register_ptr & 0b00001000);
//	bool mode1_irq     = (*lcdstat_register_ptr & 0b00010000);
//	bool mode2_irq     = (*lcdstat_register_ptr & 0b00100000);
//	bool lyc_ly_irq    = (*lcdstat_register_ptr & 0b01000000);
//	
//
//	bool irq_needed = false;
//
//	if (!this->lcdEnabled())
//	{
//		// reset to the start of the drawing routine, set ly back to 0
//		this->cycle_counter = 0;
//		*ly_ptr = 0;
//
//		//update register to mode one
//		*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000001);
//
//		return;
//	}
//	
//	// if in vblank
//	if (*ly_ptr >= 144)
//	{
//		*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000001);
//		irq_needed = mode1_irq;
//	}
//	// if not in vblank where are in h
//	else
//	{
//		// OAM scan, mode 2
//		if (this->cycle_counter <= 80)
//		{
//			*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000010);
//			irq_needed = mode2_irq;
//		}
//		// pixel transfer, mode 3
//		else if (this->cycle_counter <= 172)
//		{
//			*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000011);
//		}
//		// h blank, mode 0
//		else
//		{
//			*lcdstat_register_ptr = ((*lcdstat_register_ptr & 0b11111100) | 0b00000000);
//			irq_needed = mode0_irq;
//		}
//
//
//	}
//
//	if (original_mode != (*lcdstat_register_ptr & 0b00000011) && irq_needed)
//		this->bus->cpu.request_interrupt(lcdstat);
//	
//	//time to check LYC = LY
//
//	//set register for equality
//	if (*ly_ptr == this->bus->io[LYC - IOOFFSET])
//	{
//		*lcdstat_register_ptr |= 0b00000100;
//		// if interrupt is enabled
//		if (lyc_ly_irq)
//			this->bus->cpu.request_interrupt(lcdstat);
//	}
//	else
//		*lcdstat_register_ptr &= ~0b00000100;
//	
//}

bool PPU::lcdEnabled()
{
	return (bool)(*registers.lcdc & (0b1 << 7));

}


/// <summary>
/// output 8000-8FFF or 8800-97FF
/// </summary>
Word PPU::getTileAddressFromNumber(const Byte tile_number, const enum eTileType eTileType)
{
	switch (eTileType)
	{
	case PPU::sprite:
		return 0x8000 + (tile_number * 16);
		break;

	case PPU::background:
	case PPU::window:
	{
		bool addressing_mode = (*registers.lcdc & (0b1 << 4));
		// LCDC.4 = 1, $8000 addressing
		if (addressing_mode == 1)
		{
			return 0x8000 + (tile_number * 16);
			break;
		}
		// LCDC.4 = 0, $9000 addressing
		return (tile_number > 127) ? (0x8800 + tile_number * 16) : (0x9000 + tile_number * 16);
		break;
	}

	default: fprintf(stderr, "Unreachable eTileType");  exit(-1); break;
	}
}

//void PPU::render_scanline()
//{
//	//resets fifos
//	//this->fifo_bg = FIFO();
//	//this->fifo_sprite = FIFO();
//
//	// temp setting of scx, scy, ly registers
//	this->setMemory(SCX, 0x80);
//	this->setMemory(SCY, 0x40);
//	this->bus->io[LY - IOOFFSET] = 0x00;
//	
//	// get tile number and address of topleft tile of viewport
//	Byte tile_number = this->fifo_bg.fetcher.getTileNumber();
//	Word tile_address = this->get_tile_address(tile_number, PPU::background);
//
//	// get scy
//	Byte scy = this->getMemory(SCY);
//
//	// get top and bottom byte of 8 pixel line from tile
//	Byte line_data0 = this->getMemory(tile_address + 2 * (scy % 8));
//	Byte line_data1 = this->getMemory((tile_address + 1) + 2 * (scy % 8));
//	
//	for (int i = 0; i < 8; i++) 
//	{
//		// get colour of pixel
//		int offset = (0b1 << (7 - i));
//		bool bit0 = line_data0 & offset;
//		bool bit1 = line_data1 & offset;
//		Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);
//		
//		//push to fifo
//		this->fifo_bg.push(FIFOPixel(colour, 0, 0, 0));
//
//	}
//
//	// get ly for setting to framebuffer
//	Byte ly = this->getMemory(LY);
//	
//	
//	// pop fifo 8 times into framebuffer
//	for (int i = 0; i < 8; i++)
//	{
//		this->addToFramebuffer(i, ly, this->fifo_bg.pop());
//	}
//
//}
//


void PPU::addToFramebuffer(const int x, const int y, const FIFOPixel fifo_pixel)
{
	if (x < XRES && y < YRES)
		this->bus->framebuffer[static_cast<long long>(x) + (XRES * static_cast<long long>(y))] = this->dmgFramebufferPixelToRGB(fifo_pixel);
}


FramebufferPixel PPU::dmgFramebufferPixelToRGB(const FIFOPixel fifo_pixel)
{

	Byte palette_register = this->getMemory(0xFF47);

	Byte id_to_palette_id = 0;
	switch (fifo_pixel.colour)
	{
	case 0:
		id_to_palette_id = (palette_register & 0b00000011); break;
	case 1:
		id_to_palette_id = (palette_register & 0b00001100) >> 2; break;
	case 2:
		id_to_palette_id = (palette_register & 0b00110000) >> 4; break;
	case 3:
		id_to_palette_id = (palette_register & 0b11000000) >> 6; break;
	};

	switch (id_to_palette_id)
	{
	case 0:
		return FramebufferPixel(GB_PALLETE_00_r, GB_PALLETE_00_g, GB_PALLETE_00_b); // white
	case 1:
		return FramebufferPixel(GB_PALLETE_01_r, GB_PALLETE_01_g, GB_PALLETE_01_b); // light gray
	case 2:
		return FramebufferPixel(GB_PALLETE_10_r, GB_PALLETE_10_g, GB_PALLETE_10_b); // dark gray
	case 3:
		return FramebufferPixel(GB_PALLETE_11_r, GB_PALLETE_11_g, GB_PALLETE_11_b); // black
	default: fprintf(stderr, "Unreachable id_to_palette_id");  exit(-1); break;
	}
}

void PPU::newScanline()
{
	(*registers.ly)++;
	this->updateState(2);

	if (*registers.ly == 144)
	{
		this->updateState(1);
		this->bus->cpu.requestInterrupt(vblank);
	}

	if (*registers.ly > 153)
		*registers.ly = 0;

	this->cycle_counter = 0;
	this->scanline_x = 0;
	this->fifo_bg.reset();
	this->fifo_sprite.reset();
	this->window_wy_triggered = false;
}

void PPU::updateState(Byte new_state)
{
	Byte original_mode = (*registers.stat & 0x3);
	bool irq_needed = false;

	switch (new_state)
	{
	case 0: {
		*registers.stat = (*registers.stat & 0xFC) | 0x0;
		irq_needed = (*registers.stat & 0b00001000);
	}break;

	case 1: {
		*registers.stat = (*registers.stat & 0xFC) | 0x1;
		if (this->lcdEnabled())
			irq_needed = (*registers.stat & 0b00010000);
	}break;

	case 2: {
		*registers.stat = (*registers.stat & 0xFC) | 0x2;
		irq_needed = (*registers.stat & 0b00100000);
	}break;

	case 3: {
		*registers.stat = (*registers.stat & 0xFC) | 0x3;
	}break;

	default: fprintf(stderr, "Unreachable PPU new state"); exit(-1); break;
	}

	//if mode has changed

	if ((original_mode != (*registers.stat & 0x3)) && irq_needed)
		this->bus->cpu.requestInterrupt(lcdstat);

	//time to check LYC = LY

//set register for equality
	if (*registers.ly == *registers.lyc)
	{
		*registers.stat |= 0b00000100;
		// if interrupt is enabled
		if (*registers.stat & 0b01000000)
			this->bus->cpu.requestInterrupt(lcdstat);
	}
	else
		*registers.stat &=  ~0b00000100;
}

Byte PPU::getPPUState()
{
	return *registers.stat & 0b00000011;
}

void Tile::consolePrint()
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

Byte Tile::getPixelColour(int x, int y)
{
	int offset = (0b1 << (7 - x));

	bool bit0 = this->bytes_per_tile[2 * static_cast<long long>(y)] & offset;
	bool bit1 = this->bytes_per_tile[(2 * static_cast<long long>(y)) + 1] & offset;

	Byte result = (((Byte)bit0 << 1) | (Byte)bit1);

	/*if (result != 0)
		std::cout << "";
	if (result == 00)
		std::cout << "  ";
	else
		std::cout << std::bitset<2>{result};
	*/
	return result;
}

Tile::Tile(BUS* bus, Word address)
{
	for (int i = 0; i < 16; i++)
	{
		this->bytes_per_tile[i] = bus->getMemory(address + i, eMemoryAccessType::ppu);
	}
}

Tile::Tile()
{
	this->bytes_per_tile.fill(0x00);
}

