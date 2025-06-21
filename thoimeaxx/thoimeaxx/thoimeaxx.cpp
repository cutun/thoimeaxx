#include <windows.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <oleacc.h>
#include <atlbase.h>
#include <atlcom.h>
#include <UIAutomation.h>
#include <vector>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "oleacc.lib")

#define HOTKEY_ID 1

class TransparentConsole {
private:
    HWND hConsole;
    bool isTransparent = false;
    bool isAlwaysOnTop = false;
    BYTE currentAlpha = 255;

public:
    TransparentConsole() {
        hConsole = GetConsoleWindow();
        SetupConsole();
    }

    void SetupConsole() {
        if (!hConsole) return;

        SetConsoleTitleA("Smart Text Capture v2.0 - Always On Top");
        SetAlwaysOnTop(true);
        SetTransparency(180);
        SetConsolePosition();
        EnableConsoleFeatures();
    }

    void SetAlwaysOnTop(bool enable) {
        if (!hConsole) return;

        if (enable) {
            SetWindowPos(hConsole, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            isAlwaysOnTop = true;
            std::cout << "[CONSOLE] Always on top: ENABLED" << std::endl;
        }
        else {
            SetWindowPos(hConsole, HWND_NOTOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            isAlwaysOnTop = false;
            std::cout << "[CONSOLE] Always on top: DISABLED" << std::endl;
        }
    }

    void SetTransparency(BYTE alpha) {
        if (!hConsole) return;

        LONG exStyle = GetWindowLong(hConsole, GWL_EXSTYLE);

        if (alpha < 255) {
            SetWindowLong(hConsole, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
            SetLayeredWindowAttributes(hConsole, 0, alpha, LWA_ALPHA);
            isTransparent = true;
            currentAlpha = alpha;

            int percentage = (alpha * 100) / 255;
            std::cout << "[CONSOLE] Transparency: " << percentage << "% opacity" << std::endl;
        }
        else {
            SetWindowLong(hConsole, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
            isTransparent = false;
            currentAlpha = 255;
            std::cout << "[CONSOLE] Transparency: DISABLED" << std::endl;
        }
    }

    void SetConsolePosition() {
        if (!hConsole) return;

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        int consoleWidth = screenWidth * 35 / 100;
        int consoleHeight = screenHeight * 50 / 100;

        int x = screenWidth - consoleWidth - 10;
        int y = 10;

        SetWindowPos(hConsole, NULL, x, y, consoleWidth, consoleHeight,
            SWP_NOZORDER | SWP_SHOWWINDOW);

        std::cout << "[CONSOLE] Positioned at top-right corner" << std::endl;
    }

    void EnableConsoleFeatures() {
        if (!hConsole) return;

        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdOut, &mode);
        SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        system("color 0A");
        std::cout << "[CONSOLE] Enhanced features enabled" << std::endl;
    }

    void ToggleTransparency() {
        if (isTransparent) {
            SetTransparency(255);
        }
        else {
            SetTransparency(180);
        }
    }

    void ToggleAlwaysOnTop() {
        SetAlwaysOnTop(!isAlwaysOnTop);
    }

    void AdjustTransparency(int delta) {
        int newAlpha = currentAlpha + delta;
        if (newAlpha < 50) newAlpha = 50;
        if (newAlpha > 255) newAlpha = 255;

        SetTransparency((BYTE)newAlpha);
    }

    void ShowControls() {
        std::cout << "\n[CONTROLS] Console Commands:" << std::endl;
        std::cout << "  T = Toggle transparency" << std::endl;
        std::cout << "  A = Toggle always on top" << std::endl;
        std::cout << "  + = Increase opacity" << std::endl;
        std::cout << "  - = Decrease opacity" << std::endl;
        std::cout << "  R = Reset position" << std::endl;
        std::cout << "  S = Save captured text to file" << std::endl;
        std::cout << "  C = Clear console" << std::endl;
        std::cout << "  H = Show/hide this help" << std::endl;
        std::cout << "  Q = Quit" << std::endl;
    }

    void ProcessConsoleInput(char key) {
        switch (key) {
        case 't':
        case 'T':
            ToggleTransparency();
            break;
        case 'a':
        case 'A':
            ToggleAlwaysOnTop();
            break;
        case '+':
        case '=':
            AdjustTransparency(20);
            break;
        case '-':
        case '_':
            AdjustTransparency(-20);
            break;
        case 'r':
        case 'R':
            SetConsolePosition();
            break;
        case 'c':
        case 'C':
            ClearConsole();
            break;
        case 's':
        case 'S':
            std::cout << "[INFO] Use 'S' after capturing text to save it" << std::endl;
            break;
        case 'h':
        case 'H':
            ShowControls();
            break;
        }
    }

    void FlashConsole() {
        if (hConsole) {
            SetLayeredWindowAttributes(hConsole, 0, 255, LWA_ALPHA);
            Sleep(50);
            SetLayeredWindowAttributes(hConsole, 0, currentAlpha, LWA_ALPHA);
        }
    }

    void ClearConsole() {
        system("cls");
        std::cout << "=====================================" << std::endl;
        std::cout << "   SMART TEXTBOX CAPTURE v2.0" << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << "[READY] Console cleared. Press H for help." << std::endl;
    }
};

class SmartTextCapture {
private:
    std::string lastClipboard;
    std::string lastCapturedText;
    int captureCount = 0;

public:
    std::string CaptureTextBox() {
        POINT originalCursor;
        GetCursorPos(&originalCursor);
        SaveCurrentClipboard();

        std::string result;

        result = GetTextFromUIAutomation();

        if (result.empty()) {
            result = GetTextWithSelectAll();
        }

        if (result.empty()) {
            result = GetTextFromInputField();
        }

        SetCursorPos(originalCursor.x, originalCursor.y);
        RestoreClipboard();

        if (!result.empty()) {
            lastCapturedText = result;
            captureCount++;
        }

        return CleanText(result);
    }

    void SaveLastCapturedText() {
        if (lastCapturedText.empty()) {
            std::cout << "[ERROR] No text to save!" << std::endl;
            return;
        }

        // Create filename with timestamp using safe functions
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        struct tm timeinfo;
        if (localtime_s(&timeinfo, &time_t) != 0) {
            std::cout << "[ERROR] Failed to get local time!" << std::endl;
            return;
        }

        std::stringstream ss;
        ss << "captured_text_" << std::put_time(&timeinfo, "%Y%m%d_%H%M%S") << ".txt";

        std::string filename = ss.str();
        std::ofstream file(filename);

        if (file.is_open()) {
            file << "=== CAPTURED TEXT ===" << std::endl;
            file << "Timestamp: " << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << std::endl;
            file << "Length: " << lastCapturedText.length() << " characters" << std::endl;
            file << "=====================" << std::endl << std::endl;
            file << lastCapturedText << std::endl;
            file.close();

            std::cout << "[SUCCESS] Text saved to: " << filename << std::endl;
        }
        else {
            std::cout << "[ERROR] Failed to save file!" << std::endl;
        }
    }

    int GetCaptureCount() const { return captureCount; }
    std::string GetLastCapturedText() const { return lastCapturedText; }

private:
    void SaveCurrentClipboard() {
        lastClipboard.clear();
        if (OpenClipboard(NULL)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* pszText = static_cast<char*>(GlobalLock(hData));
                if (pszText) {
                    lastClipboard = std::string(pszText);
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
    }

    void RestoreClipboard() {
        Sleep(100);
        if (!lastClipboard.empty() && OpenClipboard(NULL)) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, lastClipboard.length() + 1);
            if (hMem) {
                char* pMem = static_cast<char*>(GlobalLock(hMem));
                if (pMem) {
                    strcpy_s(pMem, lastClipboard.length() + 1, lastClipboard.c_str());
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_TEXT, hMem);
                }
            }
            CloseClipboard();
        }
    }

    std::string GetTextFromUIAutomation() {
        HRESULT hr = CoInitialize(NULL);
        if (FAILED(hr)) return "";

        CComPtr<IUIAutomation> pAutomation;
        hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER,
            __uuidof(IUIAutomation), (void**)&pAutomation);

        if (FAILED(hr) || !pAutomation) {
            CoUninitialize();
            return "";
        }

        POINT pt;
        GetCursorPos(&pt);

        CComPtr<IUIAutomationElement> pElement;
        hr = pAutomation->ElementFromPoint(pt, &pElement);

        std::string result;
        if (SUCCEEDED(hr) && pElement) {
            CComPtr<IUIAutomationValuePattern> pValuePattern;
            hr = pElement->GetCurrentPatternAs(UIA_ValuePatternId,
                __uuidof(IUIAutomationValuePattern),
                (void**)&pValuePattern);

            if (SUCCEEDED(hr) && pValuePattern) {
                BSTR bstrValue = NULL;
                if (SUCCEEDED(pValuePattern->get_CurrentValue(&bstrValue)) && bstrValue) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, bstrValue, -1, NULL, 0, NULL, NULL);
                    if (len > 0) {
                        std::vector<char> buffer(len);
                        WideCharToMultiByte(CP_UTF8, 0, bstrValue, -1, buffer.data(), len, NULL, NULL);
                        result = std::string(buffer.data());
                    }
                    SysFreeString(bstrValue);
                }
            }

            if (result.empty()) {
                CComPtr<IUIAutomationTextPattern> pTextPattern;
                hr = pElement->GetCurrentPatternAs(UIA_TextPatternId,
                    __uuidof(IUIAutomationTextPattern),
                    (void**)&pTextPattern);

                if (SUCCEEDED(hr) && pTextPattern) {
                    CComPtr<IUIAutomationTextRange> pDocRange;
                    if (SUCCEEDED(pTextPattern->get_DocumentRange(&pDocRange)) && pDocRange) {
                        BSTR bstrText = NULL;
                        if (SUCCEEDED(pDocRange->GetText(-1, &bstrText)) && bstrText) {
                            int len = WideCharToMultiByte(CP_UTF8, 0, bstrText, -1, NULL, 0, NULL, NULL);
                            if (len > 0) {
                                std::vector<char> buffer(len);
                                WideCharToMultiByte(CP_UTF8, 0, bstrText, -1, buffer.data(), len, NULL, NULL);
                                result = std::string(buffer.data());
                            }
                            SysFreeString(bstrText);
                        }
                    }
                }
            }

            if (result.empty()) {
                BSTR bstrName = NULL;
                if (SUCCEEDED(pElement->get_CurrentName(&bstrName)) && bstrName) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, bstrName, -1, NULL, 0, NULL, NULL);
                    if (len > 0) {
                        std::vector<char> buffer(len);
                        WideCharToMultiByte(CP_UTF8, 0, bstrName, -1, buffer.data(), len, NULL, NULL);
                        result = std::string(buffer.data());
                    }
                    SysFreeString(bstrName);
                }
            }
        }

        CoUninitialize();
        return result;
    }

    std::string GetTextWithSelectAll() {
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            CloseClipboard();
        }

        POINT pt;
        GetCursorPos(&pt);
        SetCursorPos(pt.x, pt.y);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        Sleep(100);

        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event('A', 0, 0, 0);
        keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        Sleep(100);

        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event('C', 0, 0, 0);
        keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        Sleep(200);

        std::string result;
        if (OpenClipboard(NULL)) {
            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData) {
                char* pszText = static_cast<char*>(GlobalLock(hData));
                if (pszText) {
                    result = std::string(pszText);
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }

        keybd_event(VK_ESCAPE, 0, 0, 0);
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
        Sleep(50);

        return result;
    }

    std::string GetTextFromInputField() {
        POINT pt;
        GetCursorPos(&pt);

        HWND hWnd = WindowFromPoint(pt);
        if (!hWnd) return "";

        char buffer[8192];

        int len = static_cast<int>(GetWindowTextA(hWnd, buffer, sizeof(buffer) - 1));
        if (len > 0) {
            buffer[len] = '\0';
            return std::string(buffer);
        }

        len = static_cast<int>(SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0));
        if (len > 0 && len < static_cast<int>(sizeof(buffer)) - 1) {
            if (SendMessageA(hWnd, WM_GETTEXT, sizeof(buffer), (LPARAM)buffer) > 0) {
                return std::string(buffer);
            }
        }

        return "";
    }

    std::string CleanText(const std::string& text) {
        if (text.empty() || text == lastClipboard) return "";

        std::string result = text;

        if (result.length() >= 3 &&
            (unsigned char)result[0] == 0xEF &&
            (unsigned char)result[1] == 0xBB &&
            (unsigned char)result[2] == 0xBF) {
            result = result.substr(3);
        }

        size_t start = result.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";

        size_t end = result.find_last_not_of(" \t\n\r");
        result = result.substr(start, end - start + 1);

        return result;
    }
};

std::string GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    struct tm timeinfo;
    if (localtime_s(&timeinfo, &time_t) != 0) {
        return "??:??:??";
    }

    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%H:%M:%S");
    return ss.str();
}

int main() {
    TransparentConsole console;
    SmartTextCapture capture;

    std::cout << "=====================================" << std::endl;
    std::cout << "   SMART TEXTBOX CAPTURE v0.0001" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "[HOTKEY] Ctrl+` = Capture textbox" << std::endl;
    std::cout << "[FEATURE] Semi-transparent overlay" << std::endl;
    std::cout << "[FEATURE] Always on top window" << std::endl;
    std::cout << "[FEATURE] Auto-save to file" << std::endl;
    std::cout << "=====================================" << std::endl;

    console.ShowControls();

    bool hotkeyRegistered = false;
    if (RegisterHotKey(NULL, HOTKEY_ID, MOD_CONTROL, VK_OEM_3)) {
        std::cout << "[SUCCESS] Using Ctrl+` as hotkey" << std::endl;
        hotkeyRegistered = true;
    }
    else if (RegisterHotKey(NULL, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'C')) {
        std::cout << "[SUCCESS] Using Ctrl+Shift+C as hotkey" << std::endl;
        hotkeyRegistered = true;
    }
    else {
        std::cout << "[ERROR] Failed to register any hotkey!" << std::endl;
        return 1;
    }

    std::cout << "[READY] Position mouse over textbox and press hotkey..." << std::endl;

    MSG msg;
    while (true) {
        if (_kbhit()) {
            char input = _getch();
            if (input == 'q' || input == 'Q') {
                std::cout << "[EXIT] Total captures: " << capture.GetCaptureCount() << std::endl;
                std::cout << "[EXIT] Goodbye!" << std::endl;
                break;
            }
            else if (input == 's' || input == 'S') {
                capture.SaveLastCapturedText();
            }
            else {
                console.ProcessConsoleInput(input);
            }
        }

        if (PeekMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY, PM_REMOVE)) {
            if (msg.wParam == HOTKEY_ID) {
                console.FlashConsole();
                std::cout << "\n[" << GetCurrentTimeString() << "] CAPTURE Processing..." << std::endl;

                std::string text = capture.CaptureTextBox();
                if (!text.empty()) {
                    std::cout << "[SUCCESS] Captured " << text.length() << " characters:" << std::endl;
                    std::cout << "+" << std::string(50, '-') << "+" << std::endl;

                    if (text.length() > 300) {
                        std::cout << "| " << text.substr(0, 300) << "..." << std::endl;
                        std::cout << "| [TRUNCATED] Total: " << text.length() << " chars" << std::endl;
                    }
                    else {
                        // Split long lines
                        std::string displayText = text;
                        size_t pos = 0;
                        while (pos < displayText.length()) {
                            size_t endPos = (std::min)(pos + 48, displayText.length());
                            std::cout << "| " << displayText.substr(pos, endPos - pos) << std::endl;
                            pos = endPos;
                        }
                    }
                    std::cout << "+" << std::string(50, '-') << "+" << std::endl;
                    std::cout << "[INFO] Press 'S' to save to file" << std::endl;
                }
                else {
                    std::cout << "[INFO] No text found at cursor position" << std::endl;
                }
                std::cout << "[READY] Ready for next capture..." << std::endl;
            }
        }

        Sleep(10);
    }

    UnregisterHotKey(NULL, HOTKEY_ID);
    return 0;
}
