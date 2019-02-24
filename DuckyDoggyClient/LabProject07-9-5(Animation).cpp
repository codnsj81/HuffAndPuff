// LabProject07-9-5.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "LabProject07-9-5(Animation).h"
#include "GameFramework.h"
#include "../Headers/Include.h"
#include <comdef.h>

#define MAX_LOADSTRING 100


HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];
CGameFramework					gGameFramework;

// 서버와 통신
SOCKET g_sock;
char buf[BUFSIZE];	// 데이터 버퍼
HANDLE hThread;
HWND		g_hWnd;
wchar_t		g_ipbuf[50];		// ip 입력 받는 버퍼
player_info g_myinfo;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
int InitializeNetwork();
void CloseNetwork();
static DWORD WINAPI RecvThread(LPVOID arg);


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT0795ANIMATION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT0795ANIMATION));

	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			gGameFramework.FrameAdvance();
		}
	}
	gGameFramework.OnDestroy();

	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABPROJECT0795ANIMATION));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);//NULL;//MAKEINTRESOURCE(IDC_LABPROJECT0795ANIMATION);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

	// 생성한 hWnd를 글로벌 변수 g_hWnd에 넣어 준다. (0211)
	g_hWnd = hMainWnd;

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
	::UpdateWindow(hMainWnd);

	return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
		//	::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		// menu 추가 (0211)
		case ID_NETWORK_ACCESS_DEFAULT: // 도기로 접속
		{
			g_myinfo.type = player_doggy;
			// 네트워크 접속, 접속 ip는 default 값
			char p[128] = "127.0.0.1";
			wcscpy(g_ipbuf, L"127.0.0.1");
			InitializeNetwork();
		}
		break;
		case ID_NETWORK_ACCESS_USER: // 더기로 접속
		{
			g_myinfo.type = player_ducky;
			// 네트워크 접속, ip 입력 받음 -> 우선 X
			InitializeNetwork();
		}
		break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//CloseNetwork();
		::PostQuitMessage(0);
		break;
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}

int InitializeNetwork()
{
	// @
	int retval;

	// 윈속을 초기화 한다.
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		MessageBoxW(g_hWnd, L"Can not load 'winsock.dll' file", MB_OK, MB_OK);
		return 1;
	}

	// 소켓을 생성한다.
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET) MessageBoxW(g_hWnd, L"socket()", MB_OK, MB_OK);

	// recv 전용 스레드를 만든다.
	//hThread = CreateThread(NULL, 0, RecvThread, (LPVOID)g_sock, 0, NULL);
	//if (NULL == hThread)	CloseHandle(hThread);

	// 서버 정보 객체를 설정 한다. 
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	/// convert wchar_t to char 
	_bstr_t b(g_ipbuf);
	///
	serveraddr.sin_addr.s_addr = inet_addr(b);
	serveraddr.sin_port = htons(SERVERPORT); 
	retval = connect(g_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR) {
		// 소켓을 종료한다.
		closesocket(g_sock);
		// Winsock End
		WSACleanup();
		MessageBoxW(g_hWnd, L"connect()", MB_OK, MB_OK);
	}
	else {
		MessageBoxW(g_hWnd, L"Connected", L"알림", MB_OK);
		g_myinfo.connected = true;

		// 서버에게 클라이언트 초기 정보를 보낸다.
		if (true == g_myinfo.connected) {
			int retval;
			// 고정
			packet_info packetinfo;
			packetinfo.type = cs_put_player;
			packetinfo.size = sizeof(player_info);
			char buf[BUFSIZE];
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			retval = send(g_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				MessageBoxW(g_hWnd, L"send()", L"send() - cs_put_player (고정)", MB_OK);
			}
			// 가변
			player_info playerinfo = g_myinfo;
			ZeroMemory(buf, sizeof(buf));
			memcpy(buf, &playerinfo, sizeof(playerinfo));
			retval = send(g_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				MessageBoxW(g_hWnd, L"send()", L"send() - cs_put_player (가변)", MB_OK);
			}
		}
	}


}

void CloseNetwork()
{
	// closesocket()
	closesocket(g_sock);

	// 윈속 종료
	WSACleanup();
}

DWORD __stdcall RecvThread(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;


	return 0;
}
