#pragma once

#include "BaseWindow.h"
#include <Windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include "FileState.h"
#include "../Request.h"
#include "../utilities/utils.h"
#include "../utilities/jsonparse.h"
#include <memory>

#define ID_BUTTON_FILE_PATH 1001  // Button ID for "File" button
#define ID_BUTTON_SAVE_PATH 1002  // Button ID for "Save" button
#define ID_BUTTON_SAVE 1003  // Button ID for "Save" button
#define ID_BUTTON_REQUEST 1004
#define ID_FILE_PATH 1005
#define ID_SAVE_PATH 1006
#define ID_STATIC_MSG 1007
#define ID_BUTTON_ABOUT 1008
#define ID_BUTTON_CLOSE 1009
#define ID_ABOUT_WINDOW 1010
#define WM_APP_CHILD_CLOSED (WM_APP + 1)
#define ID_STATIC_ABOUT_MSG 1011


class FileState;
class Request;

class WindowState : public BaseWindow<WindowState>
{
public:
	WindowState(HWND hParent, FileState& fileState, Request& request, JsonParse& jsonParse);
	~WindowState();


	PCWSTR  ClassName() const override;
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL CreateParentWindow();
	BOOL CreateChildWindow();



	//static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//static LRESULT CALLBACK AboutWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	//int RunMsgLoop();

	void get_open_path();
	void get_save_path();
	void make_request();
	void save_output();
	void save_output_csv();



	bool save_ftype_csv();
	int get_parent_middle_width(int parent_width, int child_width);

	//std::wstring stringToWideString(const std::string& str);
	//std::string WideToStr(const std::wstring& wstr);

	HWND GetHWND() const { return m_hParent; };
	std::unique_ptr<WindowState> childWindow;

private:
	HINSTANCE hInstance;	// A handle to an instance.This is the base address of the module in memory.
	int nCmdShow;

	HWND m_hParent;

	HWND childHwnd;

	HWND hwndFileButton;
	HWND hwndRequestButton;
	HWND hwndSaveButton;
	HWND hwndSaveButton2;
	HWND hwndFilePath;				// Handle for file path
	HWND hwndSavePath;				// Handle for file path
	HWND hwndWaitingMsg;
	HWND hwndAboutButton;
	HWND hwndAboutPopup;
	HWND hwndCloseButton;



	int PARENT_WINDOW_HEIGHT = 250;
	int PARENT_WINDOW_WIDTH = 650;

	int nWidth;
	int nHeight;

	HBRUSH hbrBackground;
	HBRUSH hBackground;

	std::string m_open_path;
	std::string m_save_path;

	FileState& fileState;
	Request& request;
	JsonParse& jsonParse;

	//HWND hwnd;						// A handle to a window.
	//HWND m_hwnd;
	//HWND hwndFileButton;
	//HWND hwndRequestButton;
	//HWND hwndSaveButton;
	//HWND hwndSaveButton2;
	//HWND hwndFilePath;				// Handle for file path
	//HWND hwndSavePath;				// Handle for file path
	//HWND hwndWaitingMsg;
	//HWND hwndAboutButton;
	//HWND hwndAboutPopup;
	//
	////HDC hdcEdit;				// Handle to the device context of the control
	////HDC hdcStatic;				// Handle to the device context of the control

	//HBRUSH hbrBackground;
	//HBRUSH hBackground;

	//std::string m_open_path;
	//std::string m_save_path;

	//FileState& fileState;
	//Request& request;
	//JsonParse& jsonParse;





	//bool RegisterWindowClass();
	//bool RegisterAboutWindowClass(HINSTANCE hInstance);
	//bool CreateMainWindow();
};