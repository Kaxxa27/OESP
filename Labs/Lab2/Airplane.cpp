#include <windows.h>
#include <corecrt_wstdio.h>

#define ID_MAIN_TIMER 1
#define ID_SPEED_LABEL 2

HINSTANCE hInst;
HWND hMainWnd;

const int MAX_SPEED = 15;
bool isLandingGear = true;
bool isAirplaneMoving = true;
bool isCrashed = false;
int airplaneX = 70;
int airplaneY = 510;
int airplaneSpeed = 0;
int airplaneWidth = 150;
int airplaneHeight = 50;

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

    // Creating a label for a airplaneSpeed
    CreateWindow(L"STATIC", L"0 km/h", WS_CHILD | WS_VISIBLE, 10, 20, 80, 20, hMainWnd, (HMENU)ID_SPEED_LABEL, hInstance, NULL);

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

        // ���������� ������ � ID_SPEED_LABEL
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
    
    // coordinates body
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

        // ��������� �������������� (������ �����)
        int radius = height / 2; // ������ ��������������
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


        // ������� ������ ��� ��������
        HRGN hRegion = CreatePolygonRgn(points, sizeof(points) / sizeof(points[0]), WINDING);

        // ��������� ������ ������
        FillRgn(hdc, hRegion, hBrush);

        Polyline(hdc, points, sizeof(points) / sizeof(points[0]));


        // ������� ��������� ������ � �����
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

        // ������� ������ ��� ��������
        HRGN hRegion = CreatePolygonRgn(points, sizeof(points) / sizeof(points[0]), WINDING);

        // ��������� ������ ������
        FillRgn(hdc, hRegion, hBrush);

        Polyline(hdc, points, sizeof(points) / sizeof(points[0]));


        // ������� ��������� ������ � �����
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
    else if (airplaneY >= clientRect.bottom - (50 + airplaneHeight)) {
        airplaneY = clientRect.bottom - (50 + airplaneHeight);
        if(!isLandingGear && !isCrashed)
        {
            StopAirplaneMovement();
            isCrashed = true;
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
