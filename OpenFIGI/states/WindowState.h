#pragma once

#define ID_BUTTON_FILE 1001  // Button ID for "File" button
#define ID_BUTTON_SAVE 1002  // Button ID for "Save" button

#include <Windows.h>
#include "FileState.h"
#include "../Request.h"

class FileState;
class Request;

class WindowState
{
public:
	WindowState(HINSTANCE hInstance, int nCmdShow, FileState& fileState, Request& request);
	~WindowState();

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int RunMsgLoop();

	void get_open_path();
	void get_save_path();

	HWND GetHWND() const;
	

private:
	int PARENT_WINDOW_HEIGHT = 400;
	int PARENT_WINDOW_WIDTH = 650;
	int nWidth;
	int nHeight;

	HINSTANCE hInstance;	// A handle to an instance.This is the base address of the module in memory.
	int nCmdShow;
	HWND hwnd;				// A handle to a window.
	
	std::string m_open_path;
	std::string m_save_path;

	FileState& fileState;
	Request& request;





	bool RegisterWindowClass();
	bool CreateMainWindow();
};