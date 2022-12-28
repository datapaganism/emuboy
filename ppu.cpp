#include "ppu.hpp"
#include "bus.hpp"
#include <bitset>
#include <algorithm>

// ppu pushes for 4 pixels per NOP
// https://www.reddit.com/r/EmuDev/comments/8uahbc/dmg_bgb_lcd_timings_and_cnt/e1iooum/


PPU::PPU()
{
	this->fifo.connectToPPU(this);
}

// called during BUS consturctor 
void PPU::connectToBus(BUS* pBus)
{
	this->bus = pBus;
	this->setRegisters();
}

Byte PPU::getMemory(const Word address)
{
	return bus->getMemory(address, eMemoryAccessType::ppu);
}

void PPU::setMemory(const Word address, const Byte data)
{
	bus->setMemory(address, data, eMemoryAccessType::ppu);
}

// sorts the array based on the x pos of each OAM entry
void PPU::sortOAMPriority()
{
	int size = oam_priority.size() - 1;
	std::sort(oam_priority.queue.begin(), oam_priority.queue.begin() + size, [](OAMentry* a, OAMentry* b) {return a->x_pos < b->x_pos; });
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

void PPU::updateGraphics(const int tcycles)
{
	// one frame takes 70221 tcycles
	// one scanline is 456 tcycles 

	// the cpu may have modified the registers since the last cycle, it is time to check and update any changes of the lcd stat.
	//this->update_lcdstat();

	/*
		every cpu machine cycle is eqv to 2 tcycles of the 

		dot to mcycle -> / 4
		scanline is 456 dots, or 114 mcycles
		oam scan is 20 mcycles

		70221 dots is a single frame
	*/

	if (this->lcdEnabled())
	{

		this->cycle_counter += tcycles;

		switch (*registers.stat & 0b00000011)
		{
		case ePPUstate::h_blank: // h blank
		{
			if (this->cycle_counter >= (456))
			{
				this->newScanline();
			}
		} break;

		case ePPUstate::v_blank: // v blank
		{
			if (this->cycle_counter >= (456))
			{
				this->newScanline();
			}
		} break;

		case ePPUstate::oam_search: // oam search
		{
			if (*registers.wy == *registers.ly)
				this->window_wy_triggered = true;

			int sprite_height = (*registers.lcdc & 0b1 << 2) ? 16 : 8;
			for (int i = 0; i < tcycles / 2; i++)
			{
				struct OAMentry* entry = (OAMentry*)this->bus->oam_ram.get() + oam_scan_iterator++;
				if (entry->x_pos != 0)
				{
					//printf("%02i | %x %x %x %x\n", oam_scan_iterator, entry->y_pos, entry->x_pos, entry->tile_no, entry->attribute);
					//if (*registers.ly == 0x64)
					//	NO_OP;
					// this will by very buggy
					if (*registers.ly >= (entry->y_pos - 16) && *registers.ly < (entry->y_pos - 16) + sprite_height)
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

			if (this->cycle_counter >= (80))
			{
				if (oam_scan_iterator != 40)
				{
					fprintf(stderr, "OAM scan should have checked 40 objs, not %i", oam_scan_iterator);  exit(-40);
				}

				if (oam_priority.size() > 1)
					this->sortOAMPriority();
				this->updateState(ePPUstate::graphics_transfer);
			}
		} break;

		case ePPUstate::graphics_transfer: // graphics transfer
		{
			//fetch and then render pixels
			this->fifo.fetchPixels(tcycles);
			this->fifo.renderPixels(tcycles);

			if (this->scanline_x >= 160)
				this->updateState(ePPUstate::h_blank);
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
	//std::cout << cycle_counter << "\n";
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
	this->fifo.reset();
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
	this->fifo.push(pixel);
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
