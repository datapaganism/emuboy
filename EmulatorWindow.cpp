#include "EmulatorWindow.hpp"

void EmulatorWindow::handleEvent(SDL_Event& e)
{
    if (e.type == SDL_WINDOWEVENT && e.window.windowID == this->mWindowID)
    {
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_KEYDOWN:
            {
                int keyPressed = e.key.keysym.sym;
                enum JoypadButtons pressed = keyToEnum(keyPressed);
                if (pressed != UNKNOWN)
                    this->pressButton(pressed);
            } break;
            case SDL_KEYUP:
            {
                int keyPressed = e.key.keysym.sym;
                enum JoypadButtons pressed = keyToEnum(keyPressed);
                if (pressed != UNKNOWN)
                    this->depressButton(pressed);
            } break;
            };
        }
    }
}

void EmulatorWindow::updateState()
{
    this->cycle_system_one_frame();
}

void EmulatorWindow::updateRender()
{
    if (this->ppu.lcd_enabled())
    {
        SDL_UpdateTexture(this->mTexture, NULL, this->framebuffer.get(), XRES * sizeof(FRAMEBUFFER_PIXEL));
        SDL_RenderCopy(this->mRenderer, this->mTexture, NULL, NULL);
    }
}


