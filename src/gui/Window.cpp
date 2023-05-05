#include "Window.hpp"
#include "Window.hpp"
#include <iostream>
#include <sstream>    
#include "../../core/include/config.hpp"


Window::Window(int width, int height, int scaling, const char* title, bool shown_on_start)
{
	this->title = title;
	this->width = ((width != NULL) ? width : DEFAULT_SCREEN_WIDTH);
	this->height = ((height != NULL) ? height : DEFAULT_SCREEN_HEIGHT);
	this->scaling = scaling;

	start_width = width;
	start_height = height;

	this->window = SDL_CreateWindow(this->title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->width * this->scaling, this->height * this->scaling, ((shown_on_start) ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN) | SDL_WINDOW_RESIZABLE);
	if (this->window != nullptr)
	{
		this->mouse_focus = true;
		this->keyboard_focus = true;
		
		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_TEXTUREACCESS_TARGET);
		if (this->renderer == nullptr)
		{
			std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
			SDL_DestroyWindow(this->window);
			this->window = nullptr;
		}
		else
		{
			SDL_SetRenderDrawColor(this->renderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);
			this->window_id = SDL_GetWindowID(this->window);
			this->shown = shown_on_start;
		}

		this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, this->width, this->height);
		if (this->texture == nullptr)
		{
			std::cout << "Texture could not be created! SDL Error: " << SDL_GetError() << "\n";
			SDL_DestroyRenderer(this->renderer);
			SDL_DestroyWindow(this->window);
			this->renderer = nullptr;
			this->window = nullptr;
		}
	}
	else
	{
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
	}
}

Window::~Window()
{
	if (this->texture != nullptr)
		SDL_DestroyTexture(this->texture);

	if (this->renderer != nullptr)
		SDL_DestroyRenderer(this->renderer);

	if (this->window != nullptr)
		SDL_DestroyWindow(this->window);

	std::cout << "Window deconstructor called\n";
}

void Window::handleWindowEvent(SDL_Event& e)
{
	if (e.type == SDL_WINDOWEVENT && e.window.windowID == this->window_id)
	{
		bool update_caption = false;
		switch (e.window.event)
		{
		case SDL_WINDOWEVENT_SHOWN: { this->shown = true; SDL_RenderPresent(this->renderer); } break;
		
		case SDL_WINDOWEVENT_HIDDEN: { this->shown = false; } break;
		
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			this->width = e.window.data1;
			this->height = e.window.data2;
			SDL_RenderPresent(this->renderer);
		} break;

		case SDL_WINDOWEVENT_EXPOSED: { SDL_RenderPresent(this->renderer); } break;
		case SDL_WINDOWEVENT_ENTER:
		{
			this->mouse_focus = true;
			update_caption = true;
		} break;

		case SDL_WINDOWEVENT_LEAVE:
		{
			this->mouse_focus = false;
			update_caption = true;
		} break;

		case SDL_WINDOWEVENT_FOCUS_GAINED:
		{
			this->keyboard_focus = true;
			update_caption = true;
		} break;

		case SDL_WINDOWEVENT_FOCUS_LOST:
		{
			this->keyboard_focus = false;
			update_caption = true;
		} break;

		case SDL_WINDOWEVENT_MINIMIZED: { this->minimized = true; } break;

		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED: { this->minimized = false; } break;

		case SDL_WINDOWEVENT_CLOSE:
		{
			// close main window
			if (e.window.windowID == 1)
				this->shown = false;

			SDL_HideWindow(this->window);
		} break;
		default: break;
		}

		if (update_caption)
		{
			updateCaption();
		}
	}
}



void  Window::focus()
{
	if (!this->shown)
	{
		SDL_ShowWindow(this->window);
	}
	SDL_RaiseWindow(this->window);
}

void  Window::render()
{
	if (!this->minimized && this->shown)
	{
		SDL_SetRenderDrawColor(this->renderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);
		SDL_RenderClear(this->renderer);

		this->updateRender();

		SDL_RenderPresent(this->renderer);
	}
}

//
// Getters Section
//


int Window::getWidth()
{
	return this->width;
}

int Window::getHeight()
{
	return this->height;
}

bool Window::hasMouseFocus()
{
	return this->mouse_focus;
}

bool Window::hasKeyboardFocus()
{
	return this->keyboard_focus;
}

bool Window::isMinimized()
{
	return this->minimized;
}

bool Window::isShown()
{
	return this->shown;
}

bool Window::initSuccess()
{
	return this->window != nullptr && this->renderer != nullptr;;
}

void Window::updateCaption()
{
	std::stringstream caption;
	caption << this->title << " - ID: " << this->window_id << " MouseFocus:" << ((this->mouse_focus) ? "On" : "Off") << " KeyboardFocus:" << ((this->keyboard_focus) ? "On" : "Off");
	SDL_SetWindowTitle(this->window, caption.str().c_str());
}


void Window::updateScaling(bool scale_up)
{
	(scale_up) ? scaling++ : scaling--;

	if (scaling < scaling_min)
		scaling = scaling_min;
	else
	if (scaling > scaling_max)
		scaling = scaling_max;

	SDL_SetWindowSize(window, start_width* scaling, start_height* scaling);
}