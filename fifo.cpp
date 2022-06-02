#include "fifo.hpp"
#include "ppu.hpp"
#include "bus.hpp"


void FIFO::push(FIFO_pixel pixel)
{
	if (tail_pos <= 15)
	{
		this->queue[++tail_pos] = pixel;
	}
}


FIFO_pixel FIFO::pop()
{
	if (tail_pos > -1)
	{
		FIFO_pixel temp = this->queue[0];
		for (int i = 0; i < this->tail_pos; i++)
		{
			this->queue[i] = this->queue[i + 1];
		}
		memset(&this->queue[this->tail_pos], 0, sizeof(FIFO_pixel));
		this->tail_pos--;

		return temp;
	}
	throw "unreachable pop";
}

void FIFO::update_fifo(int cyclesUsed)
{
	this->fifo_cycle_counter += cyclesUsed;

	while (this->fifo_cycle_counter >= (4/4))
	{
		this->fifo_cycle_counter -= (4/4);


		// while fifo not empty
		if (this->tail_pos > -1)
		{
			BUS* pBus = this->ppu_parent->bus;
			Byte ly = pBus->io[LY - IOOFFSET];
			
			// between 0 and 159 pixels portion of the scanline
			if ( this->ppu_parent->scanline_x < 160)
			{
				/*
					Check scx register, % 8 gives the amount of pixels we are within a tile, if not 0, pop the fifo by the result
				*/
				Byte scx_pop = pBus->io[SCX - IOOFFSET] % 8;
				if ((this->ppu_parent->scanline_x == 0) && (scx_pop != 0))
				{
					for (int i = 0; i < (scx_pop); i++)
					{
						this->pop();
					}
				}

				/*
				The scroll registers are re - read on each tile fetch, except for the low 3 bits of SCX, which are only read at the beginning of the scanline(for the initial shifting of pixels).

					All models before the CGB - D read the Y coordinate once for each bitplane(so a very precisely timed SCY write allows �desyncing� them), but CGB - D and later use the same Y coordinate for both no matter what.
					*/
				if (ly < 144 && this->ppu_parent->scanline_x < 160)
					this->ppu_parent->add_to_framebuffer(this->ppu_parent->scanline_x, ly, this->pop());


				this->ppu_parent->scanline_x++;
			}


		}
	}
}

void FIFO::reset()
{
	this->tail_pos = -1;
	this->queue.fill(FIFO_pixel(NULL, NULL, NULL, NULL));
	this->fifo_cycle_counter = 0;
	this->fetcher.reset();
}

FIFO::FIFO()
{
	this->fetcher.connect_to_fifo(this);
}

void FIFO::connect_to_ppu(PPU* pPPU)
{
	this->ppu_parent = pPPU;
}

