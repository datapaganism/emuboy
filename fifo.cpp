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

void FIFO::popBy(int count)
{
	if (!empty)
	{
		head_pos+= count;
		if (head_pos >= fifo_max_size)
			head_pos = 0;
		full = false;
		empty = head_pos == tail_pos;
		return;
	}
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

