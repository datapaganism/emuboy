#pragma once
#include "BGMapViewer.hpp"
#include "config.hpp"


BGMapViewer::BGMapViewer(BUS* bus_ptr, int width, int height, int scaling, const char* title, bool shownOnStart) : Window(width, height, scaling, title, shownOnStart)
{
    this->bus_ptr = bus_ptr;
	this->framebuffer = std::make_unique<FRAMEBUFFER_PIXEL[]>(8 * 32 * 8 * 32);
}

void BGMapViewer::handleEvent(SDL_Event& e) {}
void BGMapViewer::updateState() {}
void BGMapViewer::updateRender()
{
    this->generate_bg_map_framebuffer();
    SDL_UpdateTexture(this->mTexture, NULL, this->framebuffer.get(), 8 * 32 * sizeof(FRAMEBUFFER_PIXEL));
    SDL_RenderCopy(this->mRenderer, this->mTexture, NULL, NULL);
}

void BGMapViewer::generate_bg_map_framebuffer()
{
    int temp_framebuffer_buffer_itr = 0;
    FIFO_pixel temp_pixel_buffer;

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
            Byte tilenumber = this->bus_ptr->get_memory(tile_map_cell_pointer, MEMORY_ACCESS_TYPE::debug);
            Word tile_address = this->bus_ptr->ppu.get_tile_address_from_number(tilenumber, PPU::background);
            TILE tile(this->bus_ptr, tile_address);
            FRAMEBUFFER_PIXEL tile_pixels_scanline_worth[8];

            for (int tile_y = 0; tile_y < 8; tile_y++)
            {
                for (int tile_x = 0; tile_x < 8; tile_x++)
                {
                    temp_pixel_buffer = (FIFO_pixel(tile.getPixelColour(tile_x, tile_y), 0, 0, 0));
                    tile_pixels_scanline_worth[tile_x] = this->bus_ptr->ppu.dmg_framebuffer_pixel_to_rgb(temp_pixel_buffer);
                }
                int offset = (8 * x) + ((width * 8) * ((8 * y) + tile_y));
                memcpy(this->framebuffer.get() + offset, tile_pixels_scanline_worth, sizeof(FRAMEBUFFER_PIXEL) * 8);
            }
        }
    }
}
