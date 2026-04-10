#pragma once

#include "BaseWindow.h"
#include <Windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include "FileState.h"
#include "../core/OpenFigiClient.h"
#include "../utilities/Utils.h"
#include <memory>
#include <mutex>
#include <thread>

#define ID_BUTTON_FILE_PATH 1001
#define ID_BUTTON_SAVE_PATH 1002
#define ID_BUTTON_SAVE 1003
#define ID_BUTTON_REQUEST 1004
#define ID_FILE_PATH 1005
#define ID_SAVE_PATH 1006
#define ID_STATIC_MSG 1007
#define ID_BUTTON_ABOUT 1008
#define ID_BUTTON_CLOSE 1009
#define ID_ABOUT_WINDOW 1010
#define ID_STATIC_ABOUT_MSG 1011
#define ID_EDIT_APIKEY 1012
#define WM_APP_CHILD_CLOSED (WM_APP + 1)
#define WM_MAKE_REQUEST_COMPLETE (WM_APP + 2)
#define WM_MAKE_REQUEST_FAILED (WM_APP + 3)
#define WM_SAVE_COMPLETE (WM_USER + 3)

class FileState;

class WindowState : public BaseWindow<WindowState> {
public:
  WindowState(HWND hParent, FileState& fileState, figi::OpenFigiClient* client);
  ~WindowState();

  PCWSTR ClassName() const override;
  LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  BOOL CreateParentWindow();
  BOOL CreateAboutWindow();

  HWND GetHWND() const { return m_hwndParent; }
  std::unique_ptr<WindowState> aboutWindow;

private:
  void get_open_path();
  void get_save_path();
  void start_request_thread();
  void start_save_thread();
  void do_request();
  void do_save();

  // Parses input lines into MappingJob objects (replaces old Request::GetIdentifierType).
  std::vector<figi::MappingJob> parse_input_lines(const std::vector<std::string>& lines);
  // Ticker dot-notation fixup (carried over from original).
  static void process_ticker(std::string& str);

  bool is_csv_save_path() const;
  int center_x(int child_width) const { return (PARENT_WINDOW_WIDTH - child_width) / 2; }

  // Thread safety: protects m_results and m_error_msg.
  std::mutex m_results_mutex;
  std::vector<figi::MappingResult> m_results;
  std::string m_error_msg;

  // Worker threads (joined in destructor).
  std::jthread m_request_thread;
  std::jthread m_save_thread;

  HWND m_hwndParent;

  HWND hwndFileButton{};
  HWND hwndRequestButton{};
  HWND hwndSaveButton{};
  HWND hwndSaveButton2{};
  HWND hwndFilePath{};
  HWND hwndSavePath{};
  HWND hwndWaitingMsg{};
  HWND hwndAboutButton{};
  HWND hwndCloseButton{};
  HWND hwndAboutText{};
  HWND m_hwndAPIKey{};
  HWND m_hwndAboutWindow{};

  HBRUSH m_hbrBackground;
  HBRUSH hbrEditBackground{};
  HFONT hFontAboutButtonText{};
  HFONT hFontAboutText{};
  HFONT hFontSmall{};

  int PARENT_WINDOW_HEIGHT = 250;
  int PARENT_WINDOW_WIDTH = 650;
  int nWidth{};
  int nHeight{};

  std::string m_open_path;
  std::string m_save_path;
  std::wstring m_apikey;

  FileState& m_fileState;
  figi::OpenFigiClient* m_client; // nullable (about window gets nullptr)
};
