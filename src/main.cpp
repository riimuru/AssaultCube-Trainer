#include "../include/includes.h"

HWND hwnd;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const MARGINS margin = {0, 0, render.RenderWidth, render.RenderHeight};

    switch (message)
    {

    case WM_PAINT:
        DwmExtendFrameIntoClientArea(hWnd, &margin);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND WindowHWND;

    // Define Window Name
    auto window_name = L"AssaultCube";

    // Find Window
    WindowHWND = FindWindow(NULL, window_name);

    RECT wndRc;

    if (WindowHWND)
    {
        GetWindowRect(WindowHWND, &wndRc);
    }
    else
    {
        // Failed to Find Target.
        ExitProcess(0);
    }

    render.RenderWidth = wndRc.right - wndRc.left;
    render.RenderHeight = wndRc.bottom - wndRc.top;

    auto centerX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (render.RenderWidth / 2);
    auto centerY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (render.RenderHeight / 2);

    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    hwnd = CreateWindowEx(0, L"WindowClass", L"", WS_EX_TOPMOST | WS_POPUP, wndRc.left, wndRc.top, render.RenderWidth, render.RenderHeight, NULL, NULL, hInstance, NULL);

    SetWindowLong(hwnd, GWL_EXSTYLE, (int)GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, ULW_COLORKEY | LWA_ALPHA);

    ShowWindow(hwnd, nCmdShow);

    render.initD3D(hwnd);

    MSG msg;

    SetWindowPos(FindWindow(NULL, window_name), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    WindowHWND = FindWindow(NULL, window_name);

    if (!WindowHWND)
    {
        ExitProcess(0);
    }

    while (true)
    {

        GetWindowRect(WindowHWND, &wndRc);

        render.RenderWidth = wndRc.right - wndRc.left;
        render.RenderHeight = wndRc.bottom - wndRc.top;

        MoveWindow(hwnd, wndRc.left, wndRc.top, render.RenderWidth, render.RenderHeight, true);

        SetWindowPos(hwnd, HWND_TOPMOST, wndRc.left, wndRc.top, render.RenderWidth, render.RenderHeight, SWP_NOMOVE | SWP_NOSIZE);

        render.render();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
        {
            exit(0);
        }
    }

    render.cleanD3D();

    return msg.wParam;
}