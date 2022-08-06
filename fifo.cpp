#include "fifo.hpp"
#include "ppu.hpp"
#include "bus.hpp"


void FIFO::push(FIFOPixel pixel)
{
	if (tail_pos <= 15)
		this->queue[++tail_pos] = pixel;
}


FIFOPixel FIFO::pop()
{
	if (tail_pos > -1)
	{
		FIFOPixel temp = this->queue[0];
		for (int i = 0; i < this->tail_pos; i++)
			this->queue[i] = this->queue[i + 1];
		memset(&this->queue[this->tail_pos], 0, sizeof(FIFOPixel));
		this->tail_pos--;
		return temp;
	}
	throw "unreachable pop";
}

void FIFO::updateFIFO(int cycles_used)
{
	this->fifo_cycle_counter += cycles_used;

	while (this->fifo_cycle_counter >= (4/4))
	{
		this->fifo_cycle_counter -= (4/4);

		// while fifo not empty
		if (this->tail_pos > -1)
		{
			Byte ly = *ppu->registers.ly;
			
			// between 0 and 159 pixels portion of the scanline
			if ( ppu->scanline_x < 160)
			{
				/*
					Check scx register, % 8 gives the amount of pixels we are within a tile, if not 0, pop the fifo by the result
				*/
				Byte scx_pop = *ppu->registers.scx % 8;
				if ((this->ppu->scanline_x == 0) && (scx_pop != 0))
					for (int i = 0; i < (scx_pop); i++)
						this->pop();

				/*
				The scroll registers are re - read on each tile fetch, except for the low 3 bits of SCX, which are only read at the beginning of the scanline(for the initial shifting of pixels).

					All models before the CGB - D read the Y coordinate once for each bitplane(so a very precisely timed SCY write allows �desyncing� them), but CGB - D and later use the same Y coordinate for both no matter what.
					*/
				if (ly < 144 && this->ppu->scanline_x < 160)
					this->ppu->addToFramebuffer(this->ppu->scanline_x, ly, this->pop());

				this->ppu->scanline_x++;
			}
		}
	}
}

void FIFO::reset()
{
	this->tail_pos = -1;
	this->queue.fill(FIFOPixel(NULL, NULL, NULL, NULL));
	this->fifo_cycle_counter = 0;
	this->fetcher.reset();
}

FIFO::FIFO()
{
	this->fetcher.connectToFIFO(this);
}

void FIFO::connectToPPU(PPU* pPPU)
{
	this->ppu = pPPU;
}

