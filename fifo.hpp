#pragma once
#include <array>

#include "config.hpp"
#include "fetcher.hpp"
#include "FIFO_pixel.hpp"

class PPU;

/*
	Emulating a system is a lot of computational work, therefore the emulator needs to be designed with performance in mind.
	After I got to the point where I emulate enough of the features of the system I decided to use Visual Studio's performance profiler.
	This takes a snapshot of your program and then shows you a breakdown of what functions have been called and also how many times they have been called.
	This shows you what functions take up most of the real CPU time.

	I set a small challenge to myself to avoid using C++ standard library features wherever possible, I limited myself to what I deemed necessary like vector and smart pointers (through the use of unique_ptr) for the reasons of avoiding memory leaks.
	As the Gameboy uses a First In First Out data structure for the graphical pixels. I took it upon myself to build a simple FIFO queue.

	The very basics are that, there is a storage object where the queue is located and then an ability to push (add objects to the queue) and to pop (take the first object in the queue off it.)
	I implemented the features and they worked for my use case, however after checking the profiler, I saw that a lot of real CPU time was spent popping pixels, this makes sense there are 160x144 pixels in each frame and there are ~60 frames a second. This function is used nearly 1.4 million times a second.
	Looking through the code of the pop() function revealed that there was room for improvement, as popping a single pixel would cause every single pixel in the queue to be moved one position closer to the front. Always making the pixel required to be popped at the front of the queue as it logically makes sense.
	However this is quite inefficient, it is better to use storage container as a circular buffer. We can keep pushing pixels onto it and let it wrap around when there are too many pixels on it, effectively overwriting the oldest pixels pushed.
	While this doesn't sound like a FIFO queue, we can track which pixel needs to popped next as the front of the queue will no longer hold the first-out element. By letting the front chase the tail in circles in this buffer, we avoid the uncessary shifting of elements and increase performance of the emulation, allowing to run the emulator on lower hardware or allow headroom for futher complexity to be implemented.

*/


constexpr int fifo_max_size = 16;
const FIFOPixel blank_pixel(0, 0, 0, 0);

class FIFO
{
public:
	FIFO();

	Fetcher fetcher;
	std::array<FIFOPixel, fifo_max_size> queue;
	bool can_fetch = true;
	int fifo_cycle_counter = 0;
	PPU* ppu = nullptr;
	int tail_pos = 0;
	int head_pos = 0;
	bool empty = true;
	bool full = false;

	void push(FIFOPixel pixel);
	void connectToPPU(PPU* pPPU);
	int elemCount();
	FIFOPixel pop();
	void popBy(int count);
	void reset();
private:
};