#include "WindowState.h"
#include <iostream>
#include <string>
#include <commctrl.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' "    \
    "name='Microsoft.Windows.Common-Controls' "                  \
    "version='6.0.0.0' "                                         \
    "processorArchitecture='*' "                                 \
    "publicKeyToken='6595b64144ccf1df' "                         \
    "language='*'\"")


// Define the window class name and window title at the top for consistency
const wchar_t CLASS_NAME[] = L"FIGIMappingTool";
const wchar_t ABOUT_CLASS_NAME[] = L"AboutPopupWinudow";
const wchar_t WINDOW_TITLE[] = L"FIGIMappingTool";

WindowState::WindowState(HWND hParent, FileState& fileState, Request& request, JsonParse& jsonParse) 
    : m_hwndParent(hParent)
    , m_hbrBackground(CreateSolidBrush(RGB(255,255,255)))
    , fileState(fileState)
    , request(request)
    , jsonParse(jsonParse)
    , aboutWindow{ nullptr }
    , m_hwndAboutWindow{ nullptr }
    , m_hwndAPIKey{ nullptr }
    , hwndAboutButton{ nullptr }
    , hwndAboutPopup{ nullptr }
    , hwndAboutText{ nullptr }
    , hwndCloseButton{ nullptr }
    , hwndSaveButton{ nullptr }
    , hwndSaveButton2{ nullptr }
    , hwndFileButton{ nullptr }
    , hwndRequestButton{ nullptr }
    , hwndFilePath{ nullptr }
    , hwndSavePath{ nullptr }
    , hwndWaitingMsg{ nullptr }
    , hbrEditBackground(NULL)
    , hFontAboutButtonText(NULL)
    , hFontAboutText(NULL)
    , hFontSmall(NULL)
    , m_apikey{ L"853a5b1c-9ee7-45a9-85fe-67504db399b0" }
{
    nWidth = GetSystemMetrics(SM_CXSCREEN);
    nHeight = GetSystemMetrics(SM_CYSCREEN);
    request.set_apikey(m_apikey);

    hbrEditBackground = CreateSolidBrush(RGB(240, 240, 240));

    hFontAboutButtonText = CreateFont(
        12,                 // Height (0 for default height)
        0,                 // Width (0 for default width based on height)
        0,                 // Escapement angle (0 for normal orientation)
        0,                 // Orientation angle (0 for normal orientation)
        FW_NORMAL,          // Weight (FW_DONTCARE lets the system choose an appropriate weight) 700 for bold
        FALSE,             // Italic (FALSE for no italic)
        FALSE,             // Underline (FALSE for no underline)
        FALSE,             // Strikeout (FALSE for no strikeout)
        DEFAULT_CHARSET,   // Character set (DEFAULT_CHARSET for default character set)
        OUT_DEFAULT_PRECIS,// Output precision (OUT_DEFAULT_PRECIS for default)
        CLIP_DEFAULT_PRECIS,// Clipping precision (CLIP_DEFAULT_PRECIS for default)
        DEFAULT_QUALITY,   // Output quality (DEFAULT_QUALITY for default)
        DEFAULT_PITCH | FF_SWISS, // Pitch and family (DEFAULT_PITCH and FF_DONTCARE for default)
        L"Segoe UI");              // Font family name ("" or `NULL` for system default font)

    hFontAboutText = CreateFont(
        16,                 // Height
        0,                  // Width
        0,                  // Escapement
        0,                  // Orientation
        FW_NORMAL,          // Weight
        FALSE,              // Italic
        FALSE,              // Underline
        FALSE,              // StrikeOut
        DEFAULT_CHARSET,    // CharSet
        OUT_DEFAULT_PRECIS, // OutPrecision
        CLIP_DEFAULT_PRECIS,// ClipPrecision
        DEFAULT_QUALITY,    // Quality
        DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
        L"Segoe UI");       // Font name

    hFontSmall = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    

}

WindowState::~WindowState()
{
    // Clean up GDI objects
    if (m_hbrBackground)
    {
        DeleteObject(m_hbrBackground);
        m_hbrBackground = NULL;
    }
    if (hbrEditBackground)
    {
        DeleteObject(hbrEditBackground);
        hbrEditBackground = NULL;
    }
    if (hFontAboutButtonText)
    {
        DeleteObject(hFontAboutButtonText);
        hFontAboutButtonText = NULL;
    }
    if (hFontAboutText)
    {
        DeleteObject(hFontAboutText);
        hFontAboutText = NULL;
    }
    if (hFontSmall)
    {
        DeleteObject(hFontSmall);
        hFontSmall = NULL;
    }
}

PCWSTR WindowState::ClassName() const
{
    return L"FIGIMappingTool";
}

LRESULT WindowState::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {

        if (LOWORD(wParam) == ID_BUTTON_FILE_PATH)
        {
            get_open_path();
            break;
        }
        else if (LOWORD(wParam) == ID_BUTTON_SAVE_PATH)
        {
            get_save_path();
            break;
        }
        else if (LOWORD(wParam) == ID_BUTTON_SAVE)
        {
            SetWindowText(hwndWaitingMsg, L"");
            if (m_save_path.empty())
            {
                SetWindowText(hwndWaitingMsg, L"Save Path Empty!");
                break;
            }
            //else if (request.GetResponse().empty())
            else if (!request.PeakResponse())
            {
                SetWindowText(hwndWaitingMsg, L"Nothing to Save");
                break;
            }
            else
            {
                SetWindowText(hwndWaitingMsg, L"Saving...");
                    // There is a slight delay when it enters the else, because it's retrieving the request.GetResponse()
                    // This is because SetWindowText is not drawn until the window is painted, which doesn't happen until later in the loop.
                    // Therefore, ven though SetWindowText looks to be before the if statement, actual painting occurs later.
                UpdateWindow(hwndWaitingMsg);
                EnableWindow(hwndSaveButton, FALSE); // Disable the Save button

                // Start the save operation on a new thread
                StartSaveThread();
                break;
            }
        }
        else if (LOWORD(wParam) == ID_BUTTON_REQUEST)
        {
            if (m_open_path.empty())
            {
                SetWindowText(hwndWaitingMsg, L"Input Path Empty!");
                break;
            }
            EnableWindow(hwndRequestButton, FALSE);
            SetWindowText(hwndWaitingMsg, L"Processing...");
            UpdateWindow(hwndWaitingMsg);
            StartMakeRequestThread();
            break;
        }
        else if (LOWORD(wParam) == ID_BUTTON_ABOUT)
        {
            CreateAboutWindow();
        }
        else if (LOWORD(wParam) == ID_BUTTON_CLOSE)
        {
            DestroyWindow(m_hwnd); // This will trigger WM_DESTROY and notify the parent
            break;
        }

        if (HIWORD(wParam) == EN_KILLFOCUS)
        {
            HWND hEditControl = (HWND)lParam;
            if (LOWORD(wParam) == ID_FILE_PATH)
            {
                wchar_t buffer[256];
                GetWindowText(hEditControl, buffer, 256);
                m_open_path = Utils::GetInstance().wideToStr(buffer);
                break;
            }
            else if (LOWORD(wParam) == ID_SAVE_PATH)
            {
                wchar_t buffer[256];
                GetWindowText(hEditControl, buffer, 256);
                m_save_path = Utils::GetInstance().wideToStr(buffer);
                break;
            }
            else if (LOWORD(wParam) == ID_EDIT_APIKEY)
            {
                wchar_t buffer[256];
                GetWindowText(hEditControl, buffer, 256);
                m_apikey = buffer;

                request.set_apikey(m_apikey);
            }
            break;
        }
        break;
    }
    case WM_MAKE_REQUEST_COMPLETE:
    {
        SetWindowText(hwndWaitingMsg, L"Complete!");
        EnableWindow(hwndRequestButton, TRUE);
        break;
    }
    case WM_SAVE_COMPLETE:
    {
        SetWindowText(hwndWaitingMsg, L"File Saved!");
        EnableWindow(hwndSaveButton, TRUE); // Re-enable the Save button
        break;
    }
    case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(m_hwnd, &rc);

            FillRect(hdc, &rc, m_hbrBackground);

            return 1; // Indicate that we've handled the message
        }
    case WM_NOTIFY:
    {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        if (pnmhdr->idFrom == ID_STATIC_ABOUT_MSG &&
            (pnmhdr->code == NM_CLICK || pnmhdr->code == NM_RETURN))
        {
            PNMLINK pNMLink = (PNMLINK)lParam;
            const wchar_t* link = pNMLink->item.szUrl;

            // Open the link in the default browser
            ShellExecute(NULL, L"open", link, NULL, NULL, SW_SHOWNORMAL);

            return 0;
        }
        break;
    }
    case WM_APP_CHILD_CLOSED:
    {
        // Reset the unique_ptr as the child window is closed
        aboutWindow.reset();
        return 0;
    }
    case WM_CTLCOLOREDIT:
    {
        HWND hEditControl = (HWND)lParam; // Handle to a window. Declared in WinDef.h as typedef HANDLE HWND;
        HDC hdcEdit = (HDC)wParam; // Handle to a device context (DC). Declared in WinDef.h as typedef HANDLE HDC;

        if (hEditControl == hwndFilePath)
        {
            // Set the text and background colors
            SetTextColor(hdcEdit, RGB(0, 0, 0));        // Text color (black)
            SetBkColor(hdcEdit, RGB(169, 169, 169));    // Background color (light gray)
        }
        else if (hEditControl == hwndSavePath)
        {
            // Set the text and background colors
            SetTextColor(hdcEdit, RGB(0, 0, 0));        // Text color (black)
            SetBkColor(hdcEdit, RGB(169, 169, 169));    // Background color (light gray)
        }

        // Return the background brush to paint the control's background
        return (INT_PTR)hbrEditBackground;
    }
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam; 
        HWND hwndStatic = (HWND)lParam;

        if (GetDlgCtrlID(hwndStatic) == ID_STATIC_MSG)
        {
            SetBkMode(hdcStatic, TRANSPARENT);
            SetTextColor(hdcStatic, RGB(0, 0, 0));
        }
        return (INT_PTR)GetStockObject(WHITE_BRUSH);
        break;
    }
    case WM_CLOSE:
    {
        if (m_hwndParent != NULL) // if m_hwndParent == NULL, that means m_hwnd == m_hwndParent == NULL
        {
            DestroyWindow(m_hwnd); // Close only the child window
            InvalidateRect(m_hwndParent, NULL, TRUE);
            UpdateWindow(m_hwndParent);
            return 0;
        }
        // return 0 **** this extra return statement was the cause of a lot of pain.
        else
        {
            DestroyWindow(m_hwnd);
            return 0;
        }
        break;
    }
    case WM_DESTROY:
    {
        if (m_hwndParent == NULL) // Only post quit message for the main window
        {
            PostQuitMessage(0); // End the application
        }
        else
        {
            // Notify the parent that the child is closing
            PostMessage(m_hwndParent, WM_APP_CHILD_CLOSED, 0, 0);
        }
        return 0;
    }

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

BOOL WindowState::CreateParentWindow()
{

    // Create the main (parent) window with the specified styles
    if (!Create(L"FIGIMappingTool", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX))
    {
        return FALSE; // If creation fails, return FALSE
    }



    int xPos = (nWidth - PARENT_WINDOW_WIDTH) / 2;
    int yPos = (nHeight - PARENT_WINDOW_HEIGHT) / 2;
    DWORD default_btn_style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON;
    //DWORD default_btn_style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON;

    // Center the window on the screen
    SetWindowPos(m_hwnd, NULL, xPos, yPos, PARENT_WINDOW_WIDTH, PARENT_WINDOW_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);

    RECT parent_rect;
    GetClientRect(m_hwnd, &parent_rect);

    int about_btn_width = 36;
    int about_btn_height = 14;
    int x_pos_about_btn = (parent_rect.right - about_btn_width - 2);
    int y_pos_about_btn = (parent_rect.bottom - about_btn_height - 2);



    // Button Windows 20 80 630 50
    hwndRequestButton = CreateWindow(L"BUTTON", L"REQUEST", default_btn_style, get_parent_middle_width(PARENT_WINDOW_WIDTH, 300), 80, 300, 50, m_hwnd, (HMENU)ID_BUTTON_REQUEST, GetModuleHandle(NULL), NULL);
    hwndFileButton = CreateWindow(L"BUTTON", L"File", default_btn_style, 550, 10, 50, 20, m_hwnd, (HMENU)ID_BUTTON_FILE_PATH, GetModuleHandle(NULL), NULL);
    hwndSaveButton = CreateWindow(L"BUTTON", L"Save", default_btn_style, 550, 35, 50, 20, m_hwnd, (HMENU)ID_BUTTON_SAVE, GetModuleHandle(NULL), NULL);
    hwndSaveButton2 = CreateWindow(L"BUTTON", L":", default_btn_style, 601, 35, 16, 20, m_hwnd, (HMENU)ID_BUTTON_SAVE_PATH, GetModuleHandle(NULL), NULL);
    hwndAboutButton = CreateWindow(L"BUTTON", L"About", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT, x_pos_about_btn, y_pos_about_btn, about_btn_width, about_btn_height, m_hwnd, (HMENU)ID_BUTTON_ABOUT, GetModuleHandle(NULL), NULL);


    // Edit Windows// x y width height
    hwndFilePath = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 10, 10, 530, 20, m_hwnd, (HMENU)ID_FILE_PATH, GetModuleHandle(NULL), NULL);
    hwndSavePath = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 10, 35, 530, 20, m_hwnd, (HMENU)ID_SAVE_PATH, GetModuleHandle(NULL), NULL);
    m_hwndAPIKey = CreateWindow(L"EDIT", m_apikey.c_str(), WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER | WS_BORDER, get_parent_middle_width(PARENT_WINDOW_WIDTH, (PARENT_WINDOW_WIDTH - 450)), 194, PARENT_WINDOW_WIDTH - 450, 16, m_hwnd, (HMENU)ID_EDIT_APIKEY, GetModuleHandle(NULL), NULL);


    // Static Windows
    hwndWaitingMsg = CreateWindow(L"STATIC", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER, get_parent_middle_width(PARENT_WINDOW_WIDTH, 200), 160, 200, 20, m_hwnd, (HMENU)ID_STATIC_MSG, GetModuleHandle(NULL), NULL);


        
    SendMessage(hwndAboutButton, WM_SETFONT, (WPARAM)hFontAboutButtonText, TRUE);
    SendMessage(m_hwndAPIKey, WM_SETFONT, (WPARAM)hFontSmall, TRUE);


    return TRUE;

}

// Creates new thread upon make_request() to prevent window becoming unresponsive/freezing.
void WindowState::StartMakeRequestThread()
{
    std::thread requestThread(&WindowState::make_request, this);
    requestThread.detach();
}

// Creates new thread upon save_output() and save_output_csv to prevent window becoming unresponsive/freezing.
void WindowState::StartSaveThread()
{
    std::thread saveThread(&WindowState::save_output_thread, this);
    saveThread.detach();
}

BOOL WindowState::CreateAboutWindow()
{

    if (aboutWindow)
    {
        return TRUE;
    }
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
    InitCommonControlsEx(&icex);

    aboutWindow = std::make_unique<WindowState>(m_hwnd, fileState, request, jsonParse);

    DWORD childStyle = WS_POPUP | WS_BORDER | WS_VISIBLE;
    RECT parentRect;

    GetWindowRect(m_hwnd, &parentRect);


    int centerX = parentRect.left + (PARENT_WINDOW_WIDTH / 2);
    int centerY = parentRect.top + (PARENT_WINDOW_HEIGHT / 2);

    int childWidth = 400;
    int childHeight = 140;

    int childX = centerX - (childWidth / 2);
    int childY = centerY - (childHeight / 2);

    const wchar_t* aboutText =
        L"FIGIMappingTool v1.1 (x64)\r\n\r\n"
        L"A lightweight tool for retrieving FIGI mappings from OpenFIGI's API using various equity identifiers.\r\n\r\n"
        L"License: MIT License\r\n"
        L"© 2024 by Michael Dakin-Green\r\n"
        L"<A HREF=\"https://github.com/mdgr122/FIGIMappingTool\">GitHub Repository</A>";



    if (aboutWindow->Create(L"", childStyle, 0, 50, 50, childWidth, childHeight, m_hwnd))
    {


        m_hwndAboutWindow = aboutWindow->Window();  // Store the handle to the child window
        SetWindowPos(m_hwndAboutWindow, HWND_TOP, childX, childY, childWidth, childHeight, SWP_NOSIZE | SWP_NOACTIVATE);


        RECT childRect;
        GetWindowRect(m_hwndAboutWindow, &childRect);
        int childBtnWidth = 16;
        int childBtnHeight = 14;

        int childBtnX = childWidth - childBtnWidth;
        int childBtnY = -1;

        //DWORD close_btn_style = WS_CHILD | WS_VISIBLE | BS_FLAT | SS_CENTER | BS_PUSHBUTTON;
        DWORD close_btn_style = WS_CHILD | WS_VISIBLE | BS_CENTER | BS_PUSHBUTTON;

        hwndCloseButton = CreateWindow(L"BUTTON", L"X", close_btn_style, childBtnX, childBtnY, childBtnWidth, childBtnHeight, m_hwndAboutWindow, (HMENU)ID_BUTTON_CLOSE, GetModuleHandle(NULL), NULL);

        hwndAboutText = CreateWindow(WC_LINK, aboutText, WS_CHILD | WS_VISIBLE | SS_LEFT, 2, 10, childWidth-2, childHeight - 20, m_hwndAboutWindow, (HMENU)ID_STATIC_ABOUT_MSG, GetModuleHandle(NULL), NULL);
        //hwndAboutText = CreateWindow(L"STATIC", aboutText, WS_CHILD | WS_VISIBLE | SS_LEFT, 2, 10, childWidth-2, childHeight - 20, m_hwndAboutWindow, (HMENU)ID_STATIC_ABOUT_MSG, GetModuleHandle(NULL), NULL);


        SendMessage(hwndAboutText, WM_SETFONT, (WPARAM)hFontAboutText, TRUE);
        
        return TRUE;
    }


    aboutWindow.reset();
    m_hwndAboutWindow = nullptr; // Clear childHwnd if creation failed
    return FALSE;
}



void WindowState::get_open_path()
{
    m_open_path = fileState.get_open_path();
    SetWindowText(this->hwndFilePath, Utils::GetInstance().strToWide(m_open_path).c_str());
}

void WindowState::get_save_path()
{
    m_save_path = fileState.get_save_path();
    SetWindowText(this->hwndSavePath, Utils::GetInstance().strToWide(m_save_path).c_str());
}

void WindowState::make_request()
{

    fileState.clear_data();
    fileState.read_file(m_open_path);

    request.ClearResponse();
    request.GetVec();
    request.GetIdentifierType();
    request.GetIdentifiers(); // Where the actual request is made
    PostMessage(m_hwnd, WM_MAKE_REQUEST_COMPLETE, 0, 0);

    //request.GetIdentifierType_Testing();
}

void WindowState::save_output()
{
    fileState.save_file(request.GetResponse(), m_save_path);
}

void WindowState::save_output_csv()
{
    jsonParse.read_json(request.GetResponse());
    fileState.save_csv_file(jsonParse.get_vec(), m_save_path);
}

void WindowState::save_output_thread()
{
    if (save_ftype_csv())
    {
        save_output_csv();
    }
    else
    {
        save_output();
    }

    // Notify the main thread that saving is complete
    PostMessage(m_hwnd, WM_SAVE_COMPLETE, 0, 0);
}

bool WindowState::save_ftype_csv()
{
    //std::string ext{};
    //bool flag = false;
    //for (size_t i = 0; i < m_save_path.size(); i++)
    //{
    //    char c = m_save_path[i];
    //    if (c == '.')
    //    {
    //        flag = true;
    //    }
    //    if (flag == true)
    //    {
    //        ext.push_back(c);
    //    }
    //}
    //if (ext == ".csv")
    //    return true;
    //return false;
    return m_save_path.size() >= 4 && m_save_path.compare(m_save_path.size() - 4, 4, ".csv") == 0;

}
