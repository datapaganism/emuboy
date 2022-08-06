#pragma once
#include "VRAMViewer.hpp"
#include "config.hpp"


VRAMViewer::VRAMViewer(BUS* bus_ptr, int width, int height, int scaling, const char* title, bool shownOnStart) : Window(width, height, scaling, title, shownOnStart)
{
    this->bus_ptr = bus_ptr;
	this->framebuffer = std::make_unique<FramebufferPixel[]>(8 * 16 * 8 * 24);
}

void VRAMViewer::handleEvent(SDL_Event& e) {}
void VRAMViewer::updateState() {}
void VRAMViewer::updateRender()
{
    this->generateVRAMFramebuffer();
    SDL_UpdateTexture(this->texture, NULL, this->framebuffer.get(), 8 * 16 * sizeof(FramebufferPixel));
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
}

void VRAMViewer::generateVRAMFramebuffer()
{
    int temp_framebuffer_buffer_itr = 0;
    FIFOPixel temp_pixel_buffer;
    Word start_memory_address = 0x8000;
    Byte tile_no_x = 0, tile_no_y = 0, scanline_y = 0, tile_x = 0;
    Byte data0 = 0, data1 = 0;
    Byte width = 16;

    for (int i = 0; i < width * 8 * 24; i++)
    {
        Word address = start_memory_address + (0x10 * tile_no_x) + (0x100 * tile_x) + (2 * (scanline_y % 8));

        data0 = this->bus_ptr->getMemory(address, eMemoryAccessType::ppu);
        data1 = this->bus_ptr->getMemory(address + 1, eMemoryAccessType::ppu);

        for (int i = 0; i < 8; i++)
        {
            // get colour of pixel
            int offset = (0b1 << (7 - i));
            bool bit0 = data0 & offset;
            bool bit1 = data1 & offset;
            Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

            //push to fifo
            temp_pixel_buffer = (FIFOPixel(colour, 0, 0, 0));
            this->framebuffer[static_cast<long long>(temp_framebuffer_buffer_itr++) + (width * 8 * static_cast<long long>(scanline_y))] = this->bus_ptr->ppu.dmgFramebufferPixelToRGB(temp_pixel_buffer);
        }
        tile_no_x++;
        if (temp_framebuffer_buffer_itr % (width * 8) == 0 && temp_framebuffer_buffer_itr != 0)
        {
            temp_framebuffer_buffer_itr = 0;
            tile_no_y++;
            scanline_y++;
        }
        if (temp_framebuffer_buffer_itr % (width * 8) == 0 && scanline_y % 8 == 0)
        {
            tile_x++;
        }
        if (tile_no_x >= width)
        {
            tile_no_x = tile_no_x;
            tile_no_x = 0;
        }
    }
}
