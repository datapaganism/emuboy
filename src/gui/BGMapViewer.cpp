#pragma once
#include "BGMapViewer.hpp"
#include "../../core/include/config.hpp"
#include <sstream>



BGMapViewer::BGMapViewer(BUS* bus_ptr, int width, int height, int scaling, const char* title, bool shownOnStart) : Window(width, height, scaling, title, shownOnStart)
{
    this->bus_ptr = bus_ptr;
	this->framebuffer = std::make_unique<FramebufferPixel[]>(8 * 32 * 8 * 32);
}

void BGMapViewer::handleEvent(SDL_Event& e)
{
    if (e.window.windowID == this->window_id)
    {
        bool update_caption = false;
        switch (e.type)
        {
        case SDL_KEYDOWN:
        {
            int keyPressed = e.key.keysym.sym;
            if (keyPressed == SDLK_LEFTBRACKET)
            {
                update_caption = true;
                this->map_address_use_alternate = !map_address_use_alternate;
                break;
            }
            if (keyPressed == SDLK_RIGHTBRACKET)
            {
                update_caption = true;
                this->tile_address_use_alternate = !tile_address_use_alternate;
                break;
            }
        } break;
        };

        if (update_caption)
        {
            std::stringstream caption;
            caption << "MAP ADDR: ";
            if (map_address_use_alternate)
                caption << "9C00";
            else
                caption << "9800";

            caption << " TILE ADDR: ";
            if (tile_address_use_alternate)
                caption << "8800";
            else
                caption << "8900";


            SDL_SetWindowTitle(this->window, caption.str().c_str());
        }
    }
}
void BGMapViewer::updateState() {}
void BGMapViewer::updateRender()
{
        int scx = *bus_ptr->ppu.registers.scx;
        int scy = *bus_ptr->ppu.registers.scy;
        this->generateBGMapFramebuffer();
        SDL_UpdateTexture(this->texture, NULL, this->framebuffer.get(), 8 * 32 * sizeof(FramebufferPixel));
        SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);

        SDL_SetRenderDrawColor(this->renderer, 255, 0, 0, 255);

        // draw top line 
        
        int x_s_pos = scx * scaling; // x_start_position
        int x_e_pos = (scx + XRES) * scaling; // x_end_position
        int x_w_s_pos = 0 * scaling; // x_wrapping_start_position
        int x_w_e_pos = (XRES - (256 - scx)) * scaling; // x_wrapping_end_position

        int y_s_pos = scy * scaling; // y_start_position
        int y_e_pos = (scy + YRES) * scaling; // y_end_position
        int y_w_s_pos = 0 * scaling; // y_wrapping_start_position
        int y_w_e_pos = (YRES - (256 - scy)) * scaling; // y_wrapping_end_position


        //SDL_Rect rect;

        //rect.x = x_s_pos;
        //rect.y = y_s_pos;
        //rect.w = XRES * scaling;
        //rect.h = YRES * scaling;

        //SDL_RenderDrawRect(this->renderer, &rect);

        SDL_RenderDrawLine(this->renderer, x_s_pos, y_s_pos, x_e_pos, y_s_pos);
        if (scx + XRES > 256)
        {
            SDL_RenderDrawLine(this->renderer, x_w_s_pos, scy, x_w_e_pos, scy);
        }
      
        // draw bottom line
     /*   SDL_RenderDrawLine(this->renderer, x_s_pos, y_e_pos, x_e_pos, y_e_pos);
        if (scx + XRES > 256)
        {
            SDL_RenderDrawLine(this->renderer, x_w_s_pos, y_e_pos, x_w_e_pos, y_e_pos);
        }*/

        SDL_SetRenderDrawColor(this->renderer, 0, 0, 255, 255);

         
        // draw left line
        SDL_RenderDrawLine(this->renderer, x_s_pos, y_s_pos, x_s_pos, y_e_pos);
        if (scy + XRES > 256)
        {
            SDL_RenderDrawLine(this->renderer, x_w_s_pos, y_w_s_pos, x_w_s_pos, y_w_e_pos);
        }

        //SDL_RenderDrawLine(this->renderer, scx + XRES * scaling,scy, scx + XRES * scaling,scy+YRES*scaling);

}

void BGMapViewer::generateBGMapFramebuffer()
{
    int temp_framebuffer_buffer_itr = 0;
    FIFOPixel temp_pixel_buffer;

    Word start_tile_no_address = (map_address_use_alternate) ? 0x9C00 : 0x9800;
    Word start_memory_address = (tile_address_use_alternate) ? 0x8800 : 0x8000;

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
            
            Word tile_address = 0;

            // LCDC.4 = 1, $8000 addressing
            if (!tile_address_use_alternate)
            {
                tile_address = 0x8000 + (tilenumber * 16);
            }
            else
            {
                tile_address = (tilenumber > 127) ? (0x8800 + (tilenumber - 128) * 16) : (0x9000 + tilenumber * 16);
            }


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
