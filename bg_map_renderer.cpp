#include "bg_map_renderer.hpp"
#include "config.h"


BG_MAP_RENDERER::BG_MAP_RENDERER()
{
    SDL_Init(SDL_INIT_VIDEO);
    this->window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        8 * 32 * RES_SCALING,
        8 * 32 * RES_SCALING, SDL_WINDOW_SHOWN);

    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_TEXTUREACCESS_TARGET);
    SDL_SetRenderDrawColor(this->renderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);

    this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 8 * 32, 8 * 32);

    SDL_RenderClear(this->renderer);
    SDL_RenderPresent(this->renderer);
}

BG_MAP_RENDERER::~BG_MAP_RENDERER()
{
    SDL_DestroyTexture(this->texture);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void BG_MAP_RENDERER::render_vram_tiles(BUS* bus)
{
    SDL_RenderClear(this->renderer);

    //convert tile data into argb8888 pixels and then feed it into this function
    this->generate_bg_framebuffer(bus);
    SDL_UpdateTexture(this->texture, NULL, this->framebuffer.get(), 8 * 32 * sizeof(FRAMEBUFFER_PIXEL));
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);

    SDL_RenderPresent(this->renderer);
}

// 16 tiles wide
// by 24 tall

void BG_MAP_RENDERER::generate_bg_framebuffer(BUS* bus)
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
            Byte tilenumber = bus->get_memory(tile_map_cell_pointer, MEMORY_ACCESS_TYPE::debug);
            Word tile_address = bus->ppu.get_tile_address_from_number(tilenumber, PPU::background);
            TILE tile(bus,tile_address);
            FRAMEBUFFER_PIXEL tile_pixels_scanline_worth[8];
            
            for (int tile_y = 0; tile_y < 8; tile_y++)
            {
                for (int tile_x = 0; tile_x < 8; tile_x++)
                {
                    temp_pixel_buffer = (FIFO_pixel(tile.getPixelColour(tile_x,tile_y), 0, 0, 0));
                    tile_pixels_scanline_worth[tile_x] = bus->ppu.dmg_framebuffer_pixel_to_rgb(temp_pixel_buffer);
                }
                int offset = (8*x) + ((width * 8) * ((8 * y) + tile_y));
                memcpy(this->framebuffer.get() + offset, tile_pixels_scanline_worth, sizeof(FRAMEBUFFER_PIXEL) * 8);
            }
        }
    }
    //for (int i = 0; i < width * 8 * width; i++)
    //{
    //    Word tile_map_cell_pointer = start_tile_no_address + (1 * bg_map_tile_pointer_x) + (0x20 * bg_map_tile_pointer_y);
    //    Byte tilenumber = bus->get_memory(tile_map_cell_pointer, MEMORY_ACCESS_TYPE::debug);
    //    
    //    Word tile_address = bus->ppu.get_tile_address_from_number(tilenumber, PPU::background);
    //    if (tile_address == 0x9904)
    //    {
    //        tile_address = tile_address;
    //    }
    //    //offset by which scanline of the tile we need
    //    Word address = tile_address; + (2 * (scanlines % 8));
 
    //    data0 = bus->get_memory(address, MEMORY_ACCESS_TYPE::debug);
    //    data1 = bus->get_memory(address + 1, MEMORY_ACCESS_TYPE::debug);

    //    for (int i = 0; i < 8; i++)
    //    {
    //        // get colour of pixel
    //        int offset = (0b1 << (7 - i));
    //        bool bit0 = data0 & offset;
    //        bool bit1 = data1 & offset;
    //        Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

    //        //push to fifo
    //        temp_pixel_buffer = (FIFO_pixel(colour, 0, 0, 0));
    //        temp_framebuffer_buffer[static_cast<long long>(temp_framebuffer_buffer_itr++) + (width * 8 * static_cast<long long>(scanlines))] = bus->ppu.dmg_framebuffer_pixel_to_rgb(temp_pixel_buffer);
    //    }
    //    bg_map_tile_pointer_x++;
    //
    //    if (bg_map_tile_pointer_x >= width)
    //    {
    //        temp_framebuffer_buffer_itr = 0;
    //        bg_map_tile_pointer_x = 0;
    //        scanlines++;
    //    }
    //    if (scanlines % 8 == 0 && scanlines != 0)
    //    {
    //        bg_map_tile_pointer_y++;
    //    }
    //}
}


