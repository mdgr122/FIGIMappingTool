#pragma once

#include <Windows.h>
#include <commdlg.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <json.hpp>
#include "WindowState.h"
#include "../utilities/utils.h"


class WindowState;

class FileState
{
public:
	FileState();
	~FileState();

	std::string get_open_path();
	std::string get_save_path();

	void read_file(std::string file_path);
	void save_file(nlohmann::json response, std::string save_path);
	void save_csv_file(std::vector<std::string> vec, std::string save_path);
	void clear_data();

	std::string format_string(const std::string& open_path);

	const HWND GetHWND() const;

	const std::vector<std::string>& GetVec() const;


	OPENFILENAME ofn_open;
	OPENFILENAME ofn_save;

private:
	wchar_t szFile[260];       // buffer for file name
	HWND hwnd;              // owner window
	HANDLE hf;              // file handle

	std::string m_open_path;
	std::string m_save_path;

	std::vector<std::string> m_input_vec;
	nlohmann::json m_response;


	std::shared_ptr<WindowState> hwnd_ptr;

};