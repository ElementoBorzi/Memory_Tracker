#include <Windows.h>
#include <psapi.h>
#include <string>
#include <vector>

// Прототип функции обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Функция получения информации об использовании памяти
void GetMemoryInfo(DWORDLONG& totalMemory, DWORDLONG& usedMemory, DWORDLONG& freeMemory);
void GetMemoryClockSpeed(DWORD& memoryClockSpeed);

// Обработчики элементов управления
void OnPaint(HWND hwnd);
void OnTimer(HWND hwnd);
void OnCopy(HWND hwnd);
void OnGitHub(HWND hwnd);

// Идентификатор таймера
constexpr UINT_PTR TIMER_ID = 1;
// Идентификатор кнопки "Скопировать"
constexpr UINT_PTR BUTTON_COPY_ID = 2;
// Идентификатор кнопки "GitHub"
constexpr UINT_PTR BUTTON_GITHUB_ID = 3;

// Глобальные переменные для хранения информации об использовании памяти
DWORDLONG g_TotalMemory = 0;
DWORDLONG g_UsedMemory = 0;
DWORDLONG g_FreeMemory = 0;
DWORD g_MemoryClockSpeed = 0;

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

    // Получение размеров экрана
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Создание окна
    HWND hwnd = CreateWindowEx(0, className, "Memory Monitor", WS_OVERLAPPEDWINDOW,
                               (screenWidth - 500) / 2, screenHeight - 150, 500, 150, NULL, NULL, hInstance, NULL);

    // Получение размеров окна
    RECT windowRect;
    GetClientRect(hwnd, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Создание кнопки "Скопировать"
    HWND buttonCopy = CreateWindow("BUTTON", "Copy", WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                   10, 10, 80, 30, hwnd, (HMENU)BUTTON_COPY_ID, hInstance, NULL);

    // Создание кнопки "GitHub"
    HWND buttonGitHub = CreateWindow("BUTTON", "GitHub", WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                     windowWidth - 90, 10, 80, 30, hwnd, (HMENU)BUTTON_GITHUB_ID, hInstance, NULL);

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
            } else if (LOWORD(wParam) == BUTTON_GITHUB_ID) {
                OnGitHub(hwnd);
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

void GetMemoryClockSpeed(DWORD& memoryClockSpeed) {
    HKEY hKey;
    DWORD dwType, dwData, dwSize = sizeof(DWORD);
    DWORD dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey);

    if (dwResult == ERROR_SUCCESS) {
        dwResult = RegQueryValueEx(hKey, "~MHz", NULL, &dwType, (LPBYTE)&dwData, &dwSize);
        if (dwResult == ERROR_SUCCESS && dwType == REG_DWORD)
            memoryClockSpeed = dwData;
        else
            memoryClockSpeed = 0;

        RegCloseKey(hKey);
    }
}

void OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    std::string text = "Total Memory: " + std::to_string(g_TotalMemory / (1024 * 1024)) + " MB\r\n";
    text += "Used Memory: " + std::to_string(g_UsedMemory / (1024 * 1024)) + " MB\r\n";
    text += "Free Memory: " + std::to_string(g_FreeMemory / (1024 * 1024)) + " MB\r\n";
    text += "Memory Clock Speed: " + std::to_string(g_MemoryClockSpeed) + " MHz\r\n";

    if (g_TotalMemory > 0) {
        int memoryLoad = static_cast<int>((g_UsedMemory * 100) / g_TotalMemory);
        text += "Memory Load: " + std::to_string(memoryLoad) + "%";
    } else {
        text += "Memory Load: N/A";
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    DrawText(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_TOP);

    EndPaint(hwnd, &ps);
}


void OnTimer(HWND hwnd) {
    GetMemoryInfo(g_TotalMemory, g_UsedMemory, g_FreeMemory);
    GetMemoryClockSpeed(g_MemoryClockSpeed);
    InvalidateRect(hwnd, NULL, TRUE);
}

void OnCopy(HWND hwnd) {
    std::string memoryInfo = "Total Memory: " + std::to_string(g_TotalMemory / (1024 * 1024)) + " MB\r\n";
    memoryInfo += "Used Memory: " + std::to_string(g_UsedMemory / (1024 * 1024)) + " MB\r\n";
    memoryInfo += "Free Memory: " + std::to_string(g_FreeMemory / (1024 * 1024)) + " MB\r\n";
    memoryInfo += "Memory Clock Speed: " + std::to_string(g_MemoryClockSpeed) + " MHz\r\n";
    memoryInfo += "Memory Load: " + std::to_string(g_UsedMemory * 100 / g_TotalMemory) + "%";

    g_CopiedMemoryInfo.push_back(memoryInfo);
}

void OnGitHub(HWND hwnd) {
    ShellExecute(NULL, "open", "https://github.com", NULL, NULL, SW_SHOWNORMAL);
}
