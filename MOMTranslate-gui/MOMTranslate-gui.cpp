#include "framework.h"
#include "MOMTranslate-gui.h"

#include "LibMOMTranslate.h"

#define MAX_LOADSTRING 100

#define TRANSLATION_BUFFER_SIZE 1024
#define CLIPBOARD_BUFFER_SIZE 1024
#define DISPLAY_TEXT_BUFFER_SIZE 1024

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HFONT hFont;

char* clipboardBuffer;
char* translationBuffer;
wchar_t* displayTextBuffer;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void                ToggleWindow(HWND hWnd);
bool                Translate(HWND hWnd);
void                CharToWCharT(char*, wchar_t*, size_t);

void                AddTrayIcon(HWND hWnd);
void                RemoveTrayIcon(HWND hWnd);

void                LoadFont(HWND hWnd);
bool                RegisterToggleHotKey(HWND hWnd);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    clipboardBuffer = (char*)malloc(sizeof(char) * CLIPBOARD_BUFFER_SIZE);
    translationBuffer = (char*)malloc(sizeof(char) * TRANSLATION_BUFFER_SIZE);
    displayTextBuffer = (wchar_t*)malloc(sizeof(wchar_t) * DISPLAY_TEXT_BUFFER_SIZE);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOMTRANSLATEGUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOMTRANSLATEGUI));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;

    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOMTRANSLATEGUI));
    wcex.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    RECT desktop;
    hInst = hInstance;
    GetWindowRect(GetDesktopWindow(), &desktop);

    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_LAYERED,
                                szWindowClass,
                                szTitle,
                                WS_POPUPWINDOW,
                                0, 0,
                                0, 0,
                                nullptr, nullptr,
                                hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    if (!RegisterToggleHotKey(hWnd)) {
        return FALSE;
    }

    AddTrayIcon(hWnd);
    LoadFont(hWnd);

    SetLayeredWindowAttributes(hWnd, 0x00FFFFFF, 200, LWA_ALPHA);
    SetWindowPos(hWnd, HWND_TOPMOST,
                 (desktop.right - WINDOW_WIDTH) / 2,
                 desktop.bottom - WINDOW_HEIGHT,
                 WINDOW_WIDTH, WINDOW_HEIGHT, SWP_FRAMECHANGED);

    SendMessage(hWnd, WM_SETFONT, (LPARAM) hFont, TRUE);

    return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            SelectObject(hdc, hFont);
            SetTextColor(hdc, 0x111111);

            CharToWCharT(translationBuffer, displayTextBuffer, DISPLAY_TEXT_BUFFER_SIZE);

            DrawText(hdc, displayTextBuffer, -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_CALCRECT);
            rect.left = 0;
            rect.top = (WINDOW_HEIGHT - rect.bottom) / 2;
            rect.bottom = (WINDOW_HEIGHT + rect.bottom) / 2;
            rect.right = WINDOW_WIDTH;
            DrawText(hdc, displayTextBuffer, -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_HOTKEY:
        if (IsWindowVisible(hWnd)) {
            ToggleWindow(hWnd);
        }
        else {
            if (Translate(hWnd)) {
                ToggleWindow(hWnd);
            }
        }
        break;
    case WM_DESTROY:
        RemoveTrayIcon(hWnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


void AddTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW notifyIconData;

    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hWnd = hWnd;
    notifyIconData.uID = IDI_SMALL;
    notifyIconData.uFlags = NIF_INFO | NIF_ICON;
    notifyIconData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));

    wcscpy_s(notifyIconData.szInfo, 256, L"Press CTRL-B to translate text from your clipboard.");
    wcscpy_s(notifyIconData.szInfoTitle, 64, L"MOM, Translate!");

    Shell_NotifyIconW(NIM_ADD, &notifyIconData);
}

void RemoveTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW notifyIconData;

    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hWnd = hWnd;
    notifyIconData.uID = IDI_SMALL;
    notifyIconData.uFlags = NIF_ICON;
    notifyIconData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));

    Shell_NotifyIconW(NIM_DELETE, &notifyIconData);
}

bool ReadClipboard(HWND hWnd) {
    if (OpenClipboard(hWnd)) {
        HGLOBAL hglb = GetClipboardData(CF_TEXT);
        strcpy_s(clipboardBuffer, CLIPBOARD_BUFFER_SIZE, (char*)hglb);
        CloseClipboard();
        return true;
    }

    return false;
}

bool Translate(HWND hWnd) {
    if (!ReadClipboard(hWnd))
        return false;

    if (!translate(clipboardBuffer, translationBuffer, TRANSLATION_BUFFER_SIZE))
        return false;
}

void ToggleWindow(HWND hWnd) {
      if (IsWindowVisible(hWnd)) {
          AnimateWindow(hWnd, 200, AW_HIDE | AW_SLIDE | AW_VER_POSITIVE);
      }
      else {
          ShowWindow(hWnd, SW_SHOWNOACTIVATE);
      }
}

void LoadFont(HWND hWnd) {
    hFont = CreateFont(42, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
                      OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
}

bool RegisterToggleHotKey(HWND hWnd) {
    return RegisterHotKey(hWnd,
                          IDC_HOTKEY_TOGGLE_SHOW,
                          MOD_CONTROL | MOD_NOREPEAT,
                          0x42);
}

void CharToWCharT(char *from, wchar_t *to, size_t to_size) {
    std::wstring wstr;

    if (sizeof(wchar_t) == 2) {
        std::u16string data = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(from);

        for (wchar_t x : data) {
            wstr += x;
        }
        
    }
    else if (sizeof(wchar_t) == 4) {
        std::u32string data = std::wstring_convert<std::codecvt_utf8_utf16<char32_t>, char32_t>{}.from_bytes(from);

        for (wchar_t x : data) {
            wstr += x;
        }
        
    }

    wcscpy_s(to, to_size, wstr.c_str());
}

