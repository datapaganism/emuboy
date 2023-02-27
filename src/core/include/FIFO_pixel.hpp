#pragma once
#include "config.hpp"

enum FIFOPixelType
{
	bg,
	win,
	sprite
};

struct FIFOPixel
{
	Byte colour : 2;
	Byte palette : 3;
	bool sprite_priority;
	bool bg_priority;
	Byte type;

	FIFOPixel()
	{
		this->bg_priority = true;
		this->sprite_priority = true;
		this->colour = 0xFF;
		this->palette = 0xFF;
		this->type = 0xFF;
	};

	FIFOPixel(Byte colour, Byte palette, bool sprite_priority, bool bg_priority) : FIFOPixel()
	{
		this->colour = colour;
		this->palette = palette;
		this->sprite_priority = sprite_priority;
		this->bg_priority = bg_priority;
	}

	FIFOPixel(Byte colour, Byte palette, bool sprite_priority, bool bg_priority, Byte type) : FIFOPixel()
	{
		this->colour = colour;
		this->palette = palette;
		this->sprite_priority = sprite_priority;
		this->bg_priority = bg_priority;
		this->type = type;
	}
};