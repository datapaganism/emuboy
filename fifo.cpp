#include "fifo.hpp"
#include "ppu.hpp"
#include "bus.hpp"



int FIFO::elemCount()
{
	if (head_pos > tail_pos)
		return (fifo_max_size - head_pos) + tail_pos + 1;
	return (tail_pos - head_pos) + 1;
}

FIFOPixel FIFO::pop()
{
	if (!empty)
	{
		FIFOPixel* temp = &queue[head_pos++];
		if (head_pos >= fifo_max_size)
			head_pos = 0;
		
		full = false;
		empty = head_pos == tail_pos;
		return *temp;
	}
	fprintf(stderr, "cannot pop empty fifo");  exit(-1);
}

void FIFO::push(FIFOPixel pixel)
{
	if (!full)
	{
		queue[tail_pos++] = pixel;
		if (tail_pos >= fifo_max_size)
			tail_pos = 0;

		empty = false;
		full = head_pos == tail_pos;
		return;
	}
	fprintf(stderr, "cannot push full fifo");  exit(-1);
}

void FIFO::updateFIFO(int cycles_used)
{
	fifo_cycle_counter += cycles_used;

	while (fifo_cycle_counter >= (4 / 4))
	{
		fifo_cycle_counter -= (4 / 4);

		// while fifo not empty
		if (!empty)
		{
			Byte ly = *ppu->registers.ly;

			// between 0 and 159 pixels portion of the scanline
			if (ppu->scanline_x < 160)
			{
				/*
					Check scx register, % 8 gives the amount of pixels we are within a tile, if not 0, pop the fifo by the result
				*/
				Byte scx_pop = *ppu->registers.scx % 8;
				if ((ppu->scanline_x == 0) && (scx_pop != 0))
					for (int i = 0; i < (scx_pop); i++)
						pop();

				/*
				The scroll registers are re - read on each tile fetch, except for the low 3 bits of SCX, which are only read at the beginning of the scanline(for the initial shifting of pixels).

					All models before the CGB - D read the Y coordinate once for each bitplane(so a very precisely timed SCY write allows �desyncing� them), but CGB - D and later use the same Y coordinate for both no matter what.
					*/
				if (ly < 144 && ppu->scanline_x < 160)
					ppu->addToFramebuffer(ppu->scanline_x, ly, pop());

				ppu->scanline_x++;
			}
		}
	}
}

void FIFO::reset()
{
	tail_pos = 0;
	head_pos = 0;
	fifo_cycle_counter = 0;
	fetcher.reset();
	empty = true;
	full = false;
}


FIFO::FIFO()
{
	fetcher.connectToFIFO(this);
}


void FIFO::connectToPPU(PPU* pPPU)
{
	ppu = pPPU;
}

