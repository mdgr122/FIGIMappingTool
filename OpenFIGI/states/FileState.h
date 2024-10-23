#pragma once

#include <Windows.h>
#include <commdlg.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "WindowState.h"

class WindowState;

class FileState
{
public:
	FileState(HWND hwnd);
	~FileState();

	std::string get_path();
	void read_file();
	std::string WideToStr(const std::wstring& wstr);
	std::string format_string(const std::string& open_path);


	OPENFILENAME ofn;

private:
	wchar_t szFile[260];       // buffer for file name
	HWND hwnd;              // owner window
	HANDLE hf;              // file handle

	std::string m_open_path;

	std::vector<std::string> m_input_vec;

};