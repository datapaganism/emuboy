#pragma once

#include "Window.hpp"
#include "../../core/include/bus.hpp"

class EmulatorWindow : public BUS , public Window 
{
public:
    void handleEvent(SDL_Event& e);
    void updateState();
    void updateRender();

    EmulatorWindow(const std::string rom_path, const std::string bios_path, int width, int height, int scaling, const char* title, bool shown_on_start) : BUS(rom_path, bios_path), Window(width, height, scaling, title, shown_on_start){};
};