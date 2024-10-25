#include "WindowState.h"
#include <iostream>
#include <string>

// Define the window class name and window title at the top for consistency
const wchar_t CLASS_NAME[] = L"OpenFIGILookup";
const wchar_t WINDOW_TITLE[] = L"OpenFIGI Lookup";

WindowState::WindowState(HINSTANCE hInstance, int nCmdShow, FileState& fileState, Request& request)
    : hInstance(hInstance)
    , hwnd{nullptr}
    , hwndFileButton{nullptr}
    , hwndRequestButton{nullptr}
    , hwndSaveButton{nullptr}
    , hwndSaveButton2{nullptr}
    , hwndFilePath{nullptr}
    , hwndSavePath{nullptr}
    , hdcEdit{nullptr}
    , hbrBackground{nullptr}
    , nCmdShow(nCmdShow)
    , nWidth{}
    , nHeight{}
    , m_open_path{}
    , m_save_path{}
    , fileState(fileState)
    , request(request)
{


    nWidth = GetSystemMetrics(SM_CXSCREEN);
    nHeight = GetSystemMetrics(SM_CYSCREEN);

    if (!RegisterWindowClass())
    {
        std::cerr << "Failed to register the window class!" << std::endl;
        return;
    }

    if (!CreateMainWindow())
    {
        std::cerr << "Failed to create the main window!" << std::endl;
        return;
    }

    if (hwnd != NULL)
    {

        ShowWindow(hwnd, nCmdShow);
        RunMsgLoop();
    }

}

WindowState::~WindowState()
{
}

// LRESULT - Signed result of message processing.
// This function matches the signature required by a function pointer of type WNDPROC. 
// When we assign lpfnWndProc above, the function's address is implicitly converted to a pointer of type WNDPROC.
LRESULT CALLBACK WindowState::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Because LRESULT CALLBACK WindowState::WindowProc is a static function, and the other functions within the WindowState class are non-static, there is an inherent mismatch.
    // Because of this, callback functions don't have direct access to the WindowState instance and we mus tstore the instance pointer in the window data.
    // This is done using  SetWindowLongPTR (SetWindowLong on 32bit). The GWLP_USERDATA index is used to store a pointer to the WindowState object in the window's internal data.
    // The pointer is retrieved with the GetWindowLongPtr which allows the callback function to gain acess to the instance of the class.
    // Once the instance is retrieved, we can use it to call non-static member functions and access instance-specific data.

    // Retrieve the instance of WindowState from the window's user data
    WindowState* pThis = reinterpret_cast<WindowState*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    HWND hEditControl = HWND(lParam);

    if (pThis)
    {
        switch (uMsg)
        {
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case ID_BUTTON_FILE_PATH:
            {
                pThis->get_open_path(); // Calling non-static member function
                break;
            }
            case ID_BUTTON_SAVE_PATH:
            {
                pThis->get_save_path();
                break;
            }
            case ID_BUTTON_SAVE:
            {
                pThis->save_output();
                break;
            }
            case ID_BUTTON_REQUEST:
            {
                pThis->make_request();
                break;
            }
            }
            switch (HIWORD(wParam))
            {
                case EN_KILLFOCUS:
                {
                    switch (LOWORD(wParam))
                    {
                    case ID_FILE_PATH:
                    {
                        wchar_t buffer[256];
                        GetWindowText(hEditControl, buffer, 256);
                        //pThis->m_open_path = std::to_string(buffer);
                        pThis->m_open_path = pThis->WideToStr(buffer);
                        std::wcout << buffer << " Update File Path" << std::endl;
                        std::cout << pThis->m_open_path << std::endl;
                        break;
                    }
                    case ID_SAVE_PATH:
                    {
                        std::cout << "Do something else" << std::endl;
                        break;
                    }
                    }
                    break;
                }
            }
            break;
        }
        case WM_CREATE:
            // Create a solid brush with your desired color, e.g., light gray.
            pThis->hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
            break;
        case WM_CTLCOLOREDIT:
        {   
            //HWND hEditControl = HWND(lParam);
            pThis->hdcEdit = (HDC)wParam;   // Handle to the device context of the control
            switch (wParam)
            {
            case EN_CHANGE:
                std::cout << "Do nothing" << std::endl;
            }
            if (hEditControl == pThis->hwndFilePath)
            {
                // Set the text and background colors
                SetTextColor(pThis->hdcEdit, RGB(0, 0, 0));        // Text color (black)
                SetBkColor(pThis->hdcEdit, RGB(169, 169, 169));    // Background color (light gray)
            }
            else if (hEditControl == pThis->hwndSavePath)
            {
                // Set the text and background colors
                SetTextColor(pThis->hdcEdit, RGB(0, 0, 0));        // Text color (black)
                SetBkColor(pThis->hdcEdit, RGB(169, 169, 169));    // Background color (light gray)
            }


            // Return the background brush to paint the control's background
            return (INT_PTR)pThis->hbrBackground;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;

            // The BeginPaint function fills in the PAINTSTRUCT structure with information on the repaint request. The current update region is given in the rcPaint member of PAINTSTRUCT.
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.
            // &ps.rcPaint is the coords of the rectangle to fill.
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

            // The EndPaint function clears the update region, which signals to Windows that the window has completed painting itself.
            EndPaint(hwnd, &ps);
        }
        return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool WindowState::RegisterWindowClass()
{
    // WNDCLASSW Struct - Contains the window class attributes that are registered by the RegisterClass function.
    WNDCLASS wc = { };

    // When we assign WindowState::WindowProc to wc.lpfnWndProc, we are taking the address of the function (implicitly) and storing it in a variable of type WNDPROC.
    // In C++, functions decay into pointers to themselves when assigned or passed as arguments, so the & is implied. Thus, the compiler treats WindowState::WindowProc as a pointer to the function.
    // In other words, because functions decay into pointers when assigned or passed as args, we do not need to assign the pointer to the address-of WindowState::WindowProc function, but we can if we want.
    // I.e., we can have wc.lpfnWndProc = &WindowState::WindowProc; instead of wc.lpfnWndProc = WindowState::WindowProc;
    wc.lpfnWndProc = WindowState::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        std::cerr << "Failed to register the window class! Error: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool WindowState::CreateMainWindow()
{
    //HWND CreateWindowExA(
    //    [in]           DWORD     dwExStyle,
    //    [in, optional] LPCSTR    lpClassName,
    //    [in, optional] LPCSTR    lpWindowName,
    //    [in]           DWORD     dwStyle,
    //    [in]           int       X,
    //    [in]           int       Y,
    //    [in]           int       nWidth,
    //    [in]           int       nHeight,
    //    [in, optional] HWND      hWndParent,
    //    [in, optional] HMENU     hMenu,
    //    [in, optional] HINSTANCE hInstance,
    //    [in, optional] LPVOID    lpParam
    //);

    hwnd = CreateWindowEx(
        0,                      // Optional window styles.
        CLASS_NAME,             // Window class
        WINDOW_TITLE,           // Window text
        WS_OVERLAPPEDWINDOW,    // Window style
        CW_USEDEFAULT,          // X
        CW_USEDEFAULT,          // Y
        PARENT_WINDOW_WIDTH,          // nWidth
        PARENT_WINDOW_HEIGHT,          // nHeight
        NULL,                   // Parent window    
        NULL,                   // Menu
        hInstance,              // Instance handle
        NULL                    // Additional application data
    );
    
    hwndFileButton = CreateWindowEx(
        0,                      // Optional window styles.
        L"BUTTON",              // Window class
        L"File",                // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,    // Window style
        550,                    // X
        10,                     // Y
        50,                     // nWidth
        20,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU)ID_BUTTON_FILE_PATH, // Menu
        hInstance,              // Instance handle
        NULL
    );

    hwndFilePath = CreateWindowEx(
        0,                      // Optional window styles.
        L"EDIT",                // Window class
        L"",                    // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER  | ES_LEFT | ES_AUTOHSCROLL,    // Window style
        10,                     // X
        10,                     // Y
        530,                    // nWidth
        20,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU) ID_FILE_PATH,   // Menu
        hInstance,//(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),              // Instance handle
        NULL
    );

    hwndRequestButton = CreateWindowEx(
        0,                      // Optional window styles.
        L"BUTTON",              // Window class
        L"Request",                // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,    // Window style
        225,                    // X
        100,                     // Y
        75,                     // nWidth
        75,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU)ID_BUTTON_REQUEST, // Menu
        hInstance,              // Instance handle
        NULL
    );

    hwndSaveButton = CreateWindowEx(
        0,                      // Optional window styles.
        L"BUTTON",              // Window class
        L"Save",                // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,    // Window style
        550,                    // X
        50,                     // Y
        50,                     // nWidth
        20,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU)ID_BUTTON_SAVE,                   // Menu
        hInstance,              // Instance handle
        NULL
    );
    
    hwndSaveButton2 = CreateWindowEx(
        0,                      // Optional window styles.
        L"BUTTON",              // Window class
        L":",                // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,    // Window style
        601,                    // X
        50,                     // Y
        10,                     // nWidth
        20,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU)ID_BUTTON_SAVE_PATH,                   // Menu
        hInstance,              // Instance handle
        NULL
    );

    hwndSavePath = CreateWindowEx(
        0,                      // Optional window styles.
        L"EDIT",                // Window class
        L"",                    // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,    // Window style
        10,                     // X
        50,                     // Y
        530,                    // nWidth
        20,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU)ID_SAVE_PATH,   // Menu
        hInstance,              // Instance handle
        NULL
    );

    // Store the pointer to the current instance in the HWND
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (hwnd == NULL)
    {
        std::cerr << "Failed to create the main window! Error: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}



int WindowState::RunMsgLoop()
{
    MSG msg = { }; // Contains message information from a thread's message queue.

    //BOOL GetMessage(
    //    [out]          LPMSG lpMsg,
    //    [in, optional] HWND  hWnd,
    //    [in]           UINT  wMsgFilterMin,
    //    [in]           UINT  wMsgFilterMax
    //);

    // GetMessage: Retrieves a message from the calling thread's message queue. The function dispatches incoming sent messages until a posted message is available for retrieval.
    // This function removes the first message from the head of the queue. If the queue is empty, the function blocks until another message is queued.
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        // The TranslateMessage function is related to keyboard input. It translates keystrokes (key down, key up) into characters. 
        TranslateMessage(&msg); // You do not really have to know how this function works; just remember to call it before DispatchMessage. 
        
        // The DispatchMessage function tells the operating system to call the window procedure of the window that is the target of the message. 
        // In other words, the operating system looks up the window handle in its table of windows, finds the function pointer associated with the window, and invokes the function.
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void WindowState::get_open_path()
{
    //FileState fileState;
    m_open_path = fileState.get_open_path();
    SetWindowText(this->hwndFilePath, stringToWideString(m_open_path).c_str());
    // fileState.read_file();

}

void WindowState::get_save_path()
{
    //FileState fileState;
    m_save_path = fileState.get_save_path();
    SetWindowText(this->hwndSavePath, stringToWideString(m_save_path).c_str());
    //fileState.read_file();
    //fileState.save_file(request.GetResponse());

}

void WindowState::make_request()
{
    //GetWindowText(this->hwndFilePath, stringToWideString(m_open_path).c_str());
    fileState.read_file(m_open_path);
    request.GetVec();
    request.GetIdentifierType();
    request.GetIdentifiers();
}

void WindowState::save_output()
{
    //GetWindowText(this->hwndSavePath, stringToWideString(m_save_path).c_str());
    fileState.save_file(request.GetResponse());
}

std::wstring WindowState::stringToWideString(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

HWND WindowState::GetHWND() const
{
    return hwnd;
}

std::string WindowState::WideToStr(const std::wstring& wstr)
{
    // Uses windows function
    int size_in_bytes = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_in_bytes, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_in_bytes, NULL, NULL);
    return strTo;
}