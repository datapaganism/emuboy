#include "fifo.hpp"
#include "stack.hpp"
#include "FIFO_pixel.hpp"
#include <iostream>

int main()
{

	const int fifo_max_size = 16;
	FIFO fifo;
	//Stack<FIFOPixel> stack;
	const FIFOPixel test(1, 1, 0, 0);

	for (int i = 0; i < fifo_max_size; i++)
		fifo.push(test);

	for (int i = 0; i < fifo_max_size; i++)
		fifo.pop();

	for (int i = 0; i < fifo_max_size; i++)
		fifo.push(test);

	fifo.reset();

	return 0;
}