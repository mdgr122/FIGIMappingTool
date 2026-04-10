#include "WindowState.h"
#include <algorithm>
#include <commctrl.h>
#include <format>
#include <shellapi.h>
#include <sstream>
#include <unordered_map>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(                                                                                                       \
    linker,                                                                                                            \
    "\"/manifestdependency:type='win32' "                                                                              \
    "name='Microsoft.Windows.Common-Controls' "                                                                        \
    "version='6.0.0.0' "                                                                                               \
    "processorArchitecture='*' "                                                                                       \
    "publicKeyToken='6595b64144ccf1df' "                                                                               \
    "language='*'\""                                                                                                   \
)

// ─── Construction / Destruction ─────────────────────────────────────────────

WindowState::WindowState(HWND hParent, FileState& fileState, figi::OpenFigiClient* client)
    : aboutWindow(nullptr)
    , m_hwndParent(hParent)
    , m_hwndAboutWindow(nullptr)
    , m_hbrBackground(CreateSolidBrush(RGB(255, 255, 255)))
    , m_fileState(fileState)
    , m_client(client)
{
    nWidth = GetSystemMetrics(SM_CXSCREEN);
    nHeight = GetSystemMetrics(SM_CYSCREEN);

    hbrEditBackground = CreateSolidBrush(RGB(240, 240, 240));
    hFontAboutButtonText = CreateFont(12,
                                      0,
                                      0,
                                      0,
                                      FW_NORMAL,
                                      FALSE,
                                      FALSE,
                                      FALSE,
                                      DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS,
                                      CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_SWISS,
                                      L"Segoe UI");
    hFontAboutText = CreateFont(16,
                                0,
                                0,
                                0,
                                FW_NORMAL,
                                FALSE,
                                FALSE,
                                FALSE,
                                DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_SWISS,
                                L"Segoe UI");
    hFontSmall = CreateFont(12,
                            0,
                            0,
                            0,
                            FW_NORMAL,
                            FALSE,
                            FALSE,
                            FALSE,
                            DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY,
                            DEFAULT_PITCH | FF_SWISS,
                            L"Segoe UI");
}

WindowState::~WindowState()
{
    if (m_request_thread.joinable())
        m_request_thread.request_stop();
    if (m_save_thread.joinable())
        m_save_thread.request_stop();

    auto del = [](auto& h)
    {
        if (h)
        {
            DeleteObject(h);
            h = nullptr;
        }
    };
    del(m_hbrBackground);
    del(hbrEditBackground);
    del(hFontAboutButtonText);
    del(hFontAboutText);
    del(hFontSmall);
}

PCWSTR WindowState::ClassName() const { return L"FIGIMappingTool"; }

// ─── Window Creation ────────────────────────────────────────────────────────

BOOL WindowState::CreateParentWindow()
{
    if (!Create(L"FIGIMappingTool", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX))
        return FALSE;

    SetWindowPos(m_hwnd,
                 nullptr,
                 (nWidth - PARENT_WINDOW_WIDTH) / 2,
                 (nHeight - PARENT_WINDOW_HEIGHT) / 2,
                 PARENT_WINDOW_WIDTH,
                 PARENT_WINDOW_HEIGHT,
                 SWP_NOZORDER | SWP_SHOWWINDOW);

    DWORD edStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL;
    DWORD btnStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON;
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // ── Row 0 (y=10): File path ─────────────────────────────────────────
    hwndFilePath = CreateWindow(L"EDIT", L"", edStyle, 10, 10, 530, 20, m_hwnd, (HMENU)ID_FILE_PATH, hInst, nullptr);
    hwndFileButton = CreateWindow(L"BUTTON",
                                  L"File",
                                  btnStyle,
                                  550,
                                  10,
                                  50,
                                  20,
                                  m_hwnd,
                                  (HMENU)ID_BUTTON_FILE_PATH,
                                  hInst,
                                  nullptr);

    // ── Row 1 (y=35): Save path ─────────────────────────────────────────
    hwndSavePath = CreateWindow(L"EDIT", L"", edStyle, 10, 35, 530, 20, m_hwnd, (HMENU)ID_SAVE_PATH, hInst, nullptr);
    hwndSaveButton = CreateWindow(L"BUTTON",
                                  L"Save",
                                  btnStyle,
                                  550,
                                  35,
                                  50,
                                  20,
                                  m_hwnd,
                                  (HMENU)ID_BUTTON_SAVE,
                                  hInst,
                                  nullptr);
    hwndSaveButton2 = CreateWindow(L"BUTTON",
                                   L":",
                                   btnStyle,
                                   601,
                                   35,
                                   16,
                                   20,
                                   m_hwnd,
                                   (HMENU)ID_BUTTON_SAVE_PATH,
                                   hInst,
                                   nullptr);

    // ── Row 2 (y=65): Mode radios ───────────────────────────────────────
    DWORD radioFirst = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP;
    DWORD radioRest = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON;
    hwndRadioMap = CreateWindow(L"BUTTON",
                                L"Map File",
                                radioFirst,
                                20,
                                65,
                                90,
                                20,
                                m_hwnd,
                                (HMENU)ID_RADIO_MAP,
                                hInst,
                                nullptr);
    hwndRadioSearch = CreateWindow(L"BUTTON",
                                   L"Search",
                                   radioRest,
                                   120,
                                   65,
                                   80,
                                   20,
                                   m_hwnd,
                                   (HMENU)ID_RADIO_SEARCH,
                                   hInst,
                                   nullptr);
    hwndRadioFilter = CreateWindow(L"BUTTON",
                                   L"Filter",
                                   radioRest,
                                   210,
                                   65,
                                   80,
                                   20,
                                   m_hwnd,
                                   (HMENU)ID_RADIO_FILTER,
                                   hInst,
                                   nullptr);
    SendMessage(hwndRadioMap, BM_SETCHECK, BST_CHECKED, 0);

    // ── Row 3 (y=92): Query ─────────────────────────────────────────────
    hwndQuery = CreateWindow(L"EDIT", L"", edStyle, 10, 92, 640, 20, m_hwnd, (HMENU)ID_EDIT_QUERY, hInst, nullptr);
    EnableWindow(hwndQuery, FALSE); // disabled in Map mode

    // ── Row 4 (y=118): ExchCode, Currency, MIC, OptType, State ──────────
    hwndExchCode = CreateWindow(L"EDIT",
                                L"",
                                edStyle,
                                10,
                                118,
                                100,
                                20,
                                m_hwnd,
                                (HMENU)ID_EDIT_EXCHCODE,
                                hInst,
                                nullptr);
    hwndCurrency = CreateWindow(L"EDIT",
                                L"",
                                edStyle,
                                115,
                                118,
                                105,
                                20,
                                m_hwnd,
                                (HMENU)ID_EDIT_CURRENCY,
                                hInst,
                                nullptr);
    hwndMicCode = CreateWindow(L"EDIT", L"", edStyle, 225, 118, 90, 20, m_hwnd, (HMENU)ID_EDIT_MICCODE, hInst, nullptr);
    hwndOptType = CreateWindow(L"EDIT",
                               L"",
                               edStyle,
                               320,
                               118,
                               95,
                               20,
                               m_hwnd,
                               (HMENU)ID_EDIT_OPTIONTYPE,
                               hInst,
                               nullptr);
    hwndStateCode = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 420,
                                 118,
                                 70,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_STATECODE,
                                 hInst,
                                 nullptr);

    // ── Row 4 continued: Unlisted checkbox ──────────────────────────────
    hwndUnlisted = CreateWindow(L"BUTTON",
                                L"Incl. Unlisted",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                500,
                                118,
                                120,
                                20,
                                m_hwnd,
                                (HMENU)ID_CHECK_UNLISTED,
                                hInst,
                                nullptr);

    // ── Row 5 (y=144): Sector, SecType, SecType2 ────────────────────────
    hwndSector = CreateWindow(L"EDIT",
                              L"",
                              edStyle,
                              10,
                              144,
                              150,
                              20,
                              m_hwnd,
                              (HMENU)ID_EDIT_MARKETSECDES,
                              hInst,
                              nullptr);
    hwndSecType = CreateWindow(L"EDIT",
                               L"",
                               edStyle,
                               165,
                               144,
                               150,
                               20,
                               m_hwnd,
                               (HMENU)ID_EDIT_SECTYPE,
                               hInst,
                               nullptr);
    hwndSecType2 = CreateWindow(L"EDIT",
                                L"",
                                edStyle,
                                320,
                                144,
                                150,
                                20,
                                m_hwnd,
                                (HMENU)ID_EDIT_SECTYPE2,
                                hInst,
                                nullptr);

    // ── Row 6 (y=170): Strike, Coupon intervals ─────────────────────────
    CreateWindow(L"STATIC", L"Strike:", WS_VISIBLE | WS_CHILD, 10, 173, 45, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndStrikeMin = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 58,
                                 170,
                                 80,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_STRIKE_MIN,
                                 hInst,
                                 nullptr);
    CreateWindow(L"STATIC", L"to", WS_VISIBLE | WS_CHILD, 143, 173, 15, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndStrikeMax = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 160,
                                 170,
                                 80,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_STRIKE_MAX,
                                 hInst,
                                 nullptr);

    CreateWindow(L"STATIC", L"Coupon:", WS_VISIBLE | WS_CHILD, 270, 173, 48, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndCouponMin = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 320,
                                 170,
                                 80,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_COUPON_MIN,
                                 hInst,
                                 nullptr);
    CreateWindow(L"STATIC", L"to", WS_VISIBLE | WS_CHILD, 405, 173, 15, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndCouponMax = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 422,
                                 170,
                                 80,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_COUPON_MAX,
                                 hInst,
                                 nullptr);

    // ── Row 7 (y=196): Expiry, Maturity intervals ───────────────────────
    CreateWindow(L"STATIC", L"Expiry:", WS_VISIBLE | WS_CHILD, 10, 199, 45, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndExpiryMin = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 58,
                                 196,
                                 100,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_EXPIRY_MIN,
                                 hInst,
                                 nullptr);
    CreateWindow(L"STATIC", L"to", WS_VISIBLE | WS_CHILD, 163, 199, 15, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndExpiryMax = CreateWindow(L"EDIT",
                                 L"",
                                 edStyle,
                                 180,
                                 196,
                                 100,
                                 20,
                                 m_hwnd,
                                 (HMENU)ID_EDIT_EXPIRY_MAX,
                                 hInst,
                                 nullptr);

    CreateWindow(L"STATIC", L"Maturity:", WS_VISIBLE | WS_CHILD, 310, 199, 55, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndMaturityMin = CreateWindow(L"EDIT",
                                   L"",
                                   edStyle,
                                   368,
                                   196,
                                   100,
                                   20,
                                   m_hwnd,
                                   (HMENU)ID_EDIT_MATURITY_MIN,
                                   hInst,
                                   nullptr);
    CreateWindow(L"STATIC", L"to", WS_VISIBLE | WS_CHILD, 473, 199, 15, 16, m_hwnd, nullptr, hInst, nullptr);
    hwndMaturityMax = CreateWindow(L"EDIT",
                                   L"",
                                   edStyle,
                                   490,
                                   196,
                                   100,
                                   20,
                                   m_hwnd,
                                   (HMENU)ID_EDIT_MATURITY_MAX,
                                   hInst,
                                   nullptr);

    // ── Request button (y=232) ──────────────────────────────────────────
    hwndRequestButton = CreateWindow(L"BUTTON",
                                     L"MAP IDENTIFIERS",
                                     btnStyle,
                                     center_x(300),
                                     232,
                                     300,
                                     45,
                                     m_hwnd,
                                     (HMENU)ID_BUTTON_REQUEST,
                                     hInst,
                                     nullptr);

    // ── Status (y=290) ──────────────────────────────────────────────────
    hwndWaitingMsg = CreateWindow(L"STATIC",
                                  L"",
                                  WS_VISIBLE | WS_CHILD | SS_CENTER,
                                  center_x(400),
                                  290,
                                  400,
                                  36,
                                  m_hwnd,
                                  (HMENU)ID_STATIC_MSG,
                                  hInst,
                                  nullptr);

    // ── Next Page (y=330) — hidden initially ─────────────────────────────
    hwndNextPageButton = CreateWindow(L"BUTTON",
                                      L"Next Page >>",
                                      btnStyle,
                                      center_x(140),
                                      332,
                                      140,
                                      28,
                                      m_hwnd,
                                      (HMENU)ID_BUTTON_NEXT_PAGE,
                                      hInst,
                                      nullptr);
    ShowWindow(hwndNextPageButton, SW_HIDE);

    // ── API Key (y=375) ─────────────────────────────────────────────────
    m_hwndAPIKey = CreateWindow(L"EDIT",
                                L"",
                                WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER,
                                center_x(250),
                                375,
                                250,
                                16,
                                m_hwnd,
                                (HMENU)ID_EDIT_APIKEY,
                                hInst,
                                nullptr);

    // ── About (bottom right) ────────────────────────────────────────────
    RECT pr;
    GetClientRect(m_hwnd, &pr);
    hwndAboutButton = CreateWindow(L"BUTTON",
                                   L"About",
                                   WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                                   pr.right - 38,
                                   pr.bottom - 16,
                                   36,
                                   14,
                                   m_hwnd,
                                   (HMENU)ID_BUTTON_ABOUT,
                                   hInst,
                                   nullptr);

    // ── Fonts ───────────────────────────────────────────────────────────
    SendMessage(hwndAboutButton, WM_SETFONT, (WPARAM)hFontAboutButtonText, TRUE);
    SendMessage(m_hwndAPIKey, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
    for (HWND h : {hwndRadioMap, hwndRadioSearch, hwndRadioFilter, hwndUnlisted})
        SendMessage(h, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

    // ── Cue banners (placeholder text) ──────────────────────────────────
    SendMessage(hwndQuery, EM_SETCUEBANNER, FALSE, (LPARAM)L"Keywords (search / filter only)");
    SendMessage(hwndExchCode, EM_SETCUEBANNER, FALSE, (LPARAM)L"ExchCode (US)");
    SendMessage(hwndCurrency, EM_SETCUEBANNER, FALSE, (LPARAM)L"Currency (USD)");
    SendMessage(hwndMicCode, EM_SETCUEBANNER, FALSE, (LPARAM)L"MIC (XNAS)");
    SendMessage(hwndOptType, EM_SETCUEBANNER, FALSE, (LPARAM)L"Call or Put");
    SendMessage(hwndStateCode, EM_SETCUEBANNER, FALSE, (LPARAM)L"State");
    SendMessage(hwndSector, EM_SETCUEBANNER, FALSE, (LPARAM)L"Market Sector (Equity)");
    SendMessage(hwndSecType, EM_SETCUEBANNER, FALSE, (LPARAM)L"Security Type");
    SendMessage(hwndSecType2, EM_SETCUEBANNER, FALSE, (LPARAM)L"Security Type 2");
    SendMessage(hwndStrikeMin, EM_SETCUEBANNER, FALSE, (LPARAM)L"min");
    SendMessage(hwndStrikeMax, EM_SETCUEBANNER, FALSE, (LPARAM)L"max");
    SendMessage(hwndCouponMin, EM_SETCUEBANNER, FALSE, (LPARAM)L"min");
    SendMessage(hwndCouponMax, EM_SETCUEBANNER, FALSE, (LPARAM)L"max");
    SendMessage(hwndExpiryMin, EM_SETCUEBANNER, FALSE, (LPARAM)L"YYYY-MM-DD");
    SendMessage(hwndExpiryMax, EM_SETCUEBANNER, FALSE, (LPARAM)L"YYYY-MM-DD");
    SendMessage(hwndMaturityMin, EM_SETCUEBANNER, FALSE, (LPARAM)L"YYYY-MM-DD");
    SendMessage(hwndMaturityMax, EM_SETCUEBANNER, FALSE, (LPARAM)L"YYYY-MM-DD");
    SendMessage(m_hwndAPIKey, EM_SETCUEBANNER, FALSE, (LPARAM)L"Enter API key (optional)");

    return TRUE;
}

// ─── Message Handler ────────────────────────────────────────────────────────

LRESULT WindowState::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);

            // ── Buttons ─────────────────────────────────────────────────────
            if (code == BN_CLICKED || code == 0)
            {
                if (id == ID_BUTTON_FILE_PATH)
                {
                    get_open_path();
                    break;
                }
                if (id == ID_BUTTON_SAVE_PATH)
                {
                    get_save_path();
                    break;
                }
                if (id == ID_BUTTON_ABOUT)
                {
                    CreateAboutWindow();
                    break;
                }
                if (id == ID_BUTTON_CLOSE)
                {
                    DestroyWindow(m_hwnd);
                    break;
                }

                // Radio buttons
                if (id == ID_RADIO_MAP || id == ID_RADIO_SEARCH || id == ID_RADIO_FILTER)
                {
                    on_mode_changed();
                    break;
                }

                // Save
                if (id == ID_BUTTON_SAVE)
                {
                    SetWindowText(hwndWaitingMsg, L"");
                    if (m_save_path.empty())
                    {
                        SetWindowText(hwndWaitingMsg, L"Save Path Empty!");
                        break;
                    }
                    {
                        std::lock_guard lock(m_results_mutex);
                        if (m_mapping_results.empty() && m_search_results.empty())
                        {
                            SetWindowText(hwndWaitingMsg, L"Nothing to Save");
                            break;
                        }
                    }
                    SetWindowText(hwndWaitingMsg, L"Saving...");
                    UpdateWindow(hwndWaitingMsg);
                    EnableWindow(hwndSaveButton, FALSE);
                    start_save_thread();
                    break;
                }

                // Request (map / search / filter)
                if (id == ID_BUTTON_REQUEST)
                {
                    if (m_mode == OpMode::Map && m_open_path.empty())
                    {
                        SetWindowText(hwndWaitingMsg, L"Input Path Empty!");
                        break;
                    }
                    EnableWindow(hwndRequestButton, FALSE);
                    ShowWindow(hwndNextPageButton, SW_HIDE);
                    SetWindowText(hwndWaitingMsg, L"Processing...");
                    UpdateWindow(hwndWaitingMsg);
                    start_request_thread();
                    break;
                }

                // Next page
                if (id == ID_BUTTON_NEXT_PAGE)
                {
                    EnableWindow(hwndNextPageButton, FALSE);
                    SetWindowText(hwndWaitingMsg, L"Loading next page...");
                    UpdateWindow(hwndWaitingMsg);
                    start_next_page_thread();
                    break;
                }
            }

            // ── Edit focus loss ─────────────────────────────────────────────
            if (code == EN_KILLFOCUS)
            {
                HWND hEdit = (HWND)lParam;
                wchar_t buf[512]{ };
                GetWindowText(hEdit, buf, 512);

                if (id == ID_FILE_PATH)
                    m_open_path = Utils::wide_to_str(buf);
                else if (id == ID_SAVE_PATH)
                    m_save_path = Utils::wide_to_str(buf);
                else if (id == ID_EDIT_APIKEY)
                {
                    m_apikey = buf;
                    if (m_client)
                        m_client->set_api_key(Utils::wide_to_str(m_apikey));
                }
                break;
            }
            break;
        }

        case WM_MAKE_REQUEST_COMPLETE:
        {
            std::lock_guard lock(m_results_mutex);
            std::wstring msg;
            if (m_mode == OpMode::Map)
            {
                msg = std::format(L"Complete! ({} results)", m_mapping_results.size());
            }
            else
            {
                msg = std::format(L"{} instruments loaded", m_search_results.size());
                if (m_total_count)
                    msg += std::format(L" (of {})", *m_total_count);
            }
            SetWindowText(hwndWaitingMsg, msg.c_str());
            EnableWindow(hwndRequestButton, TRUE);

            // Show Next Page button if pagination available.
            if (m_next_cursor.has_value())
            {
                ShowWindow(hwndNextPageButton, SW_SHOW);
                EnableWindow(hwndNextPageButton, TRUE);
            }
            else
            {
                ShowWindow(hwndNextPageButton, SW_HIDE);
            }
            break;
        }

        case WM_MAKE_REQUEST_FAILED:
        {
            std::string err;
            {
                std::lock_guard lock(m_results_mutex);
                err = m_error_msg;
            }
            SetWindowText(hwndWaitingMsg, Utils::str_to_wide(err).c_str());
            EnableWindow(hwndRequestButton, TRUE);
            ShowWindow(hwndNextPageButton, SW_HIDE);
            break;
        }

        case WM_SAVE_COMPLETE: SetWindowText(hwndWaitingMsg, L"File Saved!");
            EnableWindow(hwndSaveButton, TRUE);
            break;

        case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            FillRect((HDC)wParam, &rc, m_hbrBackground);
            return 1;
        }

        case WM_NOTIFY:
        {
            LPNMHDR p = (LPNMHDR)lParam;
            if (p->idFrom == ID_STATIC_ABOUT_MSG && (p->code == NM_CLICK || p->code == NM_RETURN))
            {
                ShellExecute(nullptr, L"open", ((PNMLINK)lParam)->item.szUrl, nullptr, nullptr, SW_SHOWNORMAL);
                return 0;
            }
            break;
        }

        case WM_APP_CHILD_CLOSED:
            aboutWindow.reset();
            return 0;

        case WM_CTLCOLOREDIT:
        {
            HWND hEdit = (HWND)lParam;
            HDC hdc = (HDC)wParam;
            if (hEdit == hwndFilePath || hEdit == hwndSavePath)
            {
                SetTextColor(hdc, RGB(0, 0, 0));
                SetBkColor(hdc, RGB(169, 169, 169));
            }
            return (INT_PTR)hbrEditBackground;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            return (INT_PTR)GetStockObject(WHITE_BRUSH);
        }

        case WM_CLOSE:
            DestroyWindow(m_hwnd);
            if (m_hwndParent)
            {
                InvalidateRect(m_hwndParent, nullptr, TRUE);
                UpdateWindow(m_hwndParent);
            }
            return 0;

        case WM_DESTROY:
            if (!m_hwndParent)
                PostQuitMessage(0);
            else
                PostMessage(m_hwndParent, WM_APP_CHILD_CLOSED, 0, 0);
            return 0;

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

// ─── Mode switching ─────────────────────────────────────────────────────────

void WindowState::on_mode_changed()
{
    if (SendMessage(hwndRadioMap, BM_GETCHECK, 0, 0) == BST_CHECKED)
        m_mode = OpMode::Map;
    else if (SendMessage(hwndRadioSearch, BM_GETCHECK, 0, 0) == BST_CHECKED)
        m_mode = OpMode::Search;
    else
        m_mode = OpMode::Filter;

    // Query field only active in Search/Filter.
    EnableWindow(hwndQuery, m_mode != OpMode::Map);
    // File path only relevant in Map mode.
    EnableWindow(hwndFilePath, m_mode == OpMode::Map);
    EnableWindow(hwndFileButton, m_mode == OpMode::Map);

    update_request_button_text();
    ShowWindow(hwndNextPageButton, SW_HIDE);
}

void WindowState::update_request_button_text()
{
    switch (m_mode)
    {
        case OpMode::Map: SetWindowText(hwndRequestButton, L"MAP IDENTIFIERS");
            break;
        case OpMode::Search: SetWindowText(hwndRequestButton, L"SEARCH");
            break;
        case OpMode::Filter: SetWindowText(hwndRequestButton, L"FILTER");
            break;
    }
}

// ─── About window ───────────────────────────────────────────────────────────

BOOL WindowState::CreateAboutWindow()
{
    if (aboutWindow)
        return TRUE;

    INITCOMMONCONTROLSEX icex{sizeof(icex), ICC_STANDARD_CLASSES | ICC_LINK_CLASS};
    InitCommonControlsEx(&icex);

    aboutWindow = std::make_unique<WindowState>(m_hwnd, m_fileState, nullptr);

    RECT pr;
    GetWindowRect(m_hwnd, &pr);
    int cw = 420, ch = 140;
    int cx = pr.left + (PARENT_WINDOW_WIDTH - cw) / 2;
    int cy = pr.top + (PARENT_WINDOW_HEIGHT - ch) / 2;

    const wchar_t* txt = L"FIGIMappingTool v2.0 (x64)\r\n\r\n"
        L"Map, search, and filter FIGI instruments via the OpenFIGI API.\r\n\r\n" L"License: MIT License\r\n"
        L"<A HREF=\"https://github.com/mdgr122/FIGIMappingTool\">GitHub Repository</A>";

    if (aboutWindow->Create(L"", WS_POPUP | WS_BORDER | WS_VISIBLE, 0, 50, 50, cw, ch, m_hwnd))
    {
        m_hwndAboutWindow = aboutWindow->Window();
        SetWindowPos(m_hwndAboutWindow, HWND_TOP, cx, cy, cw, ch, SWP_NOSIZE | SWP_NOACTIVATE);

        hwndCloseButton = CreateWindow(L"BUTTON",
                                       L"X",
                                       WS_CHILD | WS_VISIBLE | BS_CENTER | BS_PUSHBUTTON,
                                       cw - 16,
                                       -1,
                                       16,
                                       14,
                                       m_hwndAboutWindow,
                                       (HMENU)ID_BUTTON_CLOSE,
                                       GetModuleHandle(nullptr),
                                       nullptr);
        hwndAboutText = CreateWindow(WC_LINK,
                                     txt,
                                     WS_CHILD | WS_VISIBLE | SS_LEFT,
                                     2,
                                     10,
                                     cw - 2,
                                     ch - 20,
                                     m_hwndAboutWindow,
                                     (HMENU)ID_STATIC_ABOUT_MSG,
                                     GetModuleHandle(nullptr),
                                     nullptr);
        SendMessage(hwndAboutText, WM_SETFONT, (WPARAM)hFontAboutText, TRUE);
        return TRUE;
    }
    aboutWindow.reset();
    m_hwndAboutWindow = nullptr;
    return FALSE;
}

// ─── File paths ─────────────────────────────────────────────────────────────

void WindowState::get_open_path()
{
    m_open_path = m_fileState.get_open_path();
    SetWindowText(hwndFilePath, Utils::str_to_wide(m_open_path).c_str());
}

void WindowState::get_save_path()
{
    m_save_path = m_fileState.get_save_path();
    SetWindowText(hwndSavePath, Utils::str_to_wide(m_save_path).c_str());
}

// ─── GUI → filter helpers ───────────────────────────────────────────────────

std::string WindowState::read_edit_text(HWND hwnd)
{
    wchar_t buf[512]{ };
    GetWindowText(hwnd, buf, 512);
    std::string s = Utils::wide_to_str(buf);
    // Trim whitespace.
    while (!s.empty() && s.front() == ' ')
        s.erase(s.begin());
    while (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

std::optional<figi::Interval<double>> WindowState::read_double_interval(HWND hMin, HWND hMax)
{
    auto sMin = read_edit_text(hMin);
    auto sMax = read_edit_text(hMax);
    if (sMin.empty() && sMax.empty())
        return std::nullopt;

    figi::Interval<double> iv;
    try
    {
        if (!sMin.empty())
            iv.lower = std::stod(sMin);
    }
    catch (...)
    {}
    try
    {
        if (!sMax.empty())
            iv.upper = std::stod(sMax);
    }
    catch (...)
    {}
    if (!iv.lower && !iv.upper)
        return std::nullopt;
    return iv;
}

std::optional<figi::Interval<std::string>> WindowState::read_string_interval(HWND hMin, HWND hMax)
{
    auto sMin = read_edit_text(hMin);
    auto sMax = read_edit_text(hMax);
    if (sMin.empty() && sMax.empty())
        return std::nullopt;

    figi::Interval<std::string> iv;
    if (!sMin.empty())
        iv.lower = sMin;
    if (!sMax.empty())
        iv.upper = sMax;
    if (!iv.lower && !iv.upper)
        return std::nullopt;
    return iv;
}

void WindowState::apply_gui_filters(figi::MappingJob& job)
{
    auto set_if = [](auto& field, const std::string& val)
    {
        if (!val.empty() && !field.has_value())
            field = val;
    };

    set_if(job.exchCode, read_edit_text(hwndExchCode));
    set_if(job.currency, read_edit_text(hwndCurrency));
    set_if(job.micCode, read_edit_text(hwndMicCode));
    set_if(job.optionType, read_edit_text(hwndOptType));
    set_if(job.stateCode, read_edit_text(hwndStateCode));
    set_if(job.marketSecDes, read_edit_text(hwndSector));
    set_if(job.securityType, read_edit_text(hwndSecType));
    set_if(job.securityType2, read_edit_text(hwndSecType2));

    if (SendMessage(hwndUnlisted, BM_GETCHECK, 0, 0) == BST_CHECKED)
        job.includeUnlistedEquities = true;

    if (!job.strike)
        job.strike = read_double_interval(hwndStrikeMin, hwndStrikeMax);
    if (!job.coupon)
        job.coupon = read_double_interval(hwndCouponMin, hwndCouponMax);
    if (!job.expiration)
        job.expiration = read_string_interval(hwndExpiryMin, hwndExpiryMax);
    if (!job.maturity)
        job.maturity = read_string_interval(hwndMaturityMin, hwndMaturityMax);
}

figi::SearchRequest WindowState::build_search_request()
{
    figi::SearchRequest req;

    auto q = read_edit_text(hwndQuery);
    if (!q.empty())
        req.query = q;

    auto set = [&](auto& field, HWND h)
    {
        auto v = read_edit_text(h);
        if (!v.empty())
            field = v;
    };
    set(req.exchCode, hwndExchCode);
    set(req.currency, hwndCurrency);
    set(req.micCode, hwndMicCode);
    set(req.optionType, hwndOptType);
    set(req.stateCode, hwndStateCode);
    set(req.marketSecDes, hwndSector);
    set(req.securityType, hwndSecType);
    set(req.securityType2, hwndSecType2);

    if (SendMessage(hwndUnlisted, BM_GETCHECK, 0, 0) == BST_CHECKED)
        req.includeUnlistedEquities = true;

    req.strike = read_double_interval(hwndStrikeMin, hwndStrikeMax);
    req.coupon = read_double_interval(hwndCouponMin, hwndCouponMax);
    req.expiration = read_string_interval(hwndExpiryMin, hwndExpiryMax);
    req.maturity = read_string_interval(hwndMaturityMin, hwndMaturityMax);

    return req;
}

// ─── Worker threads ─────────────────────────────────────────────────────────

void WindowState::start_request_thread()
{
    if (m_request_thread.joinable())
    {
        m_request_thread.request_stop();
        m_request_thread.join();
    }
    m_request_thread = std::jthread([this](std::stop_token) { do_request(); });
}

void WindowState::start_save_thread()
{
    if (m_save_thread.joinable())
    {
        m_save_thread.request_stop();
        m_save_thread.join();
    }
    m_save_thread = std::jthread([this](std::stop_token) { do_save(); });
}

void WindowState::start_next_page_thread()
{
    if (m_request_thread.joinable())
    {
        m_request_thread.request_stop();
        m_request_thread.join();
    }
    m_request_thread = std::jthread([this](std::stop_token) { do_next_page(); });
}

void WindowState::do_request()
{
    if (!m_client)
    {
        PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
        return;
    }

    // Clear previous results.
    {
        std::lock_guard lock(m_results_mutex);
        m_mapping_results.clear();
        m_search_results.clear();
        m_next_cursor.reset();
        m_total_count.reset();
        m_error_msg.clear();
    }

    if (m_mode == OpMode::Map)
    {
        // ── Mapping mode ────────────────────────────────────────────────
        m_fileState.clear_data();
        m_fileState.read_file(m_open_path);

        auto& lines = m_fileState.get_lines();
        auto jobs = is_csv_path(m_open_path) ? parse_csv_lines(lines) : parse_txt_lines(lines);

        if (jobs.empty())
        {
            std::lock_guard lock(m_results_mutex);
            m_error_msg = "No valid identifiers found in file";
            PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
            return;
        }

        // Apply GUI filter overrides to every job.
        for (auto& job : jobs)
            apply_gui_filters(job);

        auto result = m_client->map(std::move(jobs));
        std::lock_guard lock(m_results_mutex);
        if (result)
        {
            m_mapping_results = std::move(*result);
            PostMessage(m_hwnd, WM_MAKE_REQUEST_COMPLETE, 0, 0);
        }
        else
        {
            m_error_msg = std::format("Error: {}", result.error().message);
            PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
        }
    }
    else
    {
        // ── Search / Filter mode ────────────────────────────────────────
        auto req = build_search_request();

        auto result = (m_mode == OpMode::Search) ? m_client->search(req) : m_client->filter(req);

        std::lock_guard lock(m_results_mutex);
        if (result)
        {
            m_search_results = std::move(result->data);
            m_next_cursor = result->next;
            m_total_count = result->total;
            PostMessage(m_hwnd, WM_MAKE_REQUEST_COMPLETE, 0, 0);
        }
        else
        {
            m_error_msg = std::format("Error: {}", result.error().message);
            PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
        }
    }
}

void WindowState::do_next_page()
{
    if (!m_client)
        return;

    std::string cursor;
    {
        std::lock_guard lock(m_results_mutex);
        if (!m_next_cursor)
            return;
        cursor = *m_next_cursor;
    }

    auto req = build_search_request();
    req.start = cursor;

    auto result = (m_mode == OpMode::Search) ? m_client->search(req) : m_client->filter(req);

    std::lock_guard lock(m_results_mutex);
    if (result)
    {
        m_search_results.insert(m_search_results.end(),
                                std::make_move_iterator(result->data.begin()),
                                std::make_move_iterator(result->data.end()));
        m_next_cursor = result->next;
        if (result->total)
            m_total_count = result->total;
        PostMessage(m_hwnd, WM_MAKE_REQUEST_COMPLETE, 0, 0);
    }
    else
    {
        m_error_msg = std::format("Error: {}", result.error().message);
        PostMessage(m_hwnd, WM_MAKE_REQUEST_FAILED, 0, 0);
    }
}

void WindowState::do_save()
{
    bool csv = is_csv_path(m_save_path);

    std::lock_guard lock(m_results_mutex);

    if (m_mode == OpMode::Map)
    {
        if (csv)
        {
            m_fileState.save_text_file(figi::export_csv(m_mapping_results), m_save_path);
        }
        else
        {
            m_fileState.save_json_file(figi::export_json(m_mapping_results), m_save_path);
        }
    }
    else
    {
        if (csv)
        {
            m_fileState.save_text_file(figi::export_csv(m_search_results), m_save_path);
        }
        else
        {
            m_fileState.save_json_file(figi::export_json(m_search_results), m_save_path);
        }
    }

    PostMessage(m_hwnd, WM_SAVE_COMPLETE, 0, 0);
}

bool WindowState::is_csv_path(const std::string& path)
{
    return path.size() >= 4 && path.compare(path.size() - 4, 4, ".csv") == 0;
}

// ─── Input parsing: TXT ─────────────────────────────────────────────────────

std::vector<figi::MappingJob> WindowState::parse_txt_lines(const std::vector<std::string>& lines)
{
    std::vector<figi::MappingJob> jobs;
    jobs.reserve(lines.size());

    for (const auto& line : lines)
    {
        std::istringstream iss(line);
        std::string base_id, context;

        if (!(iss >> base_id))
            continue;
        if (iss >> context)
        {
            if (context == "Index")
            {
                base_id += " Index";
                context.clear();
            }
        }

        auto detected = figi::OpenFigiClient::detect_id_type(base_id);
        if (!detected)
            continue;

        figi::MappingJob job{.idType = *detected, .idValue = base_id};

        if (job.idType == figi::IdType::TICKER)
            process_ticker(job.idValue);

        if (!context.empty())
        {
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

// ─── Input parsing: CSV ─────────────────────────────────────────────────────

std::vector<std::string> WindowState::split_csv_line(const std::string& line)
{
    std::vector<std::string> fields;
    std::string current;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        if (in_quotes)
        {
            if (c == '"')
            {
                if (i + 1 < line.size() && line[i + 1] == '"')
                {
                    current += '"';
                    ++i;
                }
                else
                {
                    in_quotes = false;
                }
            }
            else
            {
                current += c;
            }
        }
        else
        {
            if (c == '"')
            {
                in_quotes = true;
            }
            else if (c == ',')
            {
                fields.push_back(current);
                current.clear();
            }
            else
            {
                current += c;
            }
        }
    }
    fields.push_back(current);
    return fields;
}

std::vector<figi::MappingJob> WindowState::parse_csv_lines(const std::vector<std::string>& lines)
{
    if (lines.empty())
        return { };

    // Parse header row.
    auto headers = split_csv_line(lines[0]);
    std::unordered_map<std::string, size_t> col;
    for (size_t i = 0; i < headers.size(); ++i)
    {
        auto h = headers[i];
        // Normalize: lowercase, trim.
        std::transform(h.begin(), h.end(), h.begin(), ::tolower);
        while (!h.empty() && h.front() == ' ')
            h.erase(h.begin());
        while (!h.empty() && h.back() == ' ')
            h.pop_back();
        col[h] = i;
    }

    // Map common aliases.
    auto find_col = [&](std::initializer_list<std::string> names) -> std::optional<size_t>
    {
        for (const auto& n : names)
            if (auto it = col.find(n); it != col.end())
                return it->second;
        return std::nullopt;
    };

    auto c_value = find_col({"idvalue", "id_value", "value", "identifier"});
    auto c_type = find_col({"idtype", "id_type", "type"});
    auto c_exch = find_col({"exchcode", "exch_code", "exchange"});
    auto c_mic = find_col({"miccode", "mic_code", "mic"});
    auto c_ccy = find_col({"currency", "ccy"});
    auto c_sector = find_col({"marketsecdes", "market_sec_des", "sector", "marketsector"});
    auto c_sectype = find_col({"securitytype", "security_type", "sectype"});
    auto c_sectype2 = find_col({"securitytype2", "security_type2", "sectype2"});
    auto c_opttype = find_col({"optiontype", "option_type"});
    auto c_state = find_col({"statecode", "state_code", "state"});

    if (!c_value)
    {
        // If no recognized header, treat as simple single-column file.
        return parse_txt_lines(lines);
    }

    auto get = [](const std::vector<std::string>& row, std::optional<size_t> idx) -> std::string
    {
        if (!idx || *idx >= row.size())
            return { };
        auto s = row[*idx];
        while (!s.empty() && s.front() == ' ')
            s.erase(s.begin());
        while (!s.empty() && s.back() == ' ')
            s.pop_back();
        return s;
    };

    std::vector<figi::MappingJob> jobs;
    jobs.reserve(lines.size() - 1);

    for (size_t i = 1; i < lines.size(); ++i)
    {
        auto fields = split_csv_line(lines[i]);
        auto idValue = get(fields, c_value);
        if (idValue.empty())
            continue;

        // Determine idType: explicit column or auto-detect.
        std::optional<figi::IdType> idType;
        auto typeStr = get(fields, c_type);
        if (!typeStr.empty())
            idType = figi::parse_id_type(typeStr);
        if (!idType)
            idType = figi::OpenFigiClient::detect_id_type(idValue);
        if (!idType)
            continue;

        figi::MappingJob job{.idType = *idType, .idValue = idValue};

        if (job.idType == figi::IdType::TICKER)
            process_ticker(job.idValue);

        auto set = [&](auto& field, const std::string& val)
        {
            if (!val.empty())
                field = val;
        };
        set(job.exchCode, get(fields, c_exch));
        set(job.currency, get(fields, c_ccy));
        set(job.micCode, get(fields, c_mic));
        set(job.marketSecDes, get(fields, c_sector));
        set(job.securityType, get(fields, c_sectype));
        set(job.securityType2, get(fields, c_sectype2));
        set(job.optionType, get(fields, c_opttype));
        set(job.stateCode, get(fields, c_state));

        jobs.push_back(std::move(job));
    }
    return jobs;
}

// ─── Ticker normalization ───────────────────────────────────────────────────

void WindowState::process_ticker(std::string& str)
{
    std::string suffix;
    if (auto pos = str.find(" Index"); pos != std::string::npos)
    {
        suffix = str.substr(pos);
        str.erase(pos);
    }

    if (auto dot = str.find('.'); dot != std::string::npos)
    {
        size_t after = str.size() - dot - 1;
        if (after == 0)
            str.erase(dot, 1);
        else if (after == 1)
            str[dot] = '/';
        else
            str.replace(dot, after + 1, "-" + str.substr(dot + 1, 1));
    }

    str += suffix;
}
