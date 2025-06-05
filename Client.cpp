// Client.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Client.h"
#include "resource.h"
#include <thread>
#include <memory>
#include <string>
#include <vector>

#define MAX_LOADSTRING 100
#define WM_COM_RECEIVED (WM_APP + 1)


struct SerialSettings {
    int portNumber;  // COM port number
    DWORD baudRate;  // Baud rate
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd; // Global variable for the main window
HWND hEditWnd;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// user forward declarations
void StartPolling(std::shared_ptr<SerialSettings> settings);
SerialSettings GetDefaultSerialSettings();
HANDLE OpenSerialPort(int portNumber, DWORD baudRate);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        RECT clientRect;
        GetClientRect(hWnd, &clientRect); // Get main window client area dimensions

        hEditWnd = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, clientRect.right - 20, clientRect.bottom - 20, // Dynamic size
            hWnd, (HMENU)IDC_CLIENT, hInst, NULL);

        break;

    case WM_COM_RECEIVED:
    {
        std::wstring* msg = reinterpret_cast<std::wstring*>(lParam);

        // Move cursor to the end
        SendMessage(hEditWnd, EM_SETSEL, -1, -1);

        // Append the new text
        SendMessageW(hEditWnd, EM_REPLACESEL, FALSE, (LPARAM)msg->c_str());

        delete msg;
    }
    break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            
            case ID_PORT_STARTPOLLING:
            {
                auto settings = std::make_shared<SerialSettings>(GetDefaultSerialSettings());
                StartPolling(settings);  
            }
            break;

            case ID_PORT_STOPPOLLING:
                //StopPolling();
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
void StartPolling(std::shared_ptr<SerialSettings> settings) {
    std::thread([settings]() {
        HANDLE hSerial = OpenSerialPort(settings->portNumber, settings->baudRate);
        if (hSerial == INVALID_HANDLE_VALUE) {
            PostMessage(hWnd, WM_COM_RECEIVED, 0, (LPARAM)new std::wstring(L"[ERROR] Failed to open COM port.\r\n"));
            return;
        }

        const DWORD bufferSize = 64;
        char buffer[bufferSize];
        DWORD bytesRead;

        while (true) {
            if (ReadFile(hSerial, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0) {
                // Convert from bytes (ASCII) to wstring
                int wlen = MultiByteToWideChar(CP_ACP, 0, buffer, bytesRead, NULL, 0);
                if (wlen > 0) {
                    std::wstring wmsg(wlen, 0);
                    MultiByteToWideChar(CP_ACP, 0, buffer, bytesRead, &wmsg[0], wlen);

                    // Allocate and send the wstring pointer
                    std::wstring* msg = new std::wstring(std::move(wmsg));
                    PostMessage(hWnd, WM_COM_RECEIVED, 0, (LPARAM)msg);
                }
            }
            Sleep(100);
        }


        CloseHandle(hSerial);
        }).detach();  // Don't forget to detach the thread!
}

HANDLE OpenSerialPort(int portNumber, DWORD baudRate) {
    std::wstring portName = std::wstring(L"\\\\.\\COM") + std::to_wstring(portNumber);

    HANDLE hSerial = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, (L"Failed to open " + portName).c_str(), L"Error", MB_OK | MB_ICONERROR);
        return INVALID_HANDLE_VALUE;
    }
    
    // Configure baud rate
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    // **SET TIMEOUTS HERE**
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;  // Small delay between packets
    timeouts.ReadTotalTimeoutConstant = 200; // Max wait time for a read operation
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts); // Apply timeouts
    
    return hSerial;
}
SerialSettings GetDefaultSerialSettings()
{
    SerialSettings settings;
    settings.portNumber = 4;
    settings.baudRate = 115200;
    return settings;
}



