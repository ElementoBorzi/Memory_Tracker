#include <Windows.h>
#include <psapi.h>
#include <string>

// Прототип функции обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Функция получения информации об использовании памяти
void GetMemoryInfo(DWORDLONG& totalMemory, DWORDLONG& usedMemory, DWORDLONG& freeMemory);

// Обработчики элементов управления
void OnPaint(HWND hwnd);
void OnTimer(HWND hwnd);

// Идентификатор таймера
constexpr UINT_PTR TIMER_ID = 1;

// Глобальные переменные для хранения информации об использовании памяти
DWORDLONG g_TotalMemory = 0;
DWORDLONG g_UsedMemory = 0;
DWORDLONG g_FreeMemory = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    const char* className = "MemoryMonitorWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    RegisterClass(&wc);

    // Создание окна
    HWND hwnd = CreateWindowEx(0, className, "Memory Monitor", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 300, 150, NULL, NULL, hInstance, NULL);

    // Отображение окна
    ShowWindow(hwnd, nCmdShow);

    // Установка таймера для обновления информации об использовании памяти
    SetTimer(hwnd, TIMER_ID, 1000, NULL);

    // Цикл обработки сообщений окна
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Уничтожение таймера при выходе
    KillTimer(hwnd, TIMER_ID);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            OnPaint(hwnd);
            return 0;
        case WM_TIMER:
            OnTimer(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void GetMemoryInfo(DWORDLONG& totalMemory, DWORDLONG& usedMemory, DWORDLONG& freeMemory) {
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);

    totalMemory = memoryStatus.ullTotalPhys;
    usedMemory = totalMemory - memoryStatus.ullAvailPhys;
    freeMemory = memoryStatus.ullAvailPhys;
}

void OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    std::string text = "Total Memory: " + std::to_string(g_TotalMemory / (1024 * 1024)) + " MB\n"
                       "Used Memory: " + std::to_string(g_UsedMemory / (1024 * 1024)) + " MB\n"
                       "Free Memory: " + std::to_string(g_FreeMemory / (1024 * 1024)) + " MB";

    RECT rect;
    GetClientRect(hwnd, &rect);

    DrawText(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    EndPaint(hwnd, &ps);
}

void OnTimer(HWND hwnd) {
    GetMemoryInfo(g_TotalMemory, g_UsedMemory, g_FreeMemory);
    InvalidateRect(hwnd, NULL, TRUE);
}
