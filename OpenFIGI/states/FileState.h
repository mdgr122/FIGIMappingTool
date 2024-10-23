#pragma once

#include <Windows.h>
#include <commdlg.h>
#include "WindowState.h"

class WindowState;

class FileState
{
public:
	FileState(HWND hwnd);
	~FileState();


	OPENFILENAME ofn;

private:

	wchar_t szFile[260];       // buffer for file name
	//HWND hwnd;              // owner window
	HANDLE hf;              // file handle


	bool get_file();
};