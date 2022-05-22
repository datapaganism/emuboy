#include "Window.hpp"
#include <iostream>
#include <sstream>    
#include "config.hpp"

Window::Window(int width, int height, int scaling, const char* title, bool shownOnStart)
{
	this->title = title;
	this->mWidth = ((width != NULL) ? width : DEFAULT_SCREEN_WIDTH);
	this->mHeight = ((height != NULL) ? height : DEFAULT_SCREEN_HEIGHT);
	this->mScaling = scaling;

	this->mWindow = SDL_CreateWindow(this->title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->mWidth * this->mScaling, this->mHeight * this->mScaling, ((shownOnStart) ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN) | SDL_WINDOW_RESIZABLE);
	if (this->mWindow != nullptr)
	{
		this->mMouseFocus = true;
		this->mKeyboardFocus = true;
		
		this->mRenderer = SDL_CreateRenderer(this->mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_TEXTUREACCESS_TARGET);
		if (this->mRenderer == nullptr)
		{
			std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
			SDL_DestroyWindow(this->mWindow);
			this->mWindow = nullptr;
		}
		else
		{
			SDL_SetRenderDrawColor(this->mRenderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);

			this->mWindowID = SDL_GetWindowID(this->mWindow);

			this->mShown = shownOnStart;
		}

		this->mTexture = SDL_CreateTexture(this->mRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, this->mWidth, this->mHeight);
		if (this->mTexture == nullptr)
		{
			std::cout << "Texture could not be created! SDL Error: " << SDL_GetError() << "\n";
			SDL_DestroyRenderer(this->mRenderer);
			SDL_DestroyWindow(this->mWindow);
			this->mRenderer = nullptr;
			this->mWindow = nullptr;
		}
	}
	else
	{
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
	}
}

Window::~Window()
{
	if (this->mTexture != nullptr)
		SDL_DestroyTexture(this->mTexture);

	if (this->mRenderer != nullptr)
		SDL_DestroyRenderer(this->mRenderer);

	if (this->mWindow != nullptr)
		SDL_DestroyWindow(this->mWindow);

	std::cout << "Window deconstructor called\n";
}

void Window::handleWindowEvent(SDL_Event& e)
{
	if (e.type == SDL_WINDOWEVENT && e.window.windowID == this->mWindowID)
	{
		bool updateCaption = false;
		switch (e.window.event)
		{
		case SDL_WINDOWEVENT_SHOWN: { this->mShown = true; SDL_RenderPresent(this->mRenderer); } break;
		
		case SDL_WINDOWEVENT_HIDDEN: { this->mShown = false; } break;
		
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			this->mWidth = e.window.data1;
			this->mHeight = e.window.data2;
			SDL_RenderPresent(this->mRenderer);
		} break;

		case SDL_WINDOWEVENT_EXPOSED: { SDL_RenderPresent(this->mRenderer); } break;
		case SDL_WINDOWEVENT_ENTER:
		{
			this->mMouseFocus = true;
			updateCaption = true;
		} break;

		case SDL_WINDOWEVENT_LEAVE:
		{
			this->mMouseFocus = false;
			updateCaption = true;
		} break;

		case SDL_WINDOWEVENT_FOCUS_GAINED:
		{
			this->mKeyboardFocus = true;
			updateCaption = true;
		} break;

		case SDL_WINDOWEVENT_FOCUS_LOST:
		{
			this->mKeyboardFocus = false;
			updateCaption = true;
		} break;

		case SDL_WINDOWEVENT_MINIMIZED: { this->mMinimized = true; } break;

		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED: { this->mMinimized = false; } break;

		case SDL_WINDOWEVENT_CLOSE:
		{
			// close main window
			if (e.window.windowID == 1)
				this->mShown = false;

			SDL_HideWindow(this->mWindow);
		} break;
		default: break;
		}

		//if (updateCaption)
		//{
		//	std::stringstream caption;
		//	caption << this->title << " - ID: " << this->mWindowID << " MouseFocus:" << ((this->mMouseFocus) ? "On" : "Off") << " KeyboardFocus:" << ((this->mKeyboardFocus) ? "On" : "Off");
		//	SDL_SetWindowTitle(this->mWindow, caption.str().c_str());
		//}
	}
}



void  Window::focus()
{
	if (!this->mShown)
	{
		SDL_ShowWindow(this->mWindow);
	}
	SDL_RaiseWindow(this->mWindow);
}

void  Window::render()
{
	if (!this->mMinimized)
	{
		SDL_SetRenderDrawColor(this->mRenderer, GB_PALLETE_BG_r, GB_PALLETE_BG_g, GB_PALLETE_BG_b, 0xFF);
		SDL_RenderClear(this->mRenderer);

		this->updateRender();

		SDL_RenderPresent(this->mRenderer);
	}
}

//
// Getters Section
//


int Window::getWidth()
{
	return this->mWidth;
}

int Window::getHeight()
{
	return this->mHeight;
}

bool Window::hasMouseFocus()
{
	return this->mMouseFocus;
}

bool Window::hasKeyboardFocus()
{
	return this->mKeyboardFocus;
}

bool Window::isMinimized()
{
	return this->mMinimized;
}

bool Window::isShown()
{
	return this->mShown;
}

bool Window::initSuccess()
{
	return this->mWindow != nullptr && this->mRenderer != nullptr;;
}