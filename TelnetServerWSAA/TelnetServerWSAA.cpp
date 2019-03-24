// TelnetServerWSAA.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TelnetServerWSAA.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"
#define WM_SOCKET WM_USER + 1
#include <sstream>
BOOL CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
SOCKET clients[64];
int numClients;
char *ids[128];
SOCKET clientsConnected[64];
int numClientsConnected;
char fileBuf[256];
char sendBuf[512];
char *inform[64];
char cmdBuf[256];
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	WNDCLASS wndclass;
	CHAR *providerClass = "AsyncSelect";
	HWND window;

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)providerClass;

	if (RegisterClass(&wndclass) == 0)
		return NULL;

	// Create a window
	if ((window = CreateWindow((LPCWSTR)providerClass, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL)) == NULL)
		return -1;
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TELNETSERVERWSAA, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);





    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TELNETSERVERWSAA));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TELNETSERVERWSAA));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TELNETSERVERWSAA);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
	   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
	   300, 10, 400, 450, hWnd, NULL, GetModuleHandle(NULL), NULL);

   SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   SOCKADDR_IN addr;
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(9000);

   bind(listener, (SOCKADDR *)&addr, sizeof(addr));
   listen(listener, 5);
   WSAAsyncSelect(listener, hWnd, WM_SOCKET, FD_ACCEPT);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
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
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket(wParam);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKADDR_IN clientAddr;
			int clientAddrLen = sizeof(clientAddr);

			SOCKET client = accept(wParam, (SOCKADDR *)&clientAddr, &clientAddrLen);

			char *ip = inet_ntoa(clientAddr.sin_addr);

			char portBuf[8];
			_itoa(ntohs(clientAddr.sin_port), portBuf, 10);

			char *msg = "Hello client";
			send(client, msg, strlen(msg), 0);
			clients[numClients] = client;
			numClients++;


			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char sendBuf[512];
			bool connected = false;
			char buf[256];
			int numberInArr = 0;
			int ret = recv(wParam, buf, sizeof(buf), 0);
			buf[ret] = 0;
			for (int i = 0; i < numClientsConnected; i++) {
				if (clientsConnected[i] == wParam) {
					connected = true;
					numberInArr = i;
					break;
				}
			}
			if (connected) {
				if (buf[ret - 1] == '\n')
					buf[ret - 1] = 0;
				sprintf(cmdBuf, "%s > c:\\test_server\\out.txt", buf);
				system(cmdBuf);
				SendDlgItemMessageA(hWnd, NULL, LB_ADDSTRING,
					0, (LPARAM)cmdBuf);
				FILE *f = fopen("c:\\test_server\\out.txt", "r");
				while (fgets(fileBuf, sizeof(fileBuf), f))
				{
					send(wParam, fileBuf, strlen(fileBuf), 0);
				}
				fclose(f);
			}
			else {
				int found = 0;
				char pass[64];
				char userName[64];
				FILE *f = fopen("Text.txt", "r");
				while (fgets(fileBuf, sizeof(fileBuf), f))
				{
					if (strcmp(buf, fileBuf) == 0)
					{
						ret = sscanf(buf, "%s %s", userName, pass);
						found = 1;
						break;
					}
				}
				fclose(f);

				if (found == 1)
				{
					char *msg = "Dang nhap thanh cong. Hay nhap lenh.\n";
					send(wParam, msg, strlen(msg), 0);
					clientsConnected[numClientsConnected] = wParam;
					numClientsConnected++;
					
					inform[0] = (char *)malloc(strlen(userName) + 1);
					memcpy(inform[0], userName, strlen(userName) + 1);
					strcat(inform[0], " login ss");
					SendDlgItemMessageA(hWnd, NULL, LB_ADDSTRING,
						0, (LPARAM)inform[0]);
					connected = true;
					break;
				}
				else
				{
					char *msg = "Dang nhap that bai. Hay thu lai.\n";
					send(wParam, msg, strlen(msg), 0);
				}
				
			}
		}
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

BOOL CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SOCKET)
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket(wParam);
			return FALSE;
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKET client = accept(wParam, NULL, NULL);
			//printf("New client accepted: %d\n", client);


			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}

		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			closesocket(wParam);
		}
	}

	return TRUE;
}