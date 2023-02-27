#pragma once
#include <memory>
#include <iostream>

#include "Window.hpp"
#include "../../core/include/bus.hpp"



class BGMapViewer : public Window
{
public:
	BGMapViewer(BUS* bus_ptr,int width, int height, int scaling, const char* title, bool shownOnStart);
	std::unique_ptr<FramebufferPixel[]> framebuffer;

	void handleEvent(SDL_Event& e);
	void updateState();
	void updateRender();

	void updateCaption() override {};

	BUS* bus_ptr = nullptr;

	void generateBGMapFramebuffer();
private:
	bool map_address_use_alternate = false;
	bool tile_address_use_alternate = false;
};