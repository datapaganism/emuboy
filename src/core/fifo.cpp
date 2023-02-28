#include "fifo.hpp"
#include "bus.hpp"
#include "ppu.hpp"


//https://www.reddit.com/r/EmuDev/comments/s6cpis/gameboy_trying_to_understand_sprite_fifo_behavior/

PixelFIFO::PixelFIFO()
{

}

void PixelFIFO::connectToPPU(PPU* ppu_ptr)
{
	this->ppu = ppu_ptr;
}

Byte PixelFIFO::getTileNumber(Word base_tilemap_address)
{
	//base address is always at ly=0, incrementing it here is improtant
	base_tilemap_address += (*ppu->registers.ly / 8) * 0x20;

	Byte tile_number = ppu->getMemory(base_tilemap_address);

	return tile_number;
}

void PixelFIFO::renderPixels(const int cycles)
{
	if (this->rendering_sprite)
		return;

	for (int i = 0; i < cycles; i++)
	{
		// while background fifo not empty
		if (!fifo_stack.empty())
		{
			const Byte ly = *ppu->registers.ly;

			// between 0 and 159 pixels portion of the scanline
			if (ppu->current_x_of_scanline < 160)
			{
				/*
					Check scx register, % 8 gives the amount of pixels we are within a tile, if not 0, pop the fifo by the result
				*/
				if (!this->rendering_sprite || !rendering_window)
				{
					const Byte scx_pop = *ppu->registers.scx % 8;
					if ((ppu->current_x_of_scanline == 0) && (scx_pop != 0))
					{
						//popBy(scx_pop);
						for (int i = 0; i < scx_pop; i++)
							fifo_stack.pop();
					}
				}
				/*
				The scroll registers are re - read on each tile fetch, except for the low 3 bits of SCX, which are only read at the beginning of the scanline(for the initial shifting of pixels).

					All models before the CGB - D read the Y coordinate once for each bitplane(so a very precisely timed SCY write allows �desyncing� them), but CGB - D and later use the same Y coordinate for both no matter what.
					*/
				if (ly < 144 && ppu->current_x_of_scanline < 160)
				{
					ppu->addToFramebuffer(ppu->current_x_of_scanline, ly, fifo_stack.front());
					fifo_stack.pop();
				}
				ppu->current_x_of_scanline++;
			}
		}
	}
}

/*
	if wx + 7 >= scx and wy >= scy
	then window is inside viewport

	word base_address;

	(lcdc.4) ? base_address = 0x9C00 : base_address = 0x9800;

	// if current x and y inside window
	if ((ly >= wy) && (fetcher_scanline_x_tiles >= (wx + 7))) AND window is active?
	{
		(lcdc.6) ? base_address = 0x9C00 : 0x9800;
	}



	if.lcdc.3 == 1 and fetcher_scanline_x_tiles not inside window then use 0x9c00
	if lcdc.3 == 1 and fetcher_scanline_x_tiles inside window, use lcdc.6 value for address.


	if lcdc.6 == 1 and fetcher_scanline_x_tiles is inside window then use 0x9c00
	if lcdc.6 == 0 and fetcher_scanline_x_tiles is not inside window then use lcdc.3 value for bg address


*/

void PixelFIFO::fetchPixels(const int cycles)
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

		Byte lcdc = *ppu->registers.lcdc;
		bool window_active = lcdc & 0b00100000;
		bool bg_active = lcdc & 0b1;

		Byte ly = *ppu->registers.ly;

		if (!rendering_sprite)
		{
			current_sprite = ppu->oam_priority.getBack();
			if (current_sprite != nullptr)
			{
				if (
					   ppu->current_x_of_scanline >= (current_sprite->x_pos - 8)            // if ppu's x is at sprite's x or bigger
					&& ppu->current_x_of_scanline < (current_sprite->x_pos - 8) + 8      // and ppu's x not passed sprite width
					&& ly >= (current_sprite->y_pos - 16)                                // if ly is at sprites ly
					&& ly < (current_sprite->y_pos - 16) +  16                    // and if ly is not passed sprite height
					)
				{
					// if there is a sprite on this line
					rendering_sprite = true;

					// if FIFO hasnt enough pixels to mix sprite with
					if (fifo_stack.size() < 8)
						fifo_needs_more_bgwin_pixels = true;
				}
			}
		}



		switch (state)
		{
		case eFetcherState::get_tile: // read tile address
		{
			if (ly < YRES)
			{
				if (rendering_sprite && !fifo_needs_more_bgwin_pixels)
				{
					this->tile_number = current_sprite->tile_no;
					this->tile_address = ppu->getTileAddressFromNumber(tile_number, PPU::sprite);
					this->state++;
					break;
				}
				else
				if (*ppu->registers.lcdc & (0b1 << 5) && ppu->window_wy_triggered && fetcher_scanline_x_tiles >= *ppu->registers.wx / 8) // window enabled and its going to be somewhere on this line
				{
					fetchPixels_state1_window();
				}
				else
				if (bg_active)
				{
					fetchPixels_state1_background();
				}

				
			}
			this->state++;
			break;
		} 
		case eFetcherState::get_tile_data_low: // get data 0
		{
			//if bg is not enabled, get no data
			this->data0 = 00;

			if (rendering_sprite && !fifo_needs_more_bgwin_pixels)
			{
				Byte current_scanline_y_offset = (ly - (current_sprite->y_pos - 16));
				Byte sprite_height = (*ppu->registers.lcdc & 0b1 << 2) ? 32 : 16;

				if (current_sprite->getYFlip())
				{
					tile_address += sprite_height;
					tile_address -= (2 * current_scanline_y_offset);
				}
				else
					tile_address += (2 * current_scanline_y_offset);



				this->data0 = ppu->getMemory(this->tile_address);
				this->state++;
				break;
			}
			else
			if (rendering_window)
			{
				this->tile_address += (2 * (fetcher_y_line % 8));
				this->data0 = ppu->getMemory(this->tile_address);
			}
			else
			if (bg_active)
			{
				this->tile_address += (2 * (fetcher_y_line % 8));
				this->data0 = ppu->getMemory(this->tile_address);
			}
			

			this->state++;
			break;
		}
		case eFetcherState::get_tile_data_high: // get data 1, construct 8 pixel buffer
		{
			//if bg is not enabled, get no data
			this->data1 = 00;

			if (rendering_sprite && !fifo_needs_more_bgwin_pixels)
			{
				incAddress();
				this->data1 = ppu->getMemory(this->tile_address);

				
				for (int i = 0; i < 8; i++)
				{
					// get colour of pixel
					int offset = (0b1 << (7 - i));
					bool bit0 = data0 & offset;
					bool bit1 = data1 & offset;
					Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

					//fetcher_pixels[(current_sprite->getXFlip()) ? 7 - i : i] = (FIFOPixel(colour, 0, current_sprite->getBGWinOverOBJ(), 0, sprite));
					fetcher_pixels.push(FIFOPixel(colour, 0, current_sprite->getBGWinOverOBJ(), 0, sprite));
				}
				if (current_sprite->getXFlip()) 
					reverse_queue(fetcher_pixels);

				this->state++;
				break;
			}


			if (bg_active || rendering_window)
			{
				incAddress();
				this->data1 = ppu->getMemory(this->tile_address);
			}

			for (int i = 0; i < 8; i++)
			{
				// get colour of pixel
				int offset = (0b1 << (7 - i));
				bool bit0 = this->data0 & offset;
				bool bit1 = this->data1 & offset;
				Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

				fetcher_pixels.push(FIFOPixel(colour, 0, 0, 0, (rendering_window) ? win : bg));
			}
			// rendering a sprite should not increment the PixelFIFO's x position
			fetcher_scanline_x_tiles++;
			if (rendering_window)
				fetcher_scanline_wx_tiles++;

			this->state++;
			break;
		};
		case eFetcherState::sleep:
		{
			// sleep if we cant push to fifo but skip of we are rendering a sprite
			if (fifo_stack.size() > 8 && !rendering_sprite)
				break;
			this->state++;
		}
		case eFetcherState::push:
		{
			if (rendering_sprite && !fifo_needs_more_bgwin_pixels)
			{
				OAMentry* obj = ppu->oam_priority.pop();
				rendering_sprite = false;

				std::queue<FIFOPixel> sprite_bg_mixed;
				Byte loop_till = fifo_stack.size();
				for (int i = 0; i < loop_till; i++)
				{
					if (!fetcher_pixels.empty())
					{
						if (
							fetcher_pixels.front().colour != 0
							//&& original_fifo_pixels[i].colour == 0
							//&& pixels[i].sprite_priority
							)
						{
							fifo_stack.front().colour = fetcher_pixels.front().colour;
						}
						fetcher_pixels.pop();
					}
					sprite_bg_mixed.push(fifo_stack.front());
					fifo_stack.pop();
				}
				fifo_stack.swap(sprite_bg_mixed);

				this->state = eFetcherState::get_tile;
				break;
			}


			// PUSH bg/win pixels to fifo
			for (int i = 0; i < 8; i++)
			{
				fifo_stack.push(fetcher_pixels.front());
				fetcher_pixels.pop();
			}

			if (fifo_needs_more_bgwin_pixels)
				fifo_needs_more_bgwin_pixels = false;

			this->state = eFetcherState::get_tile;
			break;
		}
		};
	}
}

void PixelFIFO::reset()
{
	this->data0 = NULL;
	this->data1 = NULL;
	this->state = 0;
	this->cycle_counter = 0;
	this->tile_number = 0;
	this->tile_address = 0;
	this->tile_map_address = 0;

	fetcher_scanline_x_tiles = 0;
	fetcher_scanline_wx_tiles = 0;
	fetcher_x_tile = 0;
	fetcher_y_line = 0;

	rendering_window = false;
	rendering_sprite = false;
	fifo_needs_more_bgwin_pixels = false;
	//FIFOStack<>::reset();
	fetcher_pixels.swap(std::queue<FIFOPixel>());
	fifo_stack.swap(std::queue<FIFOPixel>());

}

void PixelFIFO::incAddress()
{
	Byte inc = this->tile_address & 0x1F;
	inc++;

	this->tile_address |= (inc & 0x1F);
}


bool PixelFIFO::isWindowActive()
{
	return *ppu->registers.lcdc & 0b00100000;
}

void PixelFIFO::fetchPixels_state1_window()
{
	/*
					Besides the Background, there is also a Window overlaying it.The content of the Window is not scrollable;
					it is always displayed starting at the top left tile of its tile map.
						The only way to adjust the Window is by modifying its position on the screen,
						which is done via the WXand WY registers.The screen coordinates of the top left corner of the Window are(WX - 7, WY).
						The tiles for the Window are stored in the Tile Data Table.Both the Backgroundand the Window share the same Tile Data Table.
						Whether the Window is displayed can be toggled using LCDC bit 5. But in Non - CGB mode this bit is only functional as long as LCDC bit 0 is set.
						Enabling the Window makes Mode 3 slightly longer on scanlines where it�s visible.
						(See WX and WY for the definition of �Window visibility�.)
					*/

	Byte wx = *ppu->registers.wx;
	Byte wy = *ppu->registers.wy;


	if (!fifo_stack.empty())
	{
		if(fifo_stack.back().type != FIFOPixelType::win)
		{
			fifo_stack.swap(std::queue<FIFOPixel>());
		}
	}

	rendering_window = true;

	fetcher_x_tile = fetcher_scanline_wx_tiles;
	fetcher_y_line = (*ppu->registers.ly - *ppu->registers.wy) % 8;

	Byte wy_tile_number = (*ppu->registers.ly - *ppu->registers.wy) / 8;



	Word win_tile_map_area_address = (*ppu->registers.lcdc & (0b1 << 6)) ? 0x9C00 : 0x9800;
	this->tile_map_address = win_tile_map_area_address + (fetcher_x_tile) + ((wy_tile_number) * 0x20);
	this->tile_number = ppu->getMemory(this->tile_map_address);
	this->tile_address = ppu->getTileAddressFromNumber(this->tile_number, PPU::window); // basically 0x8000 + (16 * tile_number)
}

void PixelFIFO::fetchPixels_state1_background()
{
	fetcher_x_tile = ((*ppu->registers.scx / 8) + fetcher_scanline_x_tiles) & 0x1F;
	fetcher_y_line = (*ppu->registers.scy + *ppu->registers.ly) & 255;

	int fetcher_y_tile = ((fetcher_y_line / 8));

	Word bg_tile_map_area_address = ((*ppu->registers.lcdc & (0b1 << 4)) == 1) ? 0x9C00 : 0x9800;
	this->tile_map_address = bg_tile_map_area_address + (fetcher_x_tile)+((fetcher_y_line / 8) * 0x20);
	this->tile_number = ppu->getMemory(this->tile_map_address);
	this->tile_address = ppu->getTileAddressFromNumber(this->tile_number, PPU::background); // basically 0x8000 + (16 * tile_number)
}

void PixelFIFO::reverse_queue(std::queue<FIFOPixel>& queue)
{
	std::stack<FIFOPixel> stack;
	while (!queue.empty())
	{
		stack.push(queue.front());
		queue.pop();
	}
	while (!stack.empty())
	{
		queue.push(stack.top());
		stack.pop();
	}

}


