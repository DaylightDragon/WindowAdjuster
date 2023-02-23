using namespace std;

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <cstdlib>
#include <thread>
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")

vector<HWND> oldRxWindows;

vector<HWND> rxWindowsForAdjusting;
vector<HWND> tempRxWindowsForAdjusting;

wstring rx_name = L"Roblox";

bool hideOnStart = true;
bool affectOldWindows = false;

void getWindowBorder(HWND hwnd, RECT &border) {
    RECT rect, frame;
    GetWindowRect(hwnd, &rect);
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(RECT));

    border.left = frame.left - rect.left;
    border.top = frame.top - rect.top;
    border.right = rect.right - frame.right;
    border.bottom = rect.bottom - frame.bottom;

    //cout << border.left << " " << border.top << " " << border.right << " " << border.bottom << " " << endl;
}

void getDesktopSizeNoTaskbar(RECT &rect) {
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
}

void setPos(HWND hwnd, int type, bool minimize) {
    int rxMinY = 638;
    RECT desktop, border;
    const HWND hDesktop = GetDesktopWindow();
    getDesktopSizeNoTaskbar(desktop);
    int desktopX = desktop.right;
    int desktopY = desktop.bottom;
    getWindowBorder(hwnd, border);

    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;

    // -7, 0, 967, 638     // 953, 0, 1927, 638     // -7, 402, 967, 1047     // 953, 402, 1927, 1047
    /*if (type == 1) {
        x1 = -7;
        y1 = 0;
        x2 = 974;
        y2 = 638;
    }
    else if (type == 2) {
        x1 = 953;
        y1 = 0;
        x2 = 974;
        y2 = 638;
    }
    else if (type == 3) {
        x1 = -7;
        y1 = 402;
        x2 = 974;
        y2 = 645;
    }
    else if (type == 4) {
        x1 = 953;
        y1 = 402;
        x2 = 974;
        y2 = 645;
    }
    */

    if (type == 1) {
        x1 = -border.left;
        y1 = border.top;
        x2 = desktopX / 2 + border.left + border.right;
        y2 = rxMinY+border.top;
    }
    else if (type == 2) {
        x1 = desktopX / 2 -border.left;
        y1 = border.top;
        x2 = desktopX / 2 + border.left + border.right;
        y2 = rxMinY + border.top;
    }
    else if (type == 3) {
        //desktopY/2-(rxMinY- desktopX/2)
        x1 = -border.left;
        y1 = desktopY - rxMinY;
        x2 = desktopX / 2 + border.left + border.right;
        y2 = rxMinY + border.bottom; // ?
    }
    else if (type == 4) {
        x1 = desktopX / 2 - border.left;
        y1 = desktopY - rxMinY;
        x2 = desktopX / 2 + border.left + border.right;
        y2 = rxMinY + border.bottom;
    }

    bool minimize2 = IsIconic(hwnd);

    RECT r;
    GetWindowRect(hwnd, &r);
    if(!(r.left==x1 && r.top==y1 && r.right==x2 && r.bottom==y2)) SetWindowPos(hwnd, NULL, x1, y1, x2, y2, NULL);

    Sleep(1);
    if(minimize2) ShowWindow(hwnd, SW_MINIMIZE); // || (minimize && hideOnStart)
}

void printWindowPos(HWND hwnd) {
    RECT p;
    GetWindowRect(hwnd, &p);
    cout << p.left << " " << p.top << endl;
    cout << p.right << " " << p.bottom << endl;
    //RECT p = WINDOWPLACEMENT;
    //SetWindowPlacement(hwnd,
}

void setPositions(bool minimize) {
    //cout << rxWindowsForAdjusting.size() << endl;
    int i = 1;
    Sleep(50);
    for (auto& w : rxWindowsForAdjusting) {
        //if(IsIconic(w)) {
            if(!IsWindowVisible(w)) ShowWindow(w, SW_RESTORE); // can it cause a bug?
        //}
        setPos(w, i, minimize);
        i++;
        if (i == 5) i = 1;
    }
    /*Sleep(100);
    if (i == 1) i = 4;
    else i--;
    setPos(rxWindowsForAdjusting[rxWindowsForAdjusting.size()-1], i, minimize);*/
}

bool containsWindow(HWND hwnd) {
    return (find(rxWindowsForAdjusting.begin(), rxWindowsForAdjusting.end(), hwnd) != rxWindowsForAdjusting.end());
}

bool checkNotStartupWindow(HWND hwnd) {
    //-7 0 967 638
    RECT p;
    GetWindowRect(hwnd, &p);
    //return (p.left == -7 && p.top == 0 && p.right == 967 && p.bottom == 638);
    int x = p.right - p.left;
    int y = p.bottom - p.top;
    //cout << x << " " << y << endl;
    
    return (x != 520 && y != 320); // 974 645 // 638 // 974 638 // 816 638
    // (x == 974 && (y == 645 || y == 638))
    // 974 638
}

void handleRxWindow(HWND hwnd) {
    if (!containsWindow(hwnd)) {
        if (!checkNotStartupWindow(hwnd)) return;
        RECT r;
        GetWindowRect(hwnd, &r);
        if (r.top == -32000) {
            ShowWindow(hwnd, SW_MINIMIZE);
            ShowWindow(hwnd, SW_RESTORE);
        }
        //cout << r.left << " " << r.right << " " << r.top << " " << r.bottom << endl;
        //printWindowPos(hwnd);
        rxWindowsForAdjusting.push_back(hwnd);
        setPositions(false); // true
        //if(hideOnStart) ShowWindow(hwnd, SW_MINIMIZE);
    }
}

void removeClosed(bool first) {
    bool recursion = false;
    for (int i = 0; i < rxWindowsForAdjusting.size(); i++) {
        if (!IsWindow(rxWindowsForAdjusting[i])) {
            rxWindowsForAdjusting.erase(rxWindowsForAdjusting.begin() + i);
            recursion = true;
            break;
        }
    }
    if (recursion) {
        removeClosed(false);
        if (first) setPositions(false);
    }
}

void removeOldExtraClosed() {
    bool recursion = false;
    for (int i = 0; i < oldRxWindows.size(); i++) {
        if (!IsWindow(oldRxWindows[i])) {
            oldRxWindows.erase(oldRxWindows.begin() + i);
            recursion = true;
            break;
        }
    }
    if (recursion) {
        removeClosed(false);
    }
}

bool isOldWindow(HWND hwnd) {
    return (find(oldRxWindows.begin(), oldRxWindows.end(), hwnd) != oldRxWindows.end());
}

static BOOL CALLBACK scanningRxCallback(HWND hwnd, LPARAM lparam) {
    int length = GetWindowTextLength(hwnd);
    wchar_t* buffer = new wchar_t[length + 1];
    GetWindowText(hwnd, buffer, length + 1);
    wstring ws(buffer);
    //string str(ws.begin(), ws.end());
    //if (!(r.right - r.left == 520 && r.top - r.bottom == 320)) {
        
    //}
    //if (520 320)
    if (ws == rx_name) {
        if (affectOldWindows || !isOldWindow(hwnd)) {
            handleRxWindow(hwnd);
            //cout << "Found!\n";
        }
    }
    return TRUE;
}

void scanRxs() {
    tempRxWindowsForAdjusting.clear();
    EnumWindows(scanningRxCallback, NULL);
}

static BOOL CALLBACK findOldCallback(HWND hwnd, LPARAM lparam) {
    int length = GetWindowTextLength(hwnd);
    wchar_t* buffer = new wchar_t[length + 1];
    GetWindowText(hwnd, buffer, length + 1);
    wstring ws(buffer);
    if (ws == rx_name) {
        oldRxWindows.push_back(hwnd);
    }
    return TRUE;
}

void findOldWindows() {
    EnumWindows(findOldCallback, NULL);
}

void printFailedHk(int failed) {
    if (failed > 0) {
        cout << "Failed to register " << failed << " hotkey";
        if (failed > 1) cout << "s";
        cout << endl;
    }

    //if (RegisterHotKey(NULL, 22, MOD_ALT | MOD_NOREPEAT, 0xDC)) { wprintf(L"Hotkey 'Alt + N': Start/Stop sending messages in RX chat\n"); }
    cout << endl;
}

void registerHotkeys() {
    int failed = 0;
    if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_NOREPEAT, 0x46)) { wprintf(L"Hotkey 'Alt + F': Pause/resume hiding on start (On by default)\n"); }
    else failed++;
    if (RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 0x46)) { wprintf(L"Hotkey 'Ctrl + Shift + F': Toggle affecting old windows (Off by default)\n"); } // ctrl alt v before to restore position
    else failed++;
    if (RegisterHotKey(NULL, 3, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 0x46)) { wprintf(L"Hotkey 'Ctrl + Alt + F': Adjust all again\n"); }
    else failed++;
    printFailedHk(failed);
}

void doTheActions() {
    removeClosed(true);
    removeOldExtraClosed();
    scanRxs();
}

void startHk() {
    registerHotkeys();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        if (msg.message == WM_HOTKEY) {
            if (msg.wParam == 1) {
                hideOnStart = !hideOnStart;
                if (hideOnStart) cout << "Hiding windows on start is now enabled" << endl;
                else cout << "Hiding windows on start is now disabled" << endl;
            }
            else if (msg.wParam == 2) {
                affectOldWindows = !affectOldWindows;
                if (affectOldWindows) cout << "Affecting old windows is now enabled" << endl;
                else cout << "Affecting old windows is now disabled" << endl;
            }
            else if (msg.wParam == 3) {
                rxWindowsForAdjusting.clear(); // bad code but such a fast way to make this
                doTheActions();
            }
        }
    }
}

int main() {
    findOldWindows();
    new thread(startHk);

    while (true) {
        //cout << "Scanning\n";
        doTheActions();
        Sleep(750);
    }
    return 0;
}