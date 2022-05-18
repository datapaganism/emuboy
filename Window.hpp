#pragma once
#include "SDL.h"



class Window
{
public:
	Window(int width, int height, int scaling, const char* title, bool shownOnStart);
	~Window();

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


	int mWindowID = -1;
	SDL_Renderer* mRenderer = nullptr;
	SDL_Texture* mTexture = nullptr;

protected:
	SDL_Window* mWindow = nullptr;
	
	const char* title = NULL;

	int mWidth = 0;
	int mHeight = 0;
	int mScaling = 0;

	bool mMouseFocus = 0;
	bool mKeyboardFocus = 0;
	bool mFullScreen = 0;
	bool mMinimized = 0;
	bool mShown = 0;
};