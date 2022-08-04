#pragma once
#include "BGMapViewer.hpp"
#include "config.hpp"


BGMapViewer::BGMapViewer(BUS* bus_ptr, int width, int height, int scaling, const char* title, bool shownOnStart) : Window(width, height, scaling, title, shownOnStart)
{
    this->bus_ptr = bus_ptr;
	this->framebuffer = std::make_unique<FramebufferPixel[]>(8 * 32 * 8 * 32);
}

void BGMapViewer::handleEvent(SDL_Event& e) {}
void BGMapViewer::updateState() {}
void BGMapViewer::updateRender()
{
    this->generateBGMapFramebuffer();
    SDL_UpdateTexture(this->texture, NULL, this->framebuffer.get(), 8 * 32 * sizeof(FramebufferPixel));
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
}

void BGMapViewer::generateBGMapFramebuffer()
{
    int temp_framebuffer_buffer_itr = 0;
    FIFOPixel temp_pixel_buffer;

    Word start_tile_no_address = 0x9800;
    Word start_memory_address = 0x8000;

    //Byte tile_no_x = 0, tile_no_y = 0, scanline_y = 0, tile_x = 0;
    Byte data0 = 0, data1 = 0;
    Byte width = 32;


    /// <summary>
    /// 0-32
    /// </summary>
    Byte bg_map_tile_pointer_x = 0;
    /// <summary>
    /// 0-32
    /// </summary>
    Byte bg_map_tile_pointer_y = 0;

    // 0 - (32*8)
    int scanlines = 0;

    for (int y = 0; y < width; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Word tile_map_cell_pointer = start_tile_no_address + (1 * x) + (0x20 * y);
            Byte tilenumber = this->bus_ptr->getMemory(tile_map_cell_pointer, eMemoryAccessType::debug);
            Word tile_address = this->bus_ptr->ppu.getTileAddressFromNumber(tilenumber, PPU::background);
            Tile tile(this->bus_ptr, tile_address);
            FramebufferPixel tile_pixels_scanline_worth[8];

            for (int tile_y = 0; tile_y < 8; tile_y++)
            {
                for (int tile_x = 0; tile_x < 8; tile_x++)
                {
                    temp_pixel_buffer = (FIFOPixel(tile.getPixelColour(tile_x, tile_y), 0, 0, 0));
                    tile_pixels_scanline_worth[tile_x] = this->bus_ptr->ppu.dmgFramebufferPixelToRGB(temp_pixel_buffer);
                }
                int offset = (8 * x) + ((width * 8) * ((8 * y) + tile_y));
                memcpy(this->framebuffer.get() + offset, tile_pixels_scanline_worth, sizeof(FramebufferPixel) * 8);
            }
        }
    }
}
