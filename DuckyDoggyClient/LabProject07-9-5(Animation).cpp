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
HANDLE hThread, hSendThread;
HWND		g_hWnd;
wchar_t		g_ipbuf[50];		// ip 입력 받는 버퍼
player_info g_myinfo;
player_info g_otherinfo;
networking_state g_networkState = recv_none;

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped; // overlapped 구조체
	SOCKET sock;
	char buf[BUFSIZE + 1];	// 응용 프로그램 버퍼. 실제 버퍼가 들어갈 메시지 버퍼.
	int recvbytes;	// 송, 수신 바이트 수
	int sendbytes;
	WSABUF wsabuf;	// WSABUF 구조체. 버퍼 자체가 아닌, 버퍼의 포인터
};


ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
int InitializeNetwork();
void CloseNetwork();
static DWORD WINAPI RecvThread(LPVOID arg);
static DWORD WINAPI SendThread(LPVOID arg);
int recvn(SOCKET s, char *buf, int len, int flags);


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
			char p[128] = "127.0.0.1";
			wcscpy(g_ipbuf, L"127.0.0.1");
			InitializeNetwork();
			 gGameFramework.SetPlayerType(player_doggy);
		}
		break;
		case ID_NETWORK_ACCESS_USER: // 더기로 접속
		{
			g_myinfo.type = player_ducky;
			char p[128] = "127.0.0.1";
			wcscpy(g_ipbuf, L"127.0.0.1");
			InitializeNetwork();
			gGameFramework.SetPlayerType(player_ducky);
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
		CloseNetwork();
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
	g_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (g_sock == INVALID_SOCKET)
		MessageBoxW(g_hWnd, L"socket()", MB_OK, MB_OK);

	// 서버 정보 객체를 설정 한다. 
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	_bstr_t b(g_ipbuf);// convert wchar_t to char
	serveraddr.sin_addr.s_addr = inet_addr(b);
	serveraddr.sin_port = htons(SERVERPORT);

	// 연결을 요청한다.
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

		bool NoDelay = TRUE;
		setsockopt(g_sock, IPPROTO_TCP, TCP_NODELAY, (const char FAR*)&NoDelay, sizeof(NoDelay));


		// recv 전용 스레드를 만든다.
		hThread = CreateThread(NULL, 0, RecvThread, (LPVOID)g_sock, 0, NULL);
		if (NULL == hThread)
			CloseHandle(hThread);


		//// 서버에게 클라이언트 초기 정보를 보낸다.
		//g_myinfo.x = gGameFramework.GetPlayer()->GetPosition().x;
		//g_myinfo.y = gGameFramework.GetPlayer()->GetPosition().y;
		//g_myinfo.z = gGameFramework.GetPlayer()->GetPosition().z;
		//int retval;
		//char buf[BUFSIZE];
		//// 고정
		//packet_info packetinfo;
		//packetinfo.type = cs_put_player;
		//packetinfo.size = sizeof(player_info);
		//packetinfo.id = -1;
		//packetinfo.sock = g_sock;
		//memcpy(buf, &packetinfo, sizeof(packetinfo));
		//// 가변 (고정 데이터에 가변 데이터 붙이는 형식으로)
		//memcpy(buf + sizeof(packetinfo), &g_myinfo, sizeof(player_info));
		//// 전송
		//retval = send(g_sock, buf, BUFSIZE, 0);
		//if (retval == SOCKET_ERROR) {
		//	MessageBoxW(g_hWnd, L"send()", L"send() - cs_put_player", MB_OK);
		//}

	}
}

void CloseNetwork()
{
	// 핸들 종료
	CloseHandle(hThread);
	// closesocket()
	closesocket(g_sock);

	// 윈속 종료
	WSACleanup();
}


DWORD __stdcall RecvThread(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval{ -1 };

	while (true) {
		packet_info packetinfo = {};
		player_info playerinfo = {};
		char buf[BUFSIZE];

		// 고정 길이. // 무슨 패킷을 받아오게 될지 미리 알아낸다.
		{
			int receiveBytes = recvn(client_sock, buf, BUFSIZE, 0);
			if (receiveBytes > 0)
				memcpy(&packetinfo, buf, sizeof(packetinfo));
		}
		// 가변 길이. 
		switch (packetinfo.type) {
		case sc_notify_yourinfo: 
		{
			int id = g_myinfo.id = packetinfo.id;			// playerinfo의 주인의 id를 받아온다.
			memcpy(&(g_myinfo), buf + sizeof(packetinfo), sizeof(g_myinfo));
			// g_networkState = recv_playerinfo;
			gGameFramework.GetPlayer()->SetPosition(XMFLOAT3{ g_myinfo.x, g_myinfo.y, g_myinfo.z });
			gGameFramework.GetPlayer()->SetMain(true);
		}
		break;
		case sc_notify_playerinfo:
		{
			int id = packetinfo.id;			// playerinfo의 주인의 id를 받아온다.
			memcpy(&(g_otherinfo), buf + sizeof(packetinfo), sizeof(g_otherinfo));
			// 1)
			//if (g_myinfo.type == player_doggy)
			//	gGameFramework.GetDucky()->SetPosition(XMFLOAT3{ g_otherinfo.x, g_otherinfo.y, g_otherinfo.z });
			//else
			//	gGameFramework.GetDoggy()->SetPosition(XMFLOAT3{ g_otherinfo.x, g_otherinfo.y, g_otherinfo.z });
			// 2)
			if (g_myinfo.type == player_doggy) {
				gGameFramework.SetPlayerPos(player_ducky, XMFLOAT3{ g_otherinfo.x, g_otherinfo.y, g_otherinfo.z });
				gGameFramework.SetPlayerAnimationSet(player_ducky, g_otherinfo.animationSet);
			}
			else {
				gGameFramework.SetPlayerPos(player_doggy, XMFLOAT3{ g_otherinfo.x, g_otherinfo.y, g_otherinfo.z });
				gGameFramework.SetPlayerAnimationSet(player_doggy, g_otherinfo.animationSet);
			}
		}
		break;
		case sc_put_player:
		{
			// int id = packetinfo.id; // 새로 접속했거나 이미 있던 클라이언트를 추가하기 위해, id를 받아온다.
			g_otherinfo.connected = true;
			memcpy(&(g_otherinfo), buf + sizeof(packetinfo), sizeof(g_otherinfo));
			// g_networkState = recv_otherinfo;
			if (g_myinfo.type == player_doggy) {
				gGameFramework.SetPlayerPos(player_ducky, XMFLOAT3{ g_otherinfo.x, g_otherinfo.y, g_otherinfo.z });
				gGameFramework.SetPlayerAnimationSet(player_ducky, g_otherinfo.animationSet);
			}
			else {
				gGameFramework.SetPlayerPos(player_doggy, XMFLOAT3{ g_otherinfo.x, g_otherinfo.y, g_otherinfo.z });
				gGameFramework.SetPlayerAnimationSet(player_doggy, g_otherinfo.animationSet);
			}
		}
		break;
		}
	}

	gGameFramework.FrameAdvance();

	return 0;
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

DWORD __stdcall SendThread(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval{ -1 };

	while (true) {
		packet_info packetinfo = {};
		player_info playerinfo = {};
		char buf[BUFSIZE];

		//@ 서버한테 위치 보내기
	//	if (true == g_myinfo.connected
	//		&& gGameFramework.GetPlayer()->GetMoveState() == STATE_GROUND) {
	//		player_info playerinfo;
	//		XMFLOAT3 pos = gGameFramework.GetPlayer()->GetPosition();
	//		playerinfo.x = pos.x; playerinfo.y = pos.y; playerinfo.z = pos.z;
	//		playerinfo.type = g_myinfo.type;
	//		int retval;
	//		/// 고정
	//		packet_info packetinfo;
	//		packetinfo.type = cs_move;
	//		packetinfo.size = sizeof(player_info);
	//		packetinfo.id = g_myinfo.id;
	//		char buf[BUFSIZE];
	//		memcpy(buf, &packetinfo, sizeof(packetinfo));
	//		/// 가변 (고정 데이터에 가변 데이터 붙이는 형식으로)
	//		memcpy(buf + sizeof(packetinfo), &playerinfo, sizeof(player_info));
	//		retval = send(g_sock, buf, BUFSIZE, 0);
	//		if (retval == SOCKET_ERROR) {
	//			MessageBoxW(g_hWnd, L"send()", L"send() - cs_move", MB_OK);
	//		}
	//	}
	}
}