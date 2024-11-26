// FIGIMappingTool.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <Windows.h>
#include "Request.h"
#include "states/WindowState.h"
#include "states/FileState.h"
#include "utilities/jsonparse.h"


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{

    FileState fileState;
    Request request(fileState);
    JsonParse jsonParse;
    HWND hwnd = NULL;
    
    
    WindowState win(hwnd, fileState, request, jsonParse);

    //if (AllocConsole()) {
    //    FILE* file;
    //    freopen_s(&file, "CONOUT$", "w", stdout);
    //    std::cout << "Console logging initialized.\n";
    //}
  
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



    //FreeConsole();
    return 0;
}
