#pragma once

#define ID_BUTTON_FILE_PATH 1001  // Button ID for "File" button
#define ID_BUTTON_SAVE_PATH 1002  // Button ID for "Save" button
#define ID_BUTTON_SAVE 1003  // Button ID for "Save" button
#define ID_BUTTON_REQUEST 1004
#define ID_FILE_PATH 1005
#define ID_SAVE_PATH 1006
#define ID_STATIC_MSG 1007
#define ID_BUTTON_ABOUT 1008
#define ID_CLOSE_BUTTON 1009
#define ID_ABOUT_WINDOW 1010

#include <Windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include "FileState.h"
#include "../Request.h"
#include "../utilities/utils.h"
#include "../utilities/jsonparse.h"

class FileState;
class Request;

class WindowState
{
public:
	WindowState(HINSTANCE hInstance, int nCmdShow, FileState& fileState, Request& request, JsonParse& jsonParse);
	~WindowState();

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK AboutWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	int RunMsgLoop();

	void get_open_path();
	void get_save_path();
	void make_request();
	void save_output();
	void save_output_csv();



	bool save_ftype_csv();

	int get_parent_middle_width(int parent_width, int child_width);

	//std::wstring stringToWideString(const std::string& str);
	//std::string WideToStr(const std::wstring& wstr);

	HWND GetHWND() const;
	

private:
	int PARENT_WINDOW_HEIGHT = 250;
	int PARENT_WINDOW_WIDTH = 650;
	int nWidth;
	int nHeight;

	HINSTANCE hInstance;	// A handle to an instance.This is the base address of the module in memory.
	int nCmdShow;

	HWND hwnd;						// A handle to a window.
	HWND m_hwnd;
	HWND hwndFileButton;
	HWND hwndRequestButton;
	HWND hwndSaveButton;
	HWND hwndSaveButton2;
	HWND hwndFilePath;				// Handle for file path
	HWND hwndSavePath;				// Handle for file path
	HWND hwndWaitingMsg;
	HWND hwndAboutButton;
	HWND hwndAboutPopup;
	
	//HDC hdcEdit;				// Handle to the device context of the control
	//HDC hdcStatic;				// Handle to the device context of the control

	HBRUSH hbrBackground;
	HBRUSH hBackground;

	std::string m_open_path;
	std::string m_save_path;

	FileState& fileState;
	Request& request;
	JsonParse& jsonParse;





	bool RegisterWindowClass();
	bool RegisterAboutWindowClass(HINSTANCE hInstance);
	bool CreateMainWindow();
};