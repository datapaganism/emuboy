#include "fetcher.hpp"
#include "fifo.hpp"
#include "bus.hpp"

FETCHER::FETCHER()
{

}

void FETCHER::connect_to_fifo(FIFO* pFIFO)
{
	this->fifo_parent = pFIFO;
}

//void FETCHER::get_tile() //windowless  //unused
//{
//	//mem address
//	// to tile number
//
//	BUS* pBus = this->fifo_parent->ppu_parent->bus;
//	Word base_tilemap_address = 0x9800;
//
//	Byte lcdc_status = pBus->get_memory(LCDC);
//	bool bg_tile_map_area = lcdc_status & (0b1 << 3);
//	bool win_tile_map_area = lcdc_status & (0b1 << 6);
//
//	Byte scx = pBus->get_memory(SCX);
//	Byte scy = pBus->get_memory(SCY);
//
//	base_tilemap_address = this->sc_registers_to_top_left_bg_map_address();
//	base_tilemap_address += (pBus->get_memory(LY) / 8) * 0x20;
//
//	/*
//	*   a tile is 8 wide
//		SCX / 8, denotes which viewport tile we are looking at, between 0 and 20
//
//
//
//	*/
//	//if bg tile
//	this->fetcher_x = ((SCX / 8) + this->fetcher_x) & 0x1F; // between 0 and 31
//	this->fetcher_y = (pBus->get_memory(LY) + scy); // between 0 and 159
//
//	Byte tile_number = pBus->get_memory(base_tilemap_address);
//
//}

Word FETCHER::sc_registers_to_top_left_bg_map_address()
{
	BUS* pBus = this->fifo_parent->ppu_parent->bus;
	Word base_address = 0x9800;

	Byte scx = pBus->io[SCX - IOOFFSET];
	Byte scy = pBus->io[SCY - IOOFFSET];

	//offset base address by scx and scy registers, scy/8 tells us how many tiles to move down, 0x20 represents one line in address  
	base_address += (((scy / 8) * 0x20) + (scx / 8));

	return base_address;
}

//Byte FETCHER::get_tile_number()
//{
//	BUS* pBus = this->fifo_parent->ppu_parent->bus;
//	Word base_tilemap_address = this->sc_registers_to_top_left_bg_map_address();
//	base_tilemap_address += (pBus->get_memory(LY) / 8) * 0x20;
//
//	Byte tile_number = pBus->get_memory(base_tilemap_address);
//
//	return tile_number;
//}

Byte FETCHER::get_tile_number(Word address)
{
	BUS* pBus = this->fifo_parent->ppu_parent->bus;
	Word base_tilemap_address = address;
	
	//base address is always at ly=0, incrementing it here is improtant
	base_tilemap_address += ((pBus->get_memory(LY) / 8) * 0x20);
	
	Byte tile_number = pBus->get_memory(base_tilemap_address);

	return tile_number;
}

void FETCHER::update_fetcher(const int cycles)
{
	this->cycle_counter += cycles;

	while (this->cycle_counter >= (8 / 4))
	{
		this->cycle_counter -= (8/4);

		switch (this->state)
		{
		case 0: // read tile address
		{
			this->tile_number = this->get_tile_number(this->sc_registers_to_top_left_bg_map_address() + (this->fifo_parent->ppu_parent->scanline_x / 8));
			this->state++;
		} break;
		case 1: // get data 0
		{
			this->address_to_read = this->fifo_parent->ppu_parent->get_tile_address_from_number(this->tile_number, PPU::background);
			Byte scy = this->fifo_parent->ppu_parent->bus-> get_memory(SCY);
			// select which line of tile to get, denoted by scy, needs to by *2 since a line is 2 bytes long
			this->data0 = this->fifo_parent->ppu_parent->bus->get_memory(this->address_to_read + 2 * (scy % 8));
			this->state++;
		} break;
		case 2: // get data 1, construct 8 pixel buffer
		{
			this->address_to_read++;
			Byte scy = this->fifo_parent->ppu_parent->bus->get_memory(SCY);
			this->data1 = this->fifo_parent->ppu_parent->bus->get_memory(this->address_to_read + 2 * (scy % 8));

			for (int i = 0; i < 8; i++)
			{
				// get colour of pixel
				int offset = (0b1 << (7 - i));
				bool bit0 = this->data0 & offset;
				bool bit1 = this->data1 & offset;
				Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

				//push to fifo
				this->temp_buffer[i] = (FIFO_pixel(colour, 0, 0, 0));
			}

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

void FETCHER::reset()
{
	this->data0 = NULL;
	this->data1 = NULL;
	this->state = 0;
	this->cycle_counter = 0;
	this->temp_buffer.fill(FIFO_pixel(NULL, NULL, NULL, NULL));
	this->address_to_read = 0;
	this->tile_number = 0;
}
