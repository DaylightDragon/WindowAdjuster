using namespace std;

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <cstdlib>

vector<HWND> oldRxWindows;

vector<HWND> rxWindowsForAdjusting;
vector<HWND> tempRxWindowsForAdjusting;

wstring rx_name = L"Roblox";

void setPos(HWND hwnd, int type, bool minimize) {
    cout << "pos " << type << endl;

    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    int x = desktop.right;
    int y = desktop.bottom;

    // -7, 0, 967, 638     // 953, 0, 1927, 638     // -7, 402, 967, 1047     // 953, 402, 1927, 1047
    if (type == 1) { SetWindowPos(hwnd, NULL, -7, 0, 974, 638, NULL); }
    else if (type == 2) { SetWindowPos(hwnd, NULL, 953, 0, 974, 638, NULL); } // 1390

    if (x == 1920 && y == 984) {
        if (type == 3) { SetWindowPos(hwnd, NULL, -7, 306, 974, 645, NULL); }
        else if (type == 4) { SetWindowPos(hwnd, NULL, 953, 306, 974, 645, NULL); }
    }
    else { // default 1920 * 1080
        if (type == 3) { SetWindowPos(hwnd, NULL, -7, 402, 974, 645, NULL); }
        else if (type == 4) { SetWindowPos(hwnd, NULL, 953, 402, 974, 645, NULL); }
    }

    Sleep(1);
    if(minimize) ShowWindow(hwnd, SW_MINIMIZE);
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
    cout << rxWindowsForAdjusting.size() << endl;
    int i = 1;
    Sleep(50);
    for (auto& w : rxWindowsForAdjusting) {
        //if(IsIconic(w)) {
            ShowWindow(w, SW_RESTORE);
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
    cout << x << " " << y << endl;
    
    return ((x==816 || x ==974) && y == 638); // 974 645 // 638 // 974 638 // 816 638
    // (x == 974 && (y == 645 || y == 638))
    // 974 638
}

void handleRxWindow(HWND hwnd) {
    if (!containsWindow(hwnd)) {
        if (!checkNotStartupWindow(hwnd)) return;
        //printWindowPos(hwnd);
        rxWindowsForAdjusting.push_back(hwnd);
        setPositions(false); // true
        ShowWindow(hwnd, SW_MINIMIZE);
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
    if (ws == rx_name) {
        if (!isOldWindow(hwnd)) {
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

int main() {
    findOldWindows();
    while (true) {
        //cout << "Scanning\n";
        removeClosed(true);
        removeOldExtraClosed();
        scanRxs();
        Sleep(750);
    }

    return 0;
}