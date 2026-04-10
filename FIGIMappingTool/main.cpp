#include <iostream>
#include <Windows.h>
#include "core/OpenFigiClient.h"
#include "states/WindowState.h"
#include "states/FileState.h"


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{

    FileState fileState;
    figi::OpenFigiClient client;

    HWND hwnd = nullptr;
    WindowState win(hwnd, fileState, &client);
    if (!win.CreateParentWindow())
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
