#include "WindowState.h"
#include <iostream>
#include <string>

// Define the window class name and window title at the top for consistency
const wchar_t CLASS_NAME[] = L"OpenFIGILookup";
const wchar_t ABOUT_CLASS_NAME[] = L"AboutPopupWinudow";
const wchar_t WINDOW_TITLE[] = L"OpenFIGI Lookup";

WindowState::WindowState(HWND hParent, FileState& fileState, Request& request, JsonParse& jsonParse) 
    : m_hParent(hParent)
    , fileState(fileState)
    , request(request)
    , jsonParse(jsonParse)
    , childHwnd{nullptr}
    , childWindow{ nullptr }
{
    nWidth = GetSystemMetrics(SM_CXSCREEN);
    nHeight = GetSystemMetrics(SM_CYSCREEN);
}

WindowState::~WindowState()
{

}

PCWSTR WindowState::ClassName() const
{
    return L"OpenFIGILookup";
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
            if (m_save_path.empty())
            {
                SetWindowText(hwndWaitingMsg, L"");
                SetWindowText(hwndWaitingMsg, L"Save Path Empty!");
                break;
            }

            if ((request.GetResponse()).empty())
            {
                SetWindowText(hwndWaitingMsg, L"");
                SetWindowText(hwndWaitingMsg, L"Nothing to Save");
                break;
            }
            if (save_ftype_csv())
            {
                save_output_csv();
                break;
            }
            else
            {
                save_output();
                break;
            }
            SetWindowText(hwndWaitingMsg, L"");
            SetWindowText(hwndWaitingMsg, L"File Saved");
            break;
        }
        else if (LOWORD(wParam) == ID_BUTTON_REQUEST)
        {
            bool flag = true;

            if (m_open_path.empty())
            {
                SetWindowText(hwndWaitingMsg, L"");
                SetWindowText(hwndWaitingMsg, L"Input Path Empty!");
                break;
            }
            while (flag)
            {
                SetWindowText(hwndWaitingMsg, L"Processing...");
                make_request();
                flag = false;
            }
            SetWindowText(hwndWaitingMsg, L"");
            SetWindowText(hwndWaitingMsg, L"Complete!");
            break;
        }
        else if (LOWORD(wParam) == ID_BUTTON_ABOUT)
        {
            CreateChildWindow();
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
            break;
        }
        break;
    }
    case WM_APP_CHILD_CLOSED:
    {
        // Reset the unique_ptr as the child window is closed
        childWindow.reset();
        return 0;
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
        if (m_hParent != NULL) // if m_hParent == NULL, that means m_hwnd == m_hParent == NULL
        {
            DestroyWindow(m_hwnd); // Close only the child window
            InvalidateRect(m_hParent, NULL, TRUE);
            UpdateWindow(m_hParent);
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
        if (m_hParent == NULL) // Only post quit message for the main window
        {
            PostQuitMessage(0); // End the application
        }
        else
        {
            // Notify the parent that the child is closing
            PostMessage(m_hParent, WM_APP_CHILD_CLOSED, 0, 0);
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
    if (!Create(L"OpenFIGILookup", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX))
    {
        return FALSE; // If creation fails, return FALSE
    }



    int xPos = (nWidth - PARENT_WINDOW_WIDTH) / 2;
    int yPos = (nHeight - PARENT_WINDOW_HEIGHT) / 2;
    DWORD default_btn_style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON;

    // Center the window on the screen
    SetWindowPos(m_hwnd, NULL, xPos, yPos, PARENT_WINDOW_WIDTH, PARENT_WINDOW_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);


    // Button Windows
    hwndRequestButton = CreateWindow(L"BUTTON", L"Request", default_btn_style, 2, 100, 630, 50, m_hwnd, (HMENU)ID_BUTTON_REQUEST, GetModuleHandle(NULL), NULL);
    hwndFileButton = CreateWindow(L"BUTTON", L"File", default_btn_style, 550, 10, 50, 20, m_hwnd, (HMENU)ID_BUTTON_FILE_PATH, GetModuleHandle(NULL), NULL);
    hwndSaveButton = CreateWindow(L"BUTTON", L"Save", default_btn_style, 550, 50, 50, 20, m_hwnd, (HMENU)ID_BUTTON_SAVE, GetModuleHandle(NULL), NULL);
    hwndSaveButton2 = CreateWindow(L"BUTTON", L":", default_btn_style, 601, 50, 16, 20, m_hwnd, (HMENU)ID_BUTTON_SAVE_PATH, GetModuleHandle(NULL), NULL);
    hwndAboutButton = CreateWindow(L"BUTTON", L"About", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT, 2, 190, 36, 14, m_hwnd, (HMENU)ID_BUTTON_ABOUT, GetModuleHandle(NULL), NULL);


    // Path windows
    hwndFilePath = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 10, 10, 530, 20, m_hwnd, (HMENU)ID_FILE_PATH, GetModuleHandle(NULL), NULL);
    hwndSavePath = CreateWindow(L"EDIT", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, 10, 50, 530, 20, m_hwnd, (HMENU)ID_SAVE_PATH, GetModuleHandle(NULL), NULL);

    // Static Message Window
    hwndWaitingMsg = CreateWindow(L"STATIC", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER, get_parent_middle_width(PARENT_WINDOW_WIDTH, 200), 175, 200, 20, m_hwnd, (HMENU)ID_STATIC_MSG, GetModuleHandle(NULL), NULL);


    HFONT hFontAbout = CreateFont(
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
    L"Microsoft Sans Serif");              // Font family name ("" or `NULL` for system default font)
        
    SendMessage(hwndAboutButton, WM_SETFONT, (WPARAM)hFontAbout, TRUE);

    return TRUE;

}

BOOL WindowState::CreateChildWindow()
{

    if (childWindow)
    {
        return TRUE;
    }

    childWindow = std::make_unique<WindowState>(m_hwnd, fileState, request, jsonParse);

    DWORD childStyle = WS_POPUP | WS_BORDER | WS_VISIBLE;
    RECT parentRect;

    GetWindowRect(m_hwnd, &parentRect);


    int centerX = parentRect.left + (PARENT_WINDOW_WIDTH / 2);
    int centerY = parentRect.top + (PARENT_WINDOW_HEIGHT / 2);

    int childWidth = 400;
    int childHeight = 200;

    int childX = centerX - (childWidth / 2);
    int childY = centerY - (childHeight / 2);

    const wchar_t* aboutText =
        L"OpenFIGI API Tool\r\n"
        L"Version: 0.4\r\n\r\n"
        L"Description:\r\n"
        L"A lightweight tool for retrieving OpenFIGI API mappings from various equity identifiers.\r\n\r\n"
        L"Developed by: Michael Dakin-Green\r\n"
        L"License: MIT License\r\n"
        L"Contact: michael.dakingreen@spglobal.com";



    if (childWindow->Create(L"", childStyle, 0, 50, 50, childWidth, childHeight, m_hwnd))
    {


        childHwnd = childWindow->Window();  // Store the handle to the child window
        SetWindowPos(childHwnd, HWND_TOPMOST, childX, childY, childWidth, childHeight, SWP_NOSIZE | SWP_NOACTIVATE);


        RECT childRect;
        GetWindowRect(childHwnd, &childRect);
        int childBtnWidth = 16;
        int childBtnHeight = 14;

        int childBtnX = childWidth - childBtnWidth;
        int childBtnY = -1;

        hwndCloseButton = CreateWindow(L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | BS_FLAT | SS_CENTER, childBtnX, childBtnY, childBtnWidth, childBtnHeight, childHwnd, (HMENU)ID_BUTTON_CLOSE, GetModuleHandle(NULL), NULL);
        HWND hwndAboutText = CreateWindow(L"STATIC", aboutText, WS_CHILD | WS_VISIBLE | SS_CENTER, 1, 10, childWidth, childHeight - 40, childHwnd, (HMENU)ID_STATIC_ABOUT_MSG, GetModuleHandle(NULL), NULL);

        HFONT hFont = CreateFont(
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

        SendMessage(hwndAboutText, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        return TRUE;
    }


    childWindow.reset();
    childHwnd = nullptr; // Clear childHwnd if creation failed
    return FALSE;
}


int WindowState::get_parent_middle_width(int parent_width, int child_width)
{
    return ((parent_width - child_width) / 2);
}


void WindowState::get_open_path()
{
    //FileState fileState;
    m_open_path = fileState.get_open_path();
    //SetWindowText(this->hwndFilePath, stringToWideString(m_open_path).c_str());
    SetWindowText(this->hwndFilePath, Utils::GetInstance().strToWide(m_open_path).c_str());
    // fileState.read_file();

}

void WindowState::get_save_path()
{
    //FileState fileState;
    m_save_path = fileState.get_save_path();
    //SetWindowText(this->hwndSavePath, stringToWideString(m_save_path).c_str());
    SetWindowText(this->hwndSavePath, Utils::GetInstance().strToWide(m_save_path).c_str());
    //fileState.read_file();
    //fileState.save_file(request.GetResponse());

}

void WindowState::make_request()
{
    //GetWindowText(this->hwndFilePath, stringToWideString(m_open_path).c_str());
    //if (!m_open_path.empty())
    //{
    //    fileState.read_file(m_open_path);
    //    request.GetVec();
    //    request.GetIdentifierType();
    //    request.GetIdentifiers(); // Where the actual request is made
    //    return true;
    //}
    //return false;
    if (!(request.GetResponse()).empty())
    {
        request.ClearResponse();
    }
    fileState.read_file(m_open_path);
    request.GetVec();
    request.GetIdentifierType();
    request.GetIdentifiers(); // Where the actual request is made
}

void WindowState::save_output()
{
    //GetWindowText(this->hwndSavePath, stringToWideString(m_save_path).c_str());

    fileState.save_file(request.GetResponse(), m_save_path);
}

void WindowState::save_output_csv()
{
    jsonParse.read_json(request.GetResponse());
    fileState.save_csv_file(jsonParse.get_vec(), m_save_path);
}

bool WindowState::save_ftype_csv()
{
    std::string ext{};
    bool flag = false;
    for (size_t i = 0; i < m_save_path.size(); i++)
    {
        char c = m_save_path[i];
        if (c == '.')
        {
            flag = true;
        }
        if (flag == true)
        {
            ext.push_back(c);
        }
    }
    if (ext == ".csv")
        return true;
    return false;
}