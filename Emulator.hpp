#pragma once
#include <memory>
#include <iostream>

#include "Window.hpp"
#include "bus.hpp"
#include <memory>

class Emulator : public Window
{
public:
	Emulator(const std::string rom_path, const std::string bios_path, int width, int height, int scaling, const char* title, bool shownOnStart);
	std::unique_ptr<FRAMEBUFFER_PIXEL[]> framebuffer;
	
	void handleEvent(SDL_Event& e);
	void updateState();
	void updateRender();

	BUS* getBusPtr();
	BUS bus;
private:
	
};