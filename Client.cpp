// Client.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Client.h"
#include "resource.h"
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <atomic>

#define MAX_LOADSTRING 100
#define WM_COM_RECEIVED (WM_APP + 1)


struct SerialSettings{
    int portNumber = 4;      // COM3 by default
    int baudRate = 115200;   // Default baud rate
    int dataBits = 8;        // Typical default: 8 data bits
    int stopBits = 1;        // 1 stop bit
    char parity = 'N';       // No parity ('N' = None, 'E' = Even, 'O' = Odd)
    bool flowControl = false; // No flow control by default
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd; // Global variable for the main window
HWND hEditWnd;

std::shared_ptr<SerialSettings> g_serialSettings = std::make_shared<SerialSettings>();
std::atomic<bool> keepPolling{ false };  // Shared stop flag

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SerialSettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// user forward declarations
void StartPolling(std::shared_ptr<SerialSettings> settings);
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
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect); // Get main window client area dimensions

        hEditWnd = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, clientRect.right - 20, clientRect.bottom - 20, // Dynamic size
            hWnd, (HMENU)IDC_CLIENT, hInst, NULL);

    }
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

    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hwndCtrl = (HWND)lParam;
        if (hwndCtrl == hEditWnd)
        {
            SetTextColor(hdc, RGB(255, 255, 255));  // white text
            SetBkColor(hdc, RGB(0, 0, 0));          // black background
            static HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            return (INT_PTR)hBrush;
        }
        break;
    }


    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case ID_PORT_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SERIAL_SETTINGS), hWnd, SerialSettingsDlgProc);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            case ID_FILE_CLEARSCREEN:
                SetWindowTextW(hEditWnd, L"");
                break;

            case ID_PORT_STARTPOLLING:
            {
                keepPolling = true;
                StartPolling(g_serialSettings);
            }
            break;

            case ID_PORT_STOPPOLLING:
                keepPolling = false;
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
INT_PTR CALLBACK SerialSettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Populate Port Number listbox (COM1 to COM10)
        for (int port = 1; port <= 10; ++port)
        {
            char buf[10];
            sprintf_s(buf, sizeof(buf), "COM%d", port);
            SendDlgItemMessageA(hDlg, IDC_LIST_PORTNUM, LB_ADDSTRING, 0, (LPARAM)buf);
        }

        // Populate Baud Rate combo box
        const char* baudRates[] = {
            "128000", "115200", "57600", "56000", "38400", "19200",
            "14400", "9600", "7200", "4800", "2400", "1800",
            "1200", "600", "300", "150", "134", "110", "75"
        };
        
        for (int i = 0; i < (int)(sizeof(baudRates) / sizeof(baudRates[0])); ++i)
        {
            SendDlgItemMessageA(hDlg, IDC_COMBO_BAUDRATE, CB_ADDSTRING, 0, (LPARAM)baudRates[i]);
        }


        // Populate Data Bits listbox (4 to 8)
        for (int dataBits = 4; dataBits <= 8; ++dataBits)
        {
            char buf[3];
            sprintf_s(buf, sizeof(buf), "%d", dataBits);
            SendDlgItemMessageA(hDlg, IDC_LIST_DATABITS, LB_ADDSTRING, 0, (LPARAM)buf);
        }

        // Populate Parity listbox
        const char* parityOptions[] = { "None", "Even", "Odd", "Mark", "Space" };
        for (int i = 0; i < (int)(sizeof(parityOptions) / sizeof(parityOptions[0])); ++i)
        {
            SendDlgItemMessageA(hDlg, IDC_LIST_PARITY, LB_ADDSTRING, 0, (LPARAM)parityOptions[i]);
        }

        // Populate Flow Control listbox
        const char* flowOptions[] = { "None", "Xon/Xoff", "Hardware" };
        for (int i = 0; i < (int)(sizeof(flowOptions) / sizeof(flowOptions[0])); ++i)
        {
            SendDlgItemMessageA(hDlg, IDC_LIST_FLOWCONTROL, LB_ADDSTRING, 0, (LPARAM)flowOptions[i]);
        }

        // Populate Stop Bits listbox (assuming you kept it a listbox, IDC_LIST_STOPBITS)
        const char* stopBitsOptions[] = { "1", "1.5", "2" };
        for (int i = 0; i < (int)(sizeof(stopBitsOptions) / sizeof(stopBitsOptions[0])); ++i)
        {
            SendDlgItemMessageA(hDlg, IDC_LIST_STOPBITS, LB_ADDSTRING, 0, (LPARAM)stopBitsOptions[i]);
        }

        // Set default selections based on your default SerialSettings values:
        // port 3 (COM3), baud 115200, data bits 8, parity None, flow None, stop bits 1
        SendDlgItemMessage(hDlg, IDC_LIST_PORTNUM, LB_SETCURSEL, 2, 0); // zero-based index: COM3 is 2
        SendDlgItemMessage(hDlg, IDC_COMBO_BAUDRATE, CB_SETCURSEL, 17, 0); // 115200 is index 17 in baudRates[]
        SendDlgItemMessage(hDlg, IDC_LIST_DATABITS, LB_SETCURSEL, 4, 0); // 8 data bits (4 is index for 8)
        SendDlgItemMessage(hDlg, IDC_LIST_PARITY, LB_SETCURSEL, 0, 0); // None parity
        SendDlgItemMessage(hDlg, IDC_LIST_FLOWCONTROL, LB_SETCURSEL, 0, 0); // None flow control
        SendDlgItemMessage(hDlg, IDC_LIST_STOPBITS, LB_SETCURSEL, 0, 0); // 1 stop bit

        SendDlgItemMessageA(hDlg, IDC_COMBO_BAUDRATE, CB_SETCURSEL, 7, 0); // set default value


        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            // TODO: Read selections from controls, update your g_serialSettings here

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
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

        while (keepPolling) {
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



