#include "ppu.hpp"
#include "bus.hpp"
#include <bitset>

// ppu pushes for 4 pixels per NOP
// https://www.reddit.com/r/EmuDev/comments/8uahbc/dmg_bgb_lcd_timings_and_cnt/e1iooum/


PPU::PPU()
{
	this->fifo_bg.connectToPPU(this);
	this->fifo_bg.fetcher.connectToPPU(this);
	this->fifo_oam.connectToPPU(this);
	this->fifo_oam.fetcher.connectToPPU(this);

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

// this function takes over updateFIFO from both of the fifos
// we have to pull pixels from both fifos and combine them for rendering.
void PPU::clockFIFOmCycle()
{
	if (this->fifo_bg.fetcher.rendering_sprite)
		return;

	for (int i = 0; i < 4; i++)
	{
		// while background fifo not empty
		if (!fifo_bg.empty)
		{
			const Byte ly = *registers.ly;

			// between 0 and 159 pixels portion of the scanline
			if (scanline_x < 160)
			{
				/*
					Check scx register, % 8 gives the amount of pixels we are within a tile, if not 0, pop the fifo by the result
				*/
				const Byte scx_pop = *registers.scx % 8;
				if ((scanline_x == 0) && (scx_pop != 0))
				{
					fifo_bg.popBy(scx_pop);
				}
				/*
				The scroll registers are re - read on each tile fetch, except for the low 3 bits of SCX, which are only read at the beginning of the scanline(for the initial shifting of pixels).

					All models before the CGB - D read the Y coordinate once for each bitplane(so a very precisely timed SCY write allows �desyncing� them), but CGB - D and later use the same Y coordinate for both no matter what.
					*/
				if (ly < 144 && scanline_x < 160)
					addToFramebuffer(scanline_x, ly, fifo_bg.pop());

				scanline_x++;
			}
		}
	}
}

FIFOPixel PPU::combinePixels()
{
	if (!fifo_bg.empty && !fifo_oam.empty)
	{
		FIFOPixel bg = fifo_bg.pop();
		FIFOPixel sprite = fifo_oam.pop();
		
		if (sprite.colour == 0x0)
			// mask sprite and OR with bg background color

		//need to mix the pixels, not finished
		return FIFOPixel();
	}
	return fifo_bg.pop();
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

	if (this->lcdEnabled())
	{

		this->cycle_counter += cycles;

		switch (*registers.stat & 0b00000011)
		{
		case ePPUstate::h_blank: // h blank
		{
			if (this->cycle_counter >= (456 / 4))
			{
				this->newScanline();
			}
		} break;

		case ePPUstate::v_blank: // v blank
		{
			if (this->cycle_counter >= (456 / 4))
			{
				this->newScanline();
			}
		} break;

		case ePPUstate::oam_search: // oam search
		{
			// do stuff
			if (*registers.wy == *registers.ly)
				this->window_wy_triggered = true;

			int sprite_height = (*registers.lcdc & 0b00000010) ? 16 : 8;
			for (int i = 0; i < cycles * 2; i++)
			{
				struct OAMentry* entry = (OAMentry*)this->bus->oam_ram.get() + oam_scan_iterator++;
				if (entry->x_pos != 0)
				{
					// this will by very buggy
					if (*registers.ly + 16 >= entry->y_pos && *registers.ly + 16 < entry->y_pos + sprite_height)
					{
						oam_priority.push(entry);
						if (oam_priority.full)
							break;
					}
				}
			}

			// OAM selection priority, during each scanline the PPU can only render 10 sprites, a hardware limitation.
			// the scan will go through the OAM sequentially, checking if an entry's Y is within LY and making sure that we check the lcdc.2 obj size.
			// I will scan the oam and if we find matches I will store them in an array that the fetcher can access when needed.
			//int sprite_height = (*registers.lcdc & 0b00000010) ? 16 : 8;
			//for (oam_scan_iterator; oam_scan_iterator < 40; oam_scan_iterator++)
			//{
			//	struct OAMentry* entry = (OAMentry*)this->bus->oam_ram.get() + oam_scan_iterator;
			//	if (entry->x_pos != 0)
			//	{
			//		// this will by very buggy
			//		if (*registers.ly + 16 >= entry->y_pos && *registers.ly + 16 < entry->y_pos + sprite_height)
			//		{
			//			oam_priority.push(entry);
			//			if (oam_priority.full)
			//				break;
			//		}
			//	}
			//}

			if (this->cycle_counter >= (80 / 4))
			{
				if (oam_scan_iterator != 40)
					exit(40);
				this->updateState(ePPUstate::graphics_transfer);
			}
		} break;

		case ePPUstate::graphics_transfer: // graphics transfer
		{
			// update bg/win fetcher and fifo
			
			// Need to write special fetcher for sprites
			this->fifo_bg.fetcher.updateFetcher(cycles);
			//this->fifo_sprite.fetcher.updateFetcher(cycles);

			clockFIFOmCycle();

			if (this->scanline_x >= 160)
				this->updateState(ePPUstate::h_blank);

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
	this->updateState(ePPUstate::h_blank);

	return;
}



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
		// LCDC.4 = 0, $8800/9000 addressing
		// tile numbers 0-127 use 0x9000 addressing
		// tile numbers 128-255 use 0x8800 addressing
		return (tile_number > 127) ? (0x8800 + (tile_number - 128) * 16) : (0x9000 + tile_number * 16);
		break;
	}

	default: fprintf(stderr, "Unreachable eTileType");  exit(-1); break;
	}
}



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
	this->fifo_oam.reset();
	this->window_wy_triggered = false;
	oam_priority.reset();
	oam_scan_iterator = 0;
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

void PPU::debugAddToBGFIFO(FIFOPixel pixel)
{
	this->fifo_bg.push(pixel);
}

void PPU::debugAddToOAMFIFO(FIFOPixel pixel)
{
	this->fifo_oam.push(pixel);
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
