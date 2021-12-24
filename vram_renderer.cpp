#include "vram_renderer.hpp"
#include "config.h"


VRAM_RENDERER::VRAM_RENDERER()
{
    SDL_Init(SDL_INIT_VIDEO);
    this->window = SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        8 * 16 * RES_SCALING,
        8 * 24 * RES_SCALING, SDL_WINDOW_SHOWN);

    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_TEXTUREACCESS_TARGET);
    SDL_SetRenderDrawColor(this->renderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);
    
    this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 8 * 16, 8 * 24);
    
    SDL_RenderClear(this->renderer);
    SDL_RenderPresent(this->renderer);
}

VRAM_RENDERER::~VRAM_RENDERER()
{
    SDL_DestroyTexture(this->texture);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void VRAM_RENDERER::render_vram_tiles(BUS *bus)
{
    SDL_RenderClear(this->renderer);


    //convert tile data into argb8888 pixels and then feed it into this function
    SDL_UpdateTexture(this->texture, NULL, generate_vram_framebuffer(bus).data(), 8 * 16 * sizeof(FRAMEBUFFER_PIXEL));
    SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);        

    SDL_RenderPresent(this->renderer);
}

// 16 tiles wide
// by 24 tall

std::array<FRAMEBUFFER_PIXEL, 8 * 16 * 8 * 24> VRAM_RENDERER::generate_vram_framebuffer(BUS* bus)
{
    std::array<FRAMEBUFFER_PIXEL, 8 * 16 * 8 * 24> temp_framebuffer_buffer;
    int temp_framebuffer_buffer_itr = 0;
    FIFO_pixel temp_pixel_buffer;
    Word start_memory_address = 0x8000;
    Byte tile_no_x = 0, tile_no_y = 0, y = 0;
    Byte data0 = 0, data1 = 0;
    Byte width = 16;

    for (int i = 0 ; i < width + 26 * 8; i++)
    {
        Word address = start_memory_address + (0x10 * tile_no_x) + (0x100 * tile_no_y) + (2 * (y % 8));

        data0 = bus->get_memory(address, MEMORY_ACCESS_TYPE::ppu);
        data1 = bus->get_memory(address +1, MEMORY_ACCESS_TYPE::ppu);

        for (int i = 0; i < 8; i++)
        {
            // get colour of pixel
            int offset = (0b1 << (7 - i));
            bool bit0 = data0 & offset;
            bool bit1 = data1 & offset;
            Byte colour = (((Byte)bit0 << 1) | (Byte)bit1);

            //push to fifo
            temp_pixel_buffer = (FIFO_pixel(colour, 0, 0, 0));
            //temp_framebuffer_buffer[temp_framebuffer_buffer_itr++] = bus->ppu.dmg_framebuffer_pixel_to_rgb(temp_pixel_buffer);
            temp_framebuffer_buffer[static_cast<long long>(temp_framebuffer_buffer_itr++) + (16*8 * static_cast<long long>(y))] = bus->ppu.dmg_framebuffer_pixel_to_rgb(temp_pixel_buffer);
        }
        tile_no_x++;
        if (tile_no_x >= width)
        {
            tile_no_x = 0;
            y++;
            address = 0;
        }
        if (y > 8) 
        {
            y = 0;
            tile_no_y++;
        }
    }
    return temp_framebuffer_buffer;



    
}


