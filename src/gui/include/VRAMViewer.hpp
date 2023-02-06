#pragma once
#include <memory>
#include <iostream>

#include "Window.hpp"
#include "../../core/include/bus.hpp"


class VRAMViewer : public Window
{
public:
	VRAMViewer(BUS* bus_ptr,int width, int height, int scaling, const char* title, bool shownOnStart);
	std::unique_ptr<FramebufferPixel[]> framebuffer;

	void handleEvent(SDL_Event& e);
	void updateState();
	void updateRender();

	BUS* bus_ptr = nullptr;

	void generateVRAMFramebuffer();
private:

};