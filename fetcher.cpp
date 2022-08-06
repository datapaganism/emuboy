#include "fetcher.hpp"
#include "fifo.hpp"
#include "bus.hpp"

Fetcher::Fetcher()
{

}

void Fetcher::connectToFIFO(FIFO* pFIFO)
{
	this->fifo_parent = pFIFO;
}

Word Fetcher::SCRegistersToTopLeftBGMapAddress()
{
	BUS* bus = this->fifo_parent->ppu_parent->bus;
	Word base_address = 0x9800;

	Byte scx = bus->io[SCX - IOOFFSET];
	Byte scy = bus->io[SCY - IOOFFSET];

	//offset base address by scx and scy registers, scy/8 tells us how many tiles to move down, 0x20 represents one line in address  
	base_address += (((scy / 8) * 0x20) + (scx / 8));

	return base_address;
}

Byte Fetcher::getTileNumber(Word address)
{
	BUS* bus = this->fifo_parent->ppu_parent->bus;
	Word base_tilemap_address = address;
	
	//base address is always at ly=0, incrementing it here is improtant
	base_tilemap_address += ((bus->getMemory(LY, eMemoryAccessType::ppu) / 8) * 0x20);
	
	Byte tile_number = bus->getMemory(base_tilemap_address, eMemoryAccessType::ppu);

	return tile_number;
}

void Fetcher::updateFetcher(const int cycles)
{
	// Window stuff
	/*
		Top left corner (WX-7, WY) Wx registers
		Controlled by lcdc bit 5, overridden by bit 0
	*/
	this->cycle_counter += cycles;

	while (this->cycle_counter >= 2)
	{
		this->cycle_counter -= 2;
		
		BUS* bus = this->fifo_parent->ppu_parent->bus;

		Byte lcdc_register = bus->getMemory(LCDC, eMemoryAccessType::ppu);
		bool window_active = lcdc_register & 0b00100000;
		bool bg_active = lcdc_register & 0b1;


		/*
			if wx + 7 >= scx and wy >= scy
			then window is inside viewport

			word base_address;

			(lcdc.4) ? base_address = 0x9C00 : base_address = 0x9800;

			// if current x and y inside window
			if ((ly >= wy) && (fetcher_scanline_x >= (wx + 7))) AND window is active?
			{
				(lcdc.6) ? base_address = 0x9C00 : 0x9800;	
			}



			if.lcdc.3 == 1 and fetcher_scanline_x not inside window then use 0x9c00
			if lcdc.3 == 1 and fetcher_scanline_x inside window, use lcdc.6 value for address.


			if lcdc.6 == 1 and fetcher_scanline_x is inside window then use 0x9c00
			if lcdc.6 == 0 and fetcher_scanline_x is not inside window then use lcdc.3 value for bg address


		*/
		Byte ly = bus->io[LY - IOOFFSET];

		switch (this->state)
		{
		case 0: // read tile address
		{
			//Word base_address;
			if (bus->io[LY - IOOFFSET] < YRES)
				if (bg_active)
				{
					this->fetcher_x = ((bus->io[SCX - IOOFFSET] / 8) + this->fetcher_scanline_x) & 0x1F;
					this->fetcher_y = (bus->io[SCY - IOOFFSET] + bus->io[LY - IOOFFSET]) & 255;

					Word bg_tile_map_area_address = ((lcdc_register & (0b1 << 4)) == 1) ? 0x9C00 : 0x9800;
					this->tile_map_address = bg_tile_map_area_address + (this->fetcher_x) + ((this->fetcher_y / 8) * 0x20 );			
					//this->tile_number = bus->video_ram[this->tile_map_address - VIDEORAMOFFSET];
					this->tile_number = bus->getMemory(this->tile_map_address, eMemoryAccessType::ppu);
					this->tile_address = bus->ppu.getTileAddressFromNumber(this->tile_number,PPU::background); // basically 0x8000 + (16 * tile_number)
				}

				if (this->isWindowActive())
				{
					Byte wx = bus->io[WX - IOOFFSET];
					Byte wy = bus->io[WY - IOOFFSET];
					Byte ly = bus->io[LY - IOOFFSET];

					/*
						Besides the Background, there is also a Window overlaying it. The content of the Window is not scrollable; it is always displayed starting at the top left tile of its tile map. The only way to adjust the Window is by modifying its position on the screen, which is done via the WX and WY registers. The screen coordinates of the top left corner of the Window are (WX-7,WY). The tiles for the Window are stored in the Tile Data Table. Both the Background and the Window share the same Tile Data Table.
						Whether the Window is displayed can be toggled using LCDC bit 5. But in Non-CGB mode this bit is only functional as long as LCDC bit 0 is set. Enabling the Window makes Mode 3 slightly longer on scanlines where it�s visible. (See WX and WY for the definition of �Window visibility�.)
					*/


					//if current x and y inside window
					if (this->fifo_parent->ppu_parent->window_wy_triggered && (this->fetcher_scanline_x >= (wx + 7)))
					{
						this->fifo_parent->reset();
						
						this->fetcher_x = (((wx + 7) / 8) + this->fetcher_scanline_x) & 0x1F;
						this->fetcher_y = (wy + bus->io[LY - IOOFFSET]) & 255;

						Word win_tile_map_area_address = ((bus->io[(LCDC) - IOOFFSET] & (0b1 << 6)) == 1) ? 0x9C00 : 0x9800;
						this->tile_map_address = win_tile_map_area_address + (this->fetcher_x) + ((this->fetcher_y / 8) * 0x20);
						this->tile_number = bus->getMemory(this->tile_map_address, eMemoryAccessType::ppu);
						this->tile_address = bus->ppu.getTileAddressFromNumber(this->tile_number, PPU::window); // basically 0x8000 + (16 * tile_number)
					}
				}

			this->state++;
		} break;
		case 1: // get data 0
		{
			//if bg is not enabled, get no data
			this->data0 = 00;

			if (bg_active || window_active)
			{
				this->tile_address += (2 * (this->fetcher_y % 8));
				this->data0 = bus->getMemory(this->tile_address, eMemoryAccessType::ppu);
			}

			
			this->state++;
		} break;
		case 2: // get data 1, construct 8 pixel buffer
		{
			
			//this->incAddress();			
			// 
			//if bg is not enabled, get no data
			this->data1 = 00;
			
			if (bg_active || window_active)
			{
				incAddress();
				this->data1 = bus->getMemory(this->tile_address, eMemoryAccessType::ppu);
			}

			for (int i = 0; i < 8; i++)
			{
				// get colour of pixel
				int offset = (0b1 << (7 - i));
				bool bit0 = this->data0 & offset;
				bool bit1 = this->data1 & offset;
				Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

				//push to fifo
				this->temp_buffer[i] = (FIFOPixel(colour, 0, 0, 0));
			}
			this->fetcher_scanline_x++;

			this->state++;
		};
		case 3: //load to fifo or wait
		{
			//if we cant push
			if (this->fifo_parent->tail_pos > 7)
				return;

			for (int i = 0; i < 8; i++)
			{
				this->fifo_parent->push(this->temp_buffer[i]);
			}

			this->state = 0;
		
		} break;
		};
	}
}

void Fetcher::reset()
{
	this->data0 = NULL;
	this->data1 = NULL;
	this->state = 0;
	this->cycle_counter = 0;
	this->temp_buffer.fill(FIFOPixel(NULL, NULL, NULL, NULL));
	this->address_to_read = 0;
	this->tile_number = 0;
	this->tile_address = 0;
	this->tile_map_address = 0;

	this->fetcher_scanline_x = 0;
	this->fetcher_x = 0;
	this->fetcher_y = 0;
}

void Fetcher::incAddress()
{
	Byte inc = this->tile_address & 0x1F;
	inc++;

	this->tile_address |= (inc & 0x1F);
}

bool Fetcher::isWindowActive()
{
	return this->fifo_parent->ppu_parent->getMemory(LCDC) & 0b00100000;
}
