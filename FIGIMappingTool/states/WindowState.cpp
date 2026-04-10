#include "WindowState.h"
#include <sstream>
#include <format>
#include <commctrl.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' "                                                          \
                        "name='Microsoft.Windows.Common-Controls' "                                                    \
                        "version='6.0.0.0' "                                                                           \
                        "processorArchitecture='*' "                                                                   \
                        "publicKeyToken='6595b64144ccf1df' "                                                           \
                        "language='*'\"")

WindowState::WindowState(HWND hParent, FileState& fileState, figi::OpenFigiClient* client)
    : m_hwndParent(hParent), m_hbrBackground(CreateSolidBrush(RGB(255, 255, 255))), m_fileState(fileState),
      m_client(client), aboutWindow(nullptr), m_hwndAboutWindow(nullptr) {
  nWidth = GetSystemMetrics(SM_CXSCREEN);
  nHeight = GetSystemMetrics(SM_CYSCREEN);

  hbrEditBackground = CreateSolidBrush(RGB(240, 240, 240));
  hFontAboutButtonText = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
  hFontAboutText = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
  hFontSmall = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

WindowState::~WindowState() {
  // jthreads auto-join, but we request stop first.
  if (m_request_thread.joinable())
    m_request_thread.request_stop();
  if (m_save_thread.joinable())
    m_save_thread.request_stop();

  auto delete_gdi = [](auto& obj) {
    if (obj) {
      DeleteObject(obj);
      obj = nullptr;
    }
  };
  delete_gdi(m_hbrBackground);
  delete_gdi(hbrEditBackground);
  delete_gdi(hFontAboutButtonText);
  delete_gdi(hFontAboutText);
  delete_gdi(hFontSmall);
}

PCWSTR WindowState::ClassName() const {
  return L"FIGIMappingTool";
}

LRESULT WindowState::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_COMMAND: {
    if (LOWORD(wParam) == ID_BUTTON_FILE_PATH) {
      get_open_path();
      break;
    } else if (LOWORD(wParam) == ID_BUTTON_SAVE_PATH) {
      get_save_path();
      break;
    } else if (LOWORD(wParam) == ID_BUTTON_SAVE) {
      SetWindowText(hwndWaitingMsg, L"");
      if (m_save_path.empty()) {
        SetWindowText(hwndWaitingMsg, L"Save Path Empty!");
        break;
      }
      {
        std::lock_guard lock(m_results_mutex);
        if (m_results.empty()) {
          SetWindowText(hwndWaitingMsg, L"Nothing to Save");
          break;
        }
      }
      SetWindowText(hwndWaitingMsg, L"Saving...");
      UpdateWindow(hwndWaitingMsg);
      EnableWindow(hwndSaveButton, FALSE);
      start_save_thread();
      break;
    } else if (LOWORD(wParam) == ID_BUTTON_REQUEST) {
      if (m_open_path.empty()) {
        SetWindowText(hwndWaitingMsg, L"Input Path Empty!");
        break;
      }
      EnableWindow(hwndRequestButton, FALSE);
      SetWindowText(hwndWaitingMsg, L"Processing...");
      UpdateWindow(hwndWaitingMsg);
      start_request_thread();
      break;
    } else if (LOWORD(wParam) == ID_BUTTON_ABOUT) {
      CreateAboutWindow();
    } else if (LOWORD(wParam) == ID_BUTTON_CLOSE) {
      DestroyWindow(m_hwnd);
      break;
    }

    if (HIWORD(wParam) == EN_KILLFOCUS) {
      HWND hEdit = (HWND)lParam;
      wchar_t buffer[256]{};
      GetWindowText(hEdit, buffer, 256);

      if (LOWORD(wParam) == ID_FILE_PATH) {
        m_open_path = Utils::wide_to_str(buffer);
      } else if (LOWORD(wParam) == ID_SAVE_PATH) {
        m_save_path = Utils::wide_to_str(buffer);
      } else if (LOWORD(wParam) == ID_EDIT_APIKEY) {
        m_apikey = buffer;
        if (m_client) {
          m_client->set_api_key(Utils::wide_to_str(m_apikey));
        }
      }
      break;
    }
    break;
  }
  case WM_MAKE_REQUEST_COMPLETE: {
    size_t count = 0;
    {
      std::lock_guard lock(m_results_mutex);
      count = m_results.size();
    }
    auto msg = std::format(L"Complete! ({} results)", count);
    SetWindowText(hwndWaitingMsg, msg.c_str());
    EnableWindow(hwndRequestButton, TRUE);
    break;
  }
  case WM_MAKE_REQUEST_FAILED: {
    std::string err;
    {
      std::lock_guard lock(m_results_mutex);
      err = m_error_msg;
    }
    auto wmsg = Utils::str_to_wide(err);
    SetWindowText(hwndWaitingMsg, wmsg.c_str());
    EnableWindow(hwndRequestButton, TRUE);
    break;
  }
  case WM_SAVE_COMPLETE: {
    SetWindowText(hwndWaitingMsg, L"File Saved!");
    EnableWindow(hwndSaveButton, TRUE);
    break;
  }
  case WM_ERASEBKGND: {
    HDC hdc = (HDC)wParam;
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    FillRect(hdc, &rc, m_hbrBackground);
    return 1;
  }
  case WM_NOTIFY: {
    LPNMHDR pnmhdr = (LPNMHDR)lParam;
    if (pnmhdr->idFrom == ID_STATIC_ABOUT_MSG && (pnmhdr->code == NM_CLICK || pnmhdr->code == NM_RETURN)) {
      PNMLINK pNMLink = (PNMLINK)lParam;
      ShellExecute(nullptr, L"open", pNMLink->item.szUrl, nullptr, nullptr, SW_SHOWNORMAL);
      return 0;
    }
    break;
  }
  case WM_APP_CHILD_CLOSED: {
    aboutWindow.reset();
    return 0;
  }
  case WM_CTLCOLOREDIT: {
    HWND hEdit = (HWND)lParam;
    HDC hdcEdit = (HDC)wParam;
    if (hEdit == hwndFilePath || hEdit == hwndSavePath) {
      SetTextColor(hdcEdit, RGB(0, 0, 0));
      SetBkColor(hdcEdit, RGB(169, 169, 169));
    }
    return (INT_PTR)hbrEditBackground;
  }
  case WM_CTLCOLORSTATIC: {
    HDC hdcStatic = (HDC)wParam;
    HWND hwndStatic = (HWND)lParam;
    if (GetDlgCtrlID(hwndStatic) == ID_STATIC_MSG) {
      SetBkMode(hdcStatic, TRANSPARENT);
      SetTextColor(hdcStatic, RGB(0, 0, 0));
    }
    return (INT_PTR)GetStockObject(WHITE_BRUSH);
  }
  case WM_CLOSE: {
    DestroyWindow(m_hwnd);
    if (m_hwndParent != nullptr) {
      InvalidateRect(m_hwndParent, nullptr, TRUE);
      UpdateWindow(m_hwndParent);
    }
    return 0;
  }
  case WM_DESTROY: {
    if (m_hwndParent == nullptr) {
      PostQuitMessage(0);
    } else {
      PostMessage(m_hwndParent, WM_APP_CHILD_CLOSED, 0, 0);
    }
    return 0;
  }
  default:
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
  }
  return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

BOOL WindowState::CreateParentWindow() {
  if (!Create(L"FIGIMappingTool", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX))
    return FALSE;

  int xPos = (nWidth - PARENT_WINDOW_WIDTH) / 2;
  int yPos = (nHeight - PARENT_WINDOW_HEIGHT) / 2;
  DWORD btn = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON;

  SetWindowPos(m_hwnd, nullptr, xPos, yPos, PARENT_WINDOW_WIDTH, PARENT_WINDOW_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);

  RECT pr;
  GetClientRect(m_hwnd, &pr);

  int abw = 36, abh = 14;
  int abx = pr.right - abw - 2;
  int aby = pr.bottom - abh - 2;

  hwndRequestButton = CreateWindow(L"BUTTON", L"REQUEST", btn, center_x(300), 80, 300, 50, m_hwnd,
                                   (HMENU)ID_BUTTON_REQUEST, GetModuleHandle(nullptr), nullptr);
  hwndFileButton = CreateWindow(L"BUTTON", L"File", btn, 550, 10, 50, 20, m_hwnd, (HMENU)ID_BUTTON_FILE_PATH,
                                GetModuleHandle(nullptr), nullptr);
  hwndSaveButton = CreateWindow(L"BUTTON", L"Save", btn, 550, 35, 50, 20, m_hwnd, (HMENU)ID_BUTTON_SAVE,
                                GetModuleHandle(nullptr), nullptr);
  hwndSaveButton2 = CreateWindow(L"BUTTON", L":", btn, 601, 35, 16, 20, m_hwnd, (HMENU)ID_BUTTON_SAVE_PATH,
                                 GetModuleHandle(nullptr), nullptr);
  hwndAboutButton = CreateWindow(L"BUTTON", L"About", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT, abx, aby, abw, abh,
                                 m_hwnd, (HMENU)ID_BUTTON_ABOUT, GetModuleHandle(nullptr), nullptr);

  hwndFilePath = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
                              10, 10, 530, 20, m_hwnd, (HMENU)ID_FILE_PATH, GetModuleHandle(nullptr), nullptr);
  hwndSavePath = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
                              10, 35, 530, 20, m_hwnd, (HMENU)ID_SAVE_PATH, GetModuleHandle(nullptr), nullptr);

  // API key field — empty by default, user must enter their own.
  m_hwndAPIKey = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
                              center_x(PARENT_WINDOW_WIDTH - 450), 194, PARENT_WINDOW_WIDTH - 450, 16, m_hwnd,
                              (HMENU)ID_EDIT_APIKEY, GetModuleHandle(nullptr), nullptr);

  hwndWaitingMsg = CreateWindow(L"STATIC", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER, center_x(300), 140, 300,
                                40, m_hwnd, (HMENU)ID_STATIC_MSG, GetModuleHandle(nullptr), nullptr);

  SendMessage(hwndAboutButton, WM_SETFONT, (WPARAM)hFontAboutButtonText, TRUE);
  SendMessage(m_hwndAPIKey, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

  // Set placeholder text for API key field.
  SendMessage(m_hwndAPIKey, EM_SETCUEBANNER, FALSE, (LPARAM)L"Enter API key (optional)");

  return TRUE;
}

BOOL WindowState::CreateAboutWindow() {
  if (aboutWindow)
    return TRUE;

  INITCOMMONCONTROLSEX icex{};
  icex.dwSize = sizeof(icex);
  icex.dwICC = ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
  InitCommonControlsEx(&icex);

  aboutWindow = std::make_unique<WindowState>(m_hwnd, m_fileState, nullptr);

  RECT parentRect;
  GetWindowRect(m_hwnd, &parentRect);

  int childWidth = 400;
  int childHeight = 140;
  int childX = parentRect.left + (PARENT_WINDOW_WIDTH - childWidth) / 2;
  int childY = parentRect.top + (PARENT_WINDOW_HEIGHT - childHeight) / 2;

  const wchar_t* aboutText = L"FIGIMappingTool v2.0 (x64)\r\n\r\n"
                             L"A lightweight tool for retrieving FIGI mappings from OpenFIGI's API "
                             L"using various equity identifiers.\r\n\r\n"
                             L"License: MIT License\r\n"
                             L"© 2026 by Michael Dakin\r\n"
                             L"<A HREF=\"https://github.com/mdgr122/FIGIMappingTool\">GitHub Repository</A>";

  DWORD childStyle = WS_POPUP | WS_BORDER | WS_VISIBLE;
  if (aboutWindow->Create(L"", childStyle, 0, 50, 50, childWidth, childHeight, m_hwnd)) {
    m_hwndAboutWindow = aboutWindow->Window();
    SetWindowPos(m_hwndAboutWindow, HWND_TOP, childX, childY, childWidth, childHeight, SWP_NOSIZE | SWP_NOACTIVATE);

    int btnW = 16, btnH = 14;
    hwndCloseButton =
        CreateWindow(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_CENTER | BS_PUSHBUTTON, childWidth - btnW, -1, btnW,
                     btnH, m_hwndAboutWindow, (HMENU)ID_BUTTON_CLOSE, GetModuleHandle(nullptr), nullptr);

    hwndAboutText =
        CreateWindow(WC_LINK, aboutText, WS_CHILD | WS_VISIBLE | SS_LEFT, 2, 10, childWidth - 2, childHeight - 20,
                     m_hwndAboutWindow, (HMENU)ID_STATIC_ABOUT_MSG, GetModuleHandle(nullptr), nullptr);

    SendMessage(hwndAboutText, WM_SETFONT, (WPARAM)hFontAboutText, TRUE);
    return TRUE;
  }

  aboutWindow.reset();
  m_hwndAboutWindow = nullptr;
  return FALSE;
}

void WindowState::get_open_path() {
  m_open_path = m_fileState.get_open_path();
  SetWindowText(hwndFilePath, Utils::str_to_wide(m_open_path).c_str());
}

void WindowState::get_save_path() {
  m_save_path = m_fileState.get_save_path();
  SetWindowText(hwndSavePath, Utils::str_to_wide(m_save_path).c_str());
}

// ─── Worker threads ─────────────────────────────────────────────────────────

void WindowState::start_request_thread() {
  // If a previous request thread is still running, let it finish.
  if (m_request_thread.joinable()) {
    m_request_thread.request_stop();
    m_request_thread.join();
  }

  m_request_thread = std::jthread([this](std::stop_token) { do_request(); });
}

void WindowState::start_save_thread() {
  if (m_save_thread.joinable()) {
    m_save_thread.request_stop();
    m_save_thread.join();
  }

  m_save_thread = std::jthread([this](std::stop_token) { do_save(); });
}

void WindowState::do_request() {
  if (!m_client) {
    PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
    return;
  }

  // 1. Read the input file.
  m_fileState.clear_data();
  m_fileState.read_file(m_open_path);

  // 2. Parse input lines into MappingJobs.
  auto jobs = parse_input_lines(m_fileState.get_lines());
  if (jobs.empty()) {
    std::lock_guard lock(m_results_mutex);
    m_error_msg = "No valid identifiers found in file";
    PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
    return;
  }

  // 3. Execute the mapping request.
  auto result = m_client->map(std::move(jobs));

  if (result) {
    std::lock_guard lock(m_results_mutex);
    m_results = std::move(*result);
    m_error_msg.clear();
    PostMessage(m_hwnd, WM_MAKE_REQUEST_COMPLETE, 0, 0);
  } else {
    std::lock_guard lock(m_results_mutex);
    m_results.clear();
    m_error_msg = std::format("Error: {}", result.error().message);
    PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
  }
}

void WindowState::do_save() {
  std::vector<figi::MappingResult> results_copy;
  {
    std::lock_guard lock(m_results_mutex);
    results_copy = m_results;
  }

  if (is_csv_save_path()) {
    auto csv = figi::export_csv(results_copy);
    m_fileState.save_text_file(csv, m_save_path);
  } else {
    auto json = figi::export_json(results_copy);
    m_fileState.save_json_file(json, m_save_path);
  }

  PostMessage(m_hwnd, WM_SAVE_COMPLETE, 0, 0);
}

bool WindowState::is_csv_save_path() const {
  return m_save_path.size() >= 4 && m_save_path.compare(m_save_path.size() - 4, 4, ".csv") == 0;
}

// ─── Input parsing ──────────────────────────────────────────────────────────

std::vector<figi::MappingJob> WindowState::parse_input_lines(const std::vector<std::string>& lines) {
  std::vector<figi::MappingJob> jobs;
  jobs.reserve(lines.size());

  for (const auto& line : lines) {
    std::istringstream iss(line);
    std::string base_id, context;

    if (!(iss >> base_id))
      continue;

    // Read optional second token.
    if (iss >> context) {
      // "SPX Index" → append " Index" to the base identifier.
      if (context == "Index") {
        base_id += " Index";
        context.clear();
      }
    }

    // Detect the identifier type.
    auto detected = figi::OpenFigiClient::detect_id_type(base_id);
    if (!detected)
      continue; // skip unrecognizable identifiers

    figi::MappingJob job{
        .idType = *detected,
        .idValue = base_id,
        .includeUnlistedEquities = true,
    };

    // Apply ticker dot-notation fixup.
    if (job.idType == figi::IdType::TICKER) {
      process_ticker(job.idValue);
    }

    // Apply context filter based on length heuristic.
    if (!context.empty()) {
      if (context.size() == 2)
        job.exchCode = context;
      else if (context.size() == 3)
        job.currency = context;
      else if (context.size() == 4)
        job.micCode = context;
    }

    jobs.push_back(std::move(job));
  }

  return jobs;
}

void WindowState::process_ticker(std::string& str) {
  // Separate " Index" suffix if present.
  std::string index_suffix;
  if (auto pos = str.find(" Index"); pos != std::string::npos) {
    index_suffix = str.substr(pos);
    str.erase(pos);
  }

  // Transform dot notation:
  //   "BRK.B" → "BRK/B"  (1 char after dot → slash)
  //   "BRK.AB" → "BRK-A" (>1 chars after dot → dash + first char only)
  //   "IBM."   → "IBM"    (trailing dot → remove)
  if (auto dot = str.find('.'); dot != std::string::npos) {
    size_t after = str.size() - dot - 1;
    if (after == 0) {
      str.erase(dot, 1);
    } else if (after == 1) {
      str[dot] = '/';
    } else {
      str.replace(dot, after + 1, "-" + str.substr(dot + 1, 1));
    }
  }

  str += index_suffix;
}
