#include <windows.h>

#define ID_MAIN_TIMER 1

HINSTANCE hInst;
HWND hMainWnd;

bool isLandingGear = false;
bool isAirplaneMoving = true;
int airplaneX = 0;
int airplaneY = 200;
int airplaneSpeed = 5;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawAirplane(HDC hdc, int x, int y, int width, int height);
void SwitchLandingGear();
void UpdateAirplanePosition(int deltaX, int deltaY);
void StartAirplaneMovement();
void StopAirplaneMovement();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

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
        L"Airplane Example",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    if (!hMainWnd)
    {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    SetTimer(hMainWnd, ID_MAIN_TIMER, 20, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

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

        // Size
        int airplaneWidth = 100;
        int airplaneHeight = 40;
        DrawAirplane(hdc, airplaneX, airplaneY, airplaneWidth, airplaneHeight);

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
            // Stop airplane movement
            StopAirplaneMovement();
            break;

        case VK_RIGHT:
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
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 165, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    Rectangle(hdc, x, y, x + width, y + height);

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    if (isLandingGear) {
        Ellipse(hdc, x + 10, y + height, x + 20, y + height + 10);
        Ellipse(hdc, x + width - 20, y + height, x + width - 10, y + height + 10);
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

    if (airplaneX + 100 <= 0) {
        airplaneX = clientRect.right;
    }
    else if (airplaneX >= clientRect.right) {
        airplaneX = -100;
    }

    if (airplaneY + 40 <= 0) {
        airplaneY = clientRect.bottom;
    }
    else if (airplaneY >= clientRect.bottom) {
        airplaneY = -40;
    }
}

void StartAirplaneMovement() {
    isAirplaneMoving = true;
}

void StopAirplaneMovement() {
    isAirplaneMoving = false;
}
