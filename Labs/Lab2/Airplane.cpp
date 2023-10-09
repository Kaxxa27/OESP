#include <windows.h>
#include <corecrt_wstdio.h>
#include <string>
#include "AirplaneDef.h"
#include <fstream> // Добавляем заголовочный файл для работы с файлами



HINSTANCE hInst;
HWND hMainWnd;

// Variables
HHOOK landingGear_hKeyboardHook = NULL;
bool isLandingGear = true;
bool isAirplaneMoving = true;
bool isCrashed = false;

// Start position
int airplaneX = 70;
int airplaneY = 510;

// Start speed
int airplaneSpeed = 0;

// Filename
CONST WCHAR* fileName = L"flight_recorder.txt";

// Memory mapped file 
HANDLE hFile = NULL;
HANDLE hMapFile = NULL;
LPVOID pMappedData = NULL;
CONST INT FILESIZE = 1048576; // 32000 click = 1 mb
size_t mappedDataSize = 0;

// Func
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void DrawAirplane(HDC hdc, int x, int y, int width, int height);
void UpdateAirplanePosition(int deltaX, int deltaY);
void SwitchLandingGear();
void StartAirplaneMovement();
void StopAirplaneMovement();

// File func
// Function for recording keystrokes to a file
void RecordKeyPress(const std::string& actionString);
void InitializeMappingFile();
void UninitializeMappingFile();

// Hook func
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
void SetKeyboardHook();
void UnhookKeyboardHook();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    InitializeMappingFile();

	hInst = hInstance;

    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        WndProc,
        0L,
        0L,
        GetModuleHandle(NULL),
        NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1),
        NULL, L"AirplaneApp", NULL
    };


    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, L"Call to RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    hMainWnd = CreateWindow(
        L"AirplaneApp",
        L"Airplane App",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    // Creating a label for a airplaneSpeed
    CreateWindow(L"STATIC", L"0 km/h", WS_CHILD | WS_VISIBLE, 10, 20, 80, 20, hMainWnd, (HMENU)ID_SPEED_LABEL, hInstance, NULL);

    if (!hMainWnd)
    {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    // Setting the hook
    SetKeyboardHook();
    // Setting the timer
    SetTimer(hMainWnd, ID_MAIN_TIMER, 20, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Delete the hook
    UnhookKeyboardHook();
    UninitializeMappingFile();

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message){
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);

        // Setting the background color to light blue (RGB(153, 204, 255))
        HBRUSH hLightBlueBrush = CreateSolidBrush(RGB(153, 204, 255));
        FillRect(hdc, &rc, hLightBlueBrush);
        DeleteObject(hLightBlueBrush);

        // Rendering the ground in light green colot (RGB(102, 255, 102))
        HBRUSH hLightGreenBrush = CreateSolidBrush(RGB(102, 255, 102));
        RECT groundRect = { rc.left, rc.bottom - 50, rc.right, rc.bottom };
        FillRect(hdc, &groundRect, hLightGreenBrush);
        DeleteObject(hLightGreenBrush);

        // Main Background the same as the system
        //FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
    
        DrawAirplane(hdc, airplaneX, airplaneY, AIRPLANE_WIDTH, AIRPLANE_HEIGHT);

        EndPaint(hWnd, &ps);
    }
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_SPACE:
            SwitchLandingGear();
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case VK_LEFT:
            if (airplaneSpeed > 0)
                airplaneSpeed--;
            // Stop airplane movement
            else StopAirplaneMovement();       
            break;

        case VK_RIGHT:
            if (airplaneSpeed < MAX_SPEED)
                airplaneSpeed++;
            // Start airplane movement
            StartAirplaneMovement();
            break;

        case VK_UP:
            UpdateAirplanePosition(0, -airplaneSpeed);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case VK_DOWN:
            UpdateAirplanePosition(0, airplaneSpeed);
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        // Updating text in ID_SPEED_LABEL
        {
            HWND hSpeedLabel = GetDlgItem(hWnd, ID_SPEED_LABEL);
            if (hSpeedLabel != NULL) {
                WCHAR speedText[16];
                swprintf_s(speedText, L"%d km/h", airplaneSpeed * 80);
                SetWindowTextW(hSpeedLabel, speedText);
            }
        }
        break;

    case WM_TIMER:
        if (wParam == ID_MAIN_TIMER && isAirplaneMoving) {
            UpdateAirplanePosition(airplaneSpeed, 0);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_DESTROY:
        KillTimer(hWnd, ID_MAIN_TIMER);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);  
    }
    return 0;
}

void DrawAirplane(HDC hdc, int x, int y, int width, int height) {
    
    // Coordinates for body
    int left = x;
    int right = x + width;
    int top = y;
    int bottom = y + height;

    // Drawing an airplane body
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 165, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Rectangle(hdc, left, top, right, bottom);

        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }
    
    // Drawing an airplane cabin
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 191, 255));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        // Drawing a semicircle (right side)
        int radius = height / 2;
        int centerX = x + width;
        int centerY = y + (height / 2);

        //Arc(hdc, right - radius, top, right + radius, bottom, right, bottom, right, top);
        Chord(hdc, right - radius, top, right + radius, bottom, right, bottom, right, top);
        //Pie(hdc, right - radius, top, right + radius, bottom, right, bottom, right, top);

        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    // Drawing an airplane tail 
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 165, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        int offsetT1 = 20;
        int offsetT2 = offsetT1 * 1.7;
        // Creating an array of tail points
        POINT points[] = {
            {left, bottom},
            {left - offsetT1, bottom - offsetT1},
            {left - offsetT2, top - offsetT2},
            {left, top},
            {left, bottom}
        };


        // Creating a contour for painting
        HRGN hRegion = CreatePolygonRgn(points, sizeof(points) / sizeof(points[0]), WINDING);

        // Fill in the region with color
        FillRgn(hdc, hRegion, hBrush);

        Polyline(hdc, points, sizeof(points) / sizeof(points[0]));


        // Delete the created region and brush
        DeleteObject(hRegion);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    // Drawing an airplane wing 
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 165, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        int offsetH = width / 4;
        int offsetV = height / 2;
        // Creating an array of wing points
        POINT points[] = {
            {right - offsetH, top + offsetV},
            {left + offsetH * 0.6, bottom + offsetV * 1.3},
            {left + offsetH * 1.4, top + offsetV},
        };

        // Creating a contour for painting
        HRGN hRegion = CreatePolygonRgn(points, sizeof(points) / sizeof(points[0]), WINDING);

        // Fill in the region with color
        FillRgn(hdc, hRegion, hBrush);

        Polyline(hdc, points, sizeof(points) / sizeof(points[0]));


        // Delete the created region and brush
        DeleteObject(hRegion);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    // Drawing an airplane windows 
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 191, 255));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        int winCount = 4;
        int offsetH = width / winCount;
        int offsetV = height / 2;

        int offset = offsetH / 4;
        for (size_t i = 0; i < winCount; i++)
        {
            Ellipse(hdc, left + i * offsetH + offset, top + offset, left + (i + 1) * offsetH - offset, top + offsetV);
        }
   
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }
    

    if (isLandingGear) {
        int offsetX = width / 10;
        int offsetY = width / 10;
        Ellipse(hdc, left + offsetX, bottom, left + offsetX * 2, bottom + offsetY);
        Ellipse(hdc, right - offsetX * 2, bottom, right - offsetX, bottom + offsetY);
    }
}

void SwitchLandingGear() {
    isLandingGear = !isLandingGear;
}

void UpdateAirplanePosition(int deltaX, int deltaY) {
    airplaneX += deltaX;
    airplaneY += deltaY;

    RECT clientRect;
    GetClientRect(hMainWnd, &clientRect);

    // Horizontal
    if (airplaneX + 100 <= 0) {
        airplaneX = clientRect.right;
    }
    else if (airplaneX >= clientRect.right) {
        airplaneX = -100;
    }

    // Vertical
    if (airplaneY <= 0) {
        airplaneY = clientRect.top;
    }
    else if (airplaneY >= clientRect.bottom - (50 + AIRPLANE_HEIGHT)) {
        airplaneY = clientRect.bottom - (50 + AIRPLANE_HEIGHT);
        
        // Collision with the ground
        if(!isLandingGear && !isCrashed)
        {
            StopAirplaneMovement();
            isCrashed = true;
            RecordKeyPress("Airplane crashed. WARNING!");
            MessageBox(hMainWnd, L"You lost control!\nBOOM!", L"Catastrophe", MB_ICONERROR);  
            SendMessage(hMainWnd, WM_DESTROY, 0, 0);
        }
    }
}

void StartAirplaneMovement() {
    isAirplaneMoving = true;
}

void StopAirplaneMovement() {
    isAirplaneMoving = false;
}

// Function for processing the left hook
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;

            switch (pKeyInfo->vkCode) {
            case VK_SPACE:
                // Depending on the state of the landing gear, we output a message to Debug
                // using ! because hook before WM 
                if (!isLandingGear) {
                    OutputDebugString(L"Landing Gear Deployed\n"); 
                    RecordKeyPress("Landing Gear Deployed.");
                }
                else {
                    OutputDebugString(L"Landing Gear Retracted\n");
                    RecordKeyPress("Landing Gear Retracted.");
                }
                break;
            case VK_LEFT:
                if (airplaneSpeed > 0)
                    RecordKeyPress("Speed reduced.");
                else RecordKeyPress("Speed equals 0.");
                break;
            case VK_RIGHT:
                if (airplaneSpeed < MAX_SPEED)
                    RecordKeyPress("Speed has increased.");
                else RecordKeyPress("Speed equals MAX_SPEED.");
                break;
            case VK_UP:
                RecordKeyPress("Airplane climb.");
                break;
            case VK_DOWN:
                RecordKeyPress("Airplane landing.");
                break;
            }
        }
    }

    return CallNextHookEx(landingGear_hKeyboardHook, nCode, wParam, lParam);
}

// Function for setting the left hook
void SetKeyboardHook() {
    landingGear_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (landingGear_hKeyboardHook == NULL) {
        MessageBox(NULL, L"Failed to set keyboard hook", L"Error", MB_ICONERROR);
    }
}

// Function for removing the left hook
void UnhookKeyboardHook() {
    if (landingGear_hKeyboardHook != NULL) {
        UnhookWindowsHookEx(landingGear_hKeyboardHook);
        landingGear_hKeyboardHook = NULL;
    }
}

void RecordKeyPress(const std::string& actionString) {
    if (pMappedData != NULL) {
        // Getting current time
        SYSTEMTIME currentTime;
        GetLocalTime(&currentTime);

        // Forming a strings with information about the time and the key
        std::string time = "Time: " + std::to_string(currentTime.wHour) + ":" +
            std::to_string(currentTime.wMinute) + ":" + std::to_string(currentTime.wSecond);
        
        std::string keyInfo =  time + " => Action: " + actionString + "\n";
   
        // Copying the information to a memory-mapped file
        size_t dataSize = keyInfo.size() * sizeof(CHAR);
        OutputDebugString(std::to_wstring(dataSize).c_str());
        OutputDebugString(L"\n");

        if (mappedDataSize + dataSize >= FILESIZE)
        {
            UninitializeMappingFile();
            InitializeMappingFile();
            mappedDataSize = 0;
        }
        memcpy((CHAR*)pMappedData + mappedDataSize, keyInfo.c_str(), dataSize);
        mappedDataSize += dataSize;
    }
}

void InitializeMappingFile() {

    // CreateFile
    hFile = CreateFile(
        fileName,                     // Filename
        GENERIC_READ | GENERIC_WRITE, // Access mode (read and write)
        0,                            // No sharing
        NULL,                         // Default protection
        CREATE_ALWAYS,                // Create a new file or overwrite an existing
        FILE_ATTRIBUTE_NORMAL,        // File attributes (normal)
        NULL                          // Template for creating files
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, L"CreateFile failed!", L"Error", MB_ICONERROR);
        return;
    }

    // CreateFileMapping
    hMapFile = CreateFileMapping(
        hFile,                       // File descriptor
        NULL,                        // Default protection
        PAGE_READWRITE,              // Display access mode (read and write)
        0,                           // Display file size (0 means the whole file)
        FILESIZE,                    // Display file size (the highest byte)
        NULL                         // Display file name
    );

    if (hMapFile == NULL) {
        MessageBox(NULL, L"CreateFileMapping failed!", L"Error", MB_ICONERROR);
        CloseHandle(hFile);
        return;
    }

    // Display the mapped file in memory
    pMappedData = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, FILESIZE);
    if (pMappedData == NULL) {
        MessageBox(NULL, L"MapViewOfFile failed!", L"Error", MB_ICONERROR);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return;
    }
}

void UninitializeMappingFile() {
 
    if (pMappedData != NULL) {
        UnmapViewOfFile(pMappedData);
        pMappedData = NULL;
    }

    if (hMapFile != NULL) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}
