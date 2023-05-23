#include <Windows.h>
#include <psapi.h>
#include <string>
#include <vector>

// Прототип функции обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Функция получения информации об использовании памяти
void GetMemoryInfo(DWORDLONG& totalMemory, DWORDLONG& usedMemory, DWORDLONG& freeMemory);

// Обработчики элементов управления
void OnPaint(HWND hwnd);
void OnTimer(HWND hwnd);
void OnCopy(HWND hwnd);

// Идентификатор таймера
constexpr UINT_PTR TIMER_ID = 1;
// Идентификатор кнопки "Скопировать"
constexpr UINT_PTR BUTTON_COPY_ID = 2;

// Глобальные переменные для хранения информации об использовании памяти
DWORDLONG g_TotalMemory = 0;
DWORDLONG g_UsedMemory = 0;
DWORDLONG g_FreeMemory = 0;

// Хранит скопированную информацию о памяти
std::vector<std::string> g_CopiedMemoryInfo;

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
                               CW_USEDEFAULT, CW_USEDEFAULT, 500, 150, NULL, NULL, hInstance, NULL);

    // Создание кнопки "Скопировать"
    HWND buttonCopy = CreateWindow("BUTTON", "Copy", WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                   10, 10, 80, 30, hwnd, (HMENU)BUTTON_COPY_ID, hInstance, NULL);

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
        case WM_COMMAND:
            if (LOWORD(wParam) == BUTTON_COPY_ID) {
                OnCopy(hwnd);
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

    std::string text = "Total Memory: " + std::to_string(g_TotalMemory / (1024 * 1024)) + " MB\r\n";
    text += "Used Memory: " + std::to_string(g_UsedMemory / (1024 * 1024)) + " MB\r\n";
    text += "Free Memory: " + std::to_string(g_FreeMemory / (1024 * 1024)) + " MB";

    RECT rect;
    GetClientRect(hwnd, &rect);

    int textLength = text.length();
    int lineHeight = DrawText(hdc, text.c_str(), textLength, &rect, DT_CALCRECT | DT_LEFT | DT_BOTTOM | DT_WORDBREAK);

    int textHeight = rect.bottom - rect.top;
    rect.top = rect.bottom - textHeight;

    DrawTextEx(hdc, const_cast<char*>(text.c_str()), textLength, &rect, DT_LEFT | DT_BOTTOM | DT_WORDBREAK, NULL);

    EndPaint(hwnd, &ps);
}



void OnTimer(HWND hwnd) {
    GetMemoryInfo(g_TotalMemory, g_UsedMemory, g_FreeMemory);
    InvalidateRect(hwnd, NULL, TRUE);
}

void OnCopy(HWND hwnd) {
    std::string memoryInfo = "Total Memory: " + std::to_string(g_TotalMemory / (1024 * 1024)) + " MB\n";
    memoryInfo += "Used Memory: " + std::to_string(g_UsedMemory / (1024 * 1024)) + " MB\n";
    memoryInfo += "Free Memory: " + std::to_string(g_FreeMemory / (1024 * 1024)) + " MB";

    // Добавляем информацию в вектор
    g_CopiedMemoryInfo.push_back(memoryInfo);

    // Копируем информацию в буфер обмена
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();

        HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, memoryInfo.size() + 1);
        if (hMemory) {
            char* pMemory = (char*)GlobalLock(hMemory);
            if (pMemory) {
                memcpy(pMemory, memoryInfo.c_str(), memoryInfo.size() + 1);
                GlobalUnlock(hMemory);
                SetClipboardData(CF_TEXT, hMemory);
            }
        }

        CloseClipboard();
    }
}
