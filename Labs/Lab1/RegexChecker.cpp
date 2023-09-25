#include <windows.h>
#include <regex>

#define ID_CHECK_BUTTON 1

#define ID_REGEX_LABEL 2
#define ID_REGEX_EDIT 3

#define ID_TEXT_LABEL 4
#define ID_TEXT_EDIT 5

// Глобальные переменные
HWND hMainWindow;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// Прототип функции
void CreateUIElements(HINSTANCE hInstance);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Инициализация главного окна
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"SimpleApp";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, L"Call to RegisterClassEx failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    // Создание главного окна
    hMainWindow = CreateWindow(L"SimpleApp", L"Regex Checker", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, hInstance, NULL);

    if (!hMainWindow)
    {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Error", MB_ICONERROR);
        return 1;
    }

    // Создание элементов интерфейса (поля и кнопки)
    CreateUIElements(hInstance);

    // Отображение главного окна
    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);

    // Основной цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void CreateUIElements(HINSTANCE hInstance)
{

    // Создание label для регулярного выражения
    CreateWindow(L"STATIC", L"Regex: ", WS_CHILD | WS_VISIBLE, 10, 20, 50, 20, hMainWindow, (HMENU)ID_REGEX_LABEL, hInstance, NULL);

    // Создание поля для регулярного выражения
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 65, 20, 200, 20, hMainWindow, (HMENU)ID_REGEX_EDIT, hInstance, NULL);

    // Создание label для текста
    CreateWindow(L"STATIC", L"Text: ", WS_CHILD | WS_VISIBLE, 10, 50, 50, 20, hMainWindow, (HMENU)ID_REGEX_LABEL, hInstance, NULL);

    // Создание поля для текста
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 65, 50, 200, 20, hMainWindow, (HMENU)ID_TEXT_EDIT, hInstance, NULL);

    // Создание кнопки
    CreateWindow(L"BUTTON", L"Check", WS_CHILD | WS_VISIBLE, 150, 80, 100, 30, hMainWindow, (HMENU)ID_CHECK_BUTTON, hInstance, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Gets HWND for all controls

    HWND hRegexField = GetDlgItem(hWnd, ID_REGEX_EDIT);
    HWND hTextField = GetDlgItem(hWnd, ID_TEXT_EDIT);
    HWND hButton = GetDlgItem(hWnd, ID_CHECK_BUTTON);

    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            /*     case ID_REGEX_EDIT:
                if (HIWORD(wParam) == EN_SETFOCUS)
                {
                    SetWindowText(GetDlgItem(hWnd, ID_REGEX_EDIT), L"");
                }
                break;
            */
        case ID_CHECK_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                // Получение регулярки
                WCHAR regexBuffer[256];
                GetWindowText(hRegexField, regexBuffer, sizeof(regexBuffer) / sizeof(regexBuffer[0]));

                // Получение текста
                WCHAR textBuffer[256];
                GetWindowText(hTextField, textBuffer, sizeof(textBuffer) / sizeof(textBuffer[0]));

                // Преобразование регулярного выражения в объект std::wregex
                std::wregex regex;
                try
                {
                    regex = std::wregex(regexBuffer);
                }
                catch (const std::regex_error&)
                {
                    MessageBox(hWnd, L"Invalid regular expression!", L"Error", MB_ICONERROR);
                    break;
                }

                // Проверка совпадения текста с регулярным выражением
                // Вывод результата в MessageBox
                if (std::regex_match(textBuffer, regex))
                {
                    MessageBox(hWnd, L"Success", L"Result", MB_ICONINFORMATION);
                }
                else
                {
                    MessageBox(hWnd, L"Failure", L"Result", MB_ICONERROR);
                }
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
