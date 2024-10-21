#include "AppWindow.h"
#include <iostream>

// Define the window class name and window title at the top for consistency
const wchar_t CLASS_NAME[] = L"OpenFIGILookup";
const wchar_t WINDOW_TITLE[] = L"OpenFIGI Lookup";

AppWindow::AppWindow(HINSTANCE hInstance, int nCmdShow)
    : hInstance(hInstance)
    , hwnd(nullptr)
    , nCmdShow(nCmdShow)
{
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

AppWindow::~AppWindow()
{
}

bool AppWindow::RegisterWindowClass()
{
    // WNDCLASSW Struct - Contains the window class attributes that are registered by the RegisterClass function.
    WNDCLASS wc = { };

    // When we assign AppWindow::WindowProc to wc.lpfnWndProc, we are taking the address of the function (implicitly) and storing it in a variable of type WNDPROC.
    // In C++, functions decay into pointers to themselves when assigned or passed as arguments, so the & is implied. Thus, the compiler treats AppWindow::WindowProc as a pointer to the function.
    // In other words, because functions decay into pointers when assigned or passed as args, we do not need to assign the pointer to the address-of AppWindow::WindowProc function, but we can if we want.
    // I.e., we can have wc.lpfnWndProc = &AppWindow::WindowProc; instead of wc.lpfnWndProc = AppWindow::WindowProc;
    wc.lpfnWndProc = AppWindow::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        std::cerr << "Failed to register the window class! Error: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool AppWindow::CreateMainWindow()
{
    
    hwnd = CreateWindowEx(
        0,                      // Optional window styles.
        CLASS_NAME,             // Window class
        WINDOW_TITLE,           // Window text
        WS_OVERLAPPEDWINDOW,    // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,               // Parent window    
        NULL,               // Menu
        hInstance,          // Instance handle
        NULL                // Additional application data
    );

    if (hwnd == NULL)
    {
        std::cerr << "Failed to create the main window! Error: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

// LRESULT - Signed result of message processing.
// This function matches the signature required by a function pointer of type WNDPROC. 
// When we assign lpfnWndProc above, the function's address is implicitly converted to a pointer of type WNDPROC.
LRESULT CALLBACK AppWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // All painting occurs here, between BeginPaint and EndPaint.
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
    return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int AppWindow::RunMsgLoop()
{
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}
