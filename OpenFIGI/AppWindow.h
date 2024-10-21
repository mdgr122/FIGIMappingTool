#pragma once

#include <Windows.h>

class AppWindow
{
public:
	AppWindow(HINSTANCE hInstance, int nCmdShow);
	~AppWindow();

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int RunMsgLoop();





private:
	HINSTANCE hInstance;	// A handle to an instance.This is the base address of the module in memory.
	int nCmdShow;
	HWND hwnd;				// A handle to a window.

	bool RegisterWindowClass();
	bool CreateMainWindow();
};