#pragma once

#include <Windows.h>
#include <Windowsx.h>
#include <commctrl.h>

template <class DERIVED_TYPE>
class BaseWindow
{
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DERIVED_TYPE* pThis = nullptr;

        if (uMsg == WM_NCCREATE)
        {
            // CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = static_cast<DERIVED_TYPE*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            //SetWindowLongPtr(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(pThis));

            pThis->m_hwnd = hwnd;
        }
        else
        {
            //pThis = static_cast<DERIVED_TYPE*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            pThis = reinterpret_cast<DERIVED_TYPE*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    BaseWindow()
        : m_hwnd{nullptr} {}

    BOOL Create(PCWSTR lpWindowName,
                DWORD dwStyle,
                DWORD dwExStyle = 0,
                int x = CW_USEDEFAULT,
                int y = CW_USEDEFAULT,
                int nWidth = CW_USEDEFAULT,
                int nHeight = CW_USEDEFAULT,
                HWND hWndParent = 0,
                HMENU hMenu = 0)
    {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = ClassName();

        RegisterClass(&wc);

        m_hwnd = CreateWindowEx(dwExStyle,
                                ClassName(),
                                lpWindowName,
                                dwStyle,
                                x,
                                y,
                                nWidth,
                                nHeight,
                                hWndParent,
                                hMenu,
                                GetModuleHandle(nullptr),
                                this);

        return (m_hwnd ? TRUE : FALSE);
    }

    HWND Window() const { return m_hwnd; }

protected:
    virtual PCWSTR ClassName() const = 0;
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND m_hwnd;

};
