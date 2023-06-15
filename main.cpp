#include "window/window.h"

LRESULT RenderWindow::handleMsg(UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pointerFactory)))
            return -1;  // Fail CreateWindowEx.

        return 0;
    case WM_DESTROY:
        discardGraphicsResources();
        safeRelease(&pointerFactory);
        PostQuitMessage(0);
        return 0;
        
    case WM_PAINT:
        onPaint();
        return 0;

    case WM_SIZE:
        resize();
        return 0;

    default:
        return DefWindowProc(hwnd_, msg, wparam, lparam);
    }
    return TRUE;
}

int main()
{
    RenderWindow window;
    
    if (!window.create(L"Window Label")) {
        // error
        return 1;
    }

    ShowWindow(window.windowHandle(), 1);

    MSG msg = { 0 };

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

    return 0;
}
