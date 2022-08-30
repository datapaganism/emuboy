#include "fifo.hpp"
#include "FIFO_pixel.hpp"
#include <iostream>

int main()
{
	FIFO fifo;
	const FIFOPixel test(1, 1, 0, 0);

	for (int i = 0; i < fifo_max_size; i++)
		fifo.push(test);

	for (int i = 0; i < fifo_max_size; i++)
		fifo.pop();

	for (int i = 0; i < fifo_max_size; i++)
		fifo.push(test);


	return 0;
}