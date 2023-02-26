#include "EmulatorWindow.hpp"
#include "joypad.hpp"

void EmulatorWindow::handleEvent(SDL_Event& e)
{
    if (e.window.windowID == this->window_id)
    {
        switch (e.type)
        {
        case SDL_KEYDOWN:
        {
            int keyPressed = e.key.keysym.sym;
            if (keyPressed == SDLK_8)
            {
                ppu.incrementPalette();
                break;
            }
            if (keyPressed == SDLK_F5)
            {
                this->saveState();
                break;
            }if (keyPressed == SDLK_F1)
            {
                this->cpu.debug_toggle = !cpu.debug_toggle;
                break;
            }
            enum eJoypadButtons pressed = keyToEnum(keyPressed);
            if (pressed != UNKNOWN)
                this->pressButton(button_to_bit(pressed));
        } break;
        case SDL_KEYUP:
        {
            int keyPressed = e.key.keysym.sym;
            enum eJoypadButtons pressed = keyToEnum(keyPressed);
            if (pressed != UNKNOWN)
                this->depressButton(button_to_bit(pressed));
        } break;
        };
    }
}

void EmulatorWindow::updateState()
{
    this->cycleSystemOneFrame();
    //this->cycleSystemOneFrameByInstruction();
}

void EmulatorWindow::updateRender()
{
    if (this->ppu.lcdEnabled())
    {
        SDL_UpdateTexture(this->texture, NULL, this->framebuffer, XRES * sizeof(FramebufferPixel));
        SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
    }
}


