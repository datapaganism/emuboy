#include "EmulatorWindow.hpp"

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
            }
            enum eJoypadButtons pressed = keyToEnum(keyPressed);
            if (pressed != UNKNOWN)
                this->pressButton(pressed);
        } break;
        case SDL_KEYUP:
        {
            int keyPressed = e.key.keysym.sym;
            enum eJoypadButtons pressed = keyToEnum(keyPressed);
            if (pressed != UNKNOWN)
                this->depressButton(pressed);
        } break;
        };
    }
}

void EmulatorWindow::updateState()
{
    this->cycleSystemOneFrame();
}

void EmulatorWindow::updateRender()
{
    if (this->ppu.lcdEnabled())
    {
        SDL_UpdateTexture(this->texture, NULL, this->framebuffer.get(), XRES * sizeof(FramebufferPixel));
        SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
    }
}


