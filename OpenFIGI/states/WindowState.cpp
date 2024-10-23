#include "WindowState.h"
#include <iostream>

// Define the window class name and window title at the top for consistency
const wchar_t CLASS_NAME[] = L"OpenFIGILookup";
const wchar_t WINDOW_TITLE[] = L"OpenFIGI Lookup";

WindowState::WindowState(HINSTANCE hInstance, int nCmdShow)
    : hInstance(hInstance)
    , hwnd(nullptr)
    , nCmdShow(nCmdShow)
    , nWidth{}
    , nHeight{}
    , file_state{hwnd}
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
        ShowWindow(hwnd, nCmdShow);

}

WindowState::~WindowState()
{
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
    
    HWND hwndButton = CreateWindowEx(
        0,                      // Optional window styles.
        L"BUTTON",              // Window class
        L"File",                // Window text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,    // Window style
        550,                    // X
        10,                     // Y
        50,                     // nWidth
        25,                     // nHeight
        hwnd,                   // Parent window    
        (HMENU) ID_BUTTON_FILE,                   // Menu
        hInstance,              // Instance handle
        NULL
    );

    if (hwnd == NULL)
    {
        std::cerr << "Failed to create the main window! Error: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

HWND WindowState::get_handle()
{
    if (!hwnd)
    {
        std::cerr << "Failed to get window handle!" << std::endl;
        return NULL;
    }
    return hwnd;
}


// LRESULT - Signed result of message processing.
// This function matches the signature required by a function pointer of type WNDPROC. 
// When we assign lpfnWndProc above, the function's address is implicitly converted to a pointer of type WNDPROC.
LRESULT CALLBACK WindowState::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (wParam)
        {
        case ID_BUTTON_FILE:
            MessageBox(hwnd, L"File button clicked!", L"Button Clicked", MB_OK);
            
            break;
        }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

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

int WindowState::RunMsgLoop()
{
    MSG msg = { }; // Contains message information from a thread's message queue.

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
