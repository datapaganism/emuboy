#pragma once
#include <memory>
#include <iostream>

#include "Window.hpp"
#include "Emulator.hpp"
#include "bus.hpp"


class BGMapViewer : public Window
{
public:
	BGMapViewer(BUS* bus_ptr,int width, int height, int scaling, const char* title, bool shownOnStart);
	std::unique_ptr<FRAMEBUFFER_PIXEL[]> framebuffer;

	void handleEvent(SDL_Event& e);
	void updateState();
	void updateRender();

	BUS* bus_ptr = nullptr;

	void generate_bg_map_framebuffer();
private:

};