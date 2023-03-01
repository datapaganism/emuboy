#pragma once
#include "SDL.h"

// Adapted from https://lazyfoo.net/tutorials/SDL/36_multiple_windows/index.php
// Original guide offers a method of displaying multiple windows at the same time, since I wanted to show some behind the scenes parts of the system like a render of the VRAM, I needed more windows
// I had originally implemented the guide and realise that I needed to extend it in order to create a better system that suits my needs better.
// I went about creating a generic window class, this class would handle rendering, interactions with the mouse, keyboard etc. The aim was to create a layer of obstraction that could be used by other classes.
// This was achieved by adding virtual functions, 'handleEvent', 'updateState', and finally 'updateRender'.
// 'handleEvent' allows us to set new specific functionality for inputs, namely adding joypad controls on top the generic window controls.
// 'updateState' allows us to update the state of emulation or anything else that we have defined that requires change over time.
// 'updateRender' is used to set specify what is rendered to the screen.

// Once the window was complete, we needed a way to manage more than one window, therefore we had to build a window manager, the WindowManager does exactly this.
// Each specific window is created through multiple inheritance, by combining a logic class like the emulator and a window for us to render to, this also allows us to separate emulator from the rendering system. 
// A great departure of from the tutorial is the use of C++ Polymorphism, this is done by keeping an array of window elements, and iterating and updating each window's state.
// During the construction of the WindowManager, we generate the inherited type objects and add them to the array, due to the array's type it will treat them as Windows which provides a good seperation of logic.


class Window
{
public:
	Window(int width, int height, int scaling, const char* title, bool shown_on_start);
	virtual ~Window();

	void handleWindowEvent(SDL_Event& e);

	virtual void handleEvent(SDL_Event& e) = 0;
	virtual void updateState() = 0;
	virtual void updateRender() = 0;


	void focus();
	void render();

	int getWidth();
	int getHeight();

	bool hasMouseFocus();
	bool hasKeyboardFocus();
	bool isMinimized();
	bool isShown();

	bool initSuccess();

	virtual void updateCaption();


	int window_id = -1;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;

protected:
	SDL_Window* window = nullptr;
	
	const char* title = NULL;

	int width = 0;
	int height = 0;
	int scaling = 0;

	bool mouse_focus = false;
	bool keyboard_focus = false;
	bool fullscreen = false;
	bool minimized = false;
	bool shown = false;
};