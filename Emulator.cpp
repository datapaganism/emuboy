#include "Emulator.hpp"


Emulator::Emulator(const std::string rom_path, const std::string bios_path, int width, int height, int scaling, const char* title, bool shownOnStart) : Window(width, height, scaling, title, shownOnStart)
{
	//this->framebuffer = std::make_unique<FRAMEBUFFER_PIXEL[]>(XRES * YRES);
	this->bus.load_game_and_bios(rom_path,bios_path);
}

void Emulator::handleEvent(SDL_Event& e)
{
    //std::cout << "handleEvent - from Emulator class\n";
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
                    bus.pressButton(pressed);

                if (keyPressed == JoypadButtonsDebug::pause)
                    bus.paused = !bus.paused;
            } break;
            case SDL_KEYUP:
            {
                int keyPressed = e.key.keysym.sym;
                enum JoypadButtons pressed = keyToEnum(keyPressed);
                if (pressed != UNKNOWN)
                    bus.depressButton(pressed);
            } break;
            };
        }

        }
}

void Emulator::updateState()
{
    this->bus.cycle_system_one_frame();
}

void Emulator::updateRender()
{
    if (bus.ppu.lcd_enabled())
    {
        SDL_UpdateTexture(this->mTexture, NULL, bus.ppu.framebuffer.get(), XRES * sizeof(FRAMEBUFFER_PIXEL));
        SDL_RenderCopy(this->mRenderer, this->mTexture, NULL, NULL);
    }
}

BUS* Emulator::getBusPtr()
{
    return &this->bus;
}
