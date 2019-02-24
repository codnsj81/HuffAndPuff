#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../../Headers/Include.h"
#include <array>
#pragma comment(lib, "ws2_32")
using namespace std;

static const int EVT_RECV = 0;
static const int EVT_SEND = 1;
static const int EVT_MOVE = 2;
static const int EVT_PLAYER_MOVE = 3;

struct XMFLOAT3
{
	float x;
	float y;
	float z;

	XMFLOAT3() = default;

	XMFLOAT3(const XMFLOAT3&) = default;
	XMFLOAT3& operator=(const XMFLOAT3&) = default;

	XMFLOAT3(XMFLOAT3&&) = default;
	XMFLOAT3& operator=(XMFLOAT3&&) = default;
};

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped; // overlapped 구조체
	SOCKET sock;
	char buf[BUFSIZE + 1];	// 응용 프로그램 버퍼
	int recvbytes;	// 송, 수신 바이트 수
	int sendbytes;
	WSABUF wsabuf;	// WSABUF 구조체
};

class Client
{
public:
	SOCKET m_sock;
	SOCKETINFO m_sockinfo;
	XMFLOAT3 m_pos;
	float m_velocity = 0.f;
	bool m_connected = false;
};


array<Client, NUM_OF_PLAYER> g_clients;

// SOCKET client_sock; // accept 리턴 값, 두 스레드가 접근하기 때문에 전역으로.
HANDLE hReadEvent, hWriteEvent; // client_sock 보호하기 위한.

// 비동기 입출력 시작과 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);
DWORD WINAPI AcceptThread(LPVOID arg);
void CALLBACK CompletionRoutine(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK PutPlayerPos(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
// 함수들
void SendPacket(int, void*);

// 오류 출력 함수
void err_quit(const char *msg);
void err_display(const char *msg);
void err_display(int errcode);

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화.
	WSADATA WSAData; 
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << ("Error - Can not load 'winsock.dll' file") << endl;
		return 1;
	}

	// 이벤트 객체 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL); /// WorkerThread 스레드가 g_clients 변수 값을 읽었음을 메인 스레드에 알리는 용도
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL); /// 메인 스레드가 g_clients 변수 값을 변경했음을 alertable wait 상태인 WorkerThread 스레드에 알리는 용도
	if (hWriteEvent == NULL) return 1;

	// worker thread 생성
	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	// accept thread 생성
	HANDLE hThread2 = CreateThread(NULL, 0, AcceptThread, NULL, 0, NULL);
	if (hThread == NULL || hThread2 == NULL) return 1;


	while (true) {
		// 메인 함수가 끝나버려.. ㅜㅜ 
	}
	CloseHandle(hThread);
	CloseHandle(hThread2);
	return 0;
}

// 새로 접속해 오는 클라이언트를 받는 역할
DWORD WINAPI AcceptThread(LPVOID arg)
{
	int retval;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) 
		err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	int id{ -1 };

	while (1) {
		WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 대기

		// accept 
		/// 클라이언트가 접속할 때마다
		SOCKET client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		/// 클라이언트 id 부여
		else {
			for (int i = 0; i < NUM_OF_PLAYER; ++i) {  
				if (false == g_clients[i].m_connected) {
					id = i;
					break;
				}
			}
			if (-1 == id) { /// 접속 유저 초과
				cout << ("Excceded Users") << std::endl;
				continue;
			}

			g_clients[id].m_connected = true;
			g_clients[id].m_sock = client_sock;
		}


		// 넘겨주고, 받아서 쓰기.
		// p->overlapped.hEvent = (HANDLE)p->sock;

		// 여기서 할 필요가 없네!!
		//if (id == 1) {
		//	//  다른 클라이언트에게 새 클라이언트 1의 접속 정보를 전송한다.
		//	// --------- Process --------------
		//	// 1. 고정 길이 패킷 전송
		//	{
		//		packet_info packetinfo;
		//		packetinfo.type = sc_put_player;
		//		packetinfo.id = id;
		//		packetinfo.size = sizeof(XMFLOAT3);
		//		/// SOCKETINFO 할당, 초기화.
		//		SOCKETINFO *p = new SOCKETINFO;
		//		ZeroMemory(&p->overlapped, sizeof(p->overlapped));
		//		p->sock = client_sock;
		//		/// SetEvent(hReadEvent); //// 필요 없음.
		//		p->recvbytes = p->sendbytes = 0;
		//		// p->wsabuf.buf에 데이터 넣어줌.
		//		memcpy(p->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
		//		p->wsabuf.len = BUFSIZE;

		//		DWORD sendBytes;
		//		if (WSASend(g_clients[id - 1].m_sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
		//			if (WSAGetLastError() != WSA_IO_PENDING) {
		//				printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
		//			}
		//		}
		//	}
		//	// 2. 가변 길이 패킷 전송
		//	{
		//		XMFLOAT3 playerpos;
		//		playerpos = 
		//		/// SOCKETINFO 할당, 초기화.
		//		SOCKETINFO *p = new SOCKETINFO;
		//		ZeroMemory(&p->overlapped, sizeof(p->overlapped));
		//		p->sock = client_sock;
		//		/// SetEvent(hReadEvent); //// 필요 없음.
		//		p->recvbytes = p->sendbytes = 0;
		//		// p->wsabuf.buf에 데이터 넣어줌.
		//		memcpy(p->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
		//		p->wsabuf.len = BUFSIZE;
		//		// 새 클라이언트에게 다른 클라이언트의 정보 리턴.
		//		if (WSASend(p->sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
		//			if (WSAGetLastError() != WSA_IO_PENDING) {
		//				printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
		//			}
		//		}
		//	}

		//}
		

		SetEvent(hWriteEvent);
	}

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 비동기 입출력 시작 
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while (1) {
		while (1) {
			DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE); // 여기서부터 alertable wait상태.
			// alertable wait 상태를 벗어나면
			if (result == WAIT_OBJECT_0) break;  // 새 클라이언트가 접속한 경우 - 루프 벗어나야 함
			if (result != WAIT_IO_COMPLETION) return 1; // 비동기 입출력 작업과 이에 따른 완료 루틴 호출이 끝난 경우 - 다시 alertable wait 상태에 진입
		}

		// 접속한 클라이언트 정보 출력
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		int id{ -1 };
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (g_clients[i].m_connected == true)
				id = i; // 신규로 접속한 클라이언트의 id를 알아내야 함
		}
		if (id == -1) return 1;

		getpeername(g_clients[id].m_sock, (SOCKADDR *)&clientaddr, &addrlen);
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 소켓 정보 구조체 할당과 초기화
		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) {
			cout << ("[오류] 메모리가 부족합니다!") << endl;
			return 1;
		}

		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = g_clients[id].m_sock;
		SetEvent(hReadEvent); // client_sock 변수 값을 읽어가는 즉시 hReadEvent를 신호 상태로
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// 비동기 입출력 시작
		DWORD recvbytes;
		DWORD flags = 0;
		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes,
			&flags, &ptr->overlapped, CompletionRoutine);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				err_display("WSARecv()");
				return 1;
			}
		}
	}
	return 0;
}

// 비동기 입출력 처리 함수(입출력 완료 루틴)
void CALLBACK CompletionRoutine(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	struct SOCKETINFO *socketInfo;
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	// recv 했을 때, 몇 번째 소켓인지, 등등의 정보 알아야.. overlapped 구조체를 통해 알아야 한다.
	socketInfo = (struct SOCKETINFO *)overlapped; // 변경할 수 없는 overlapped 구조체를 타입 캐스팅. SOCKETINFO의 구조체의 첫 멤버 변수가 overlapped이기 때문에,, 걍 꼼수임,, callback 함수의 인자로 들어오는 것 중에 정보가 없으므로.


	if (dataBytes == 0) // recv로 0 바이트를 읽었을 때 ? -> 접속된 클라이언트에서 소켓 접속을 끈 것 (CloseSocket()을 호출 한 것.)
	{
		// 클라에서 끊었는데 굳이 잡고 있을 필요 X
		closesocket(socketInfo->sock);
		free(socketInfo);
		return;
	}

	// 소켓 정보에서 받은 데이터가 0의 크기라면, 처음 받은 것임. 즉 최초 callback인 경우.
	if (socketInfo->recvbytes == 0)
	{
		// WSARecv(최초 대기에 대한)의 콜백일 경우
		socketInfo->recvbytes = dataBytes; /// 이만큼 받았으니까 이만큼 받음 ㅇㅅㅇ
		socketInfo->sendbytes = 0; /// 하나도 안 보냈음 아직 ㅇㅅㅇ
		socketInfo->wsabuf.buf = socketInfo->buf;
		socketInfo->wsabuf.len = socketInfo->recvbytes; /// 실제로 받은 데이터 만큼만 보내야 한다.
		  
		cout << "TRACE - Receive message : " << socketInfo->buf << " ( " << dataBytes << " bytes) " << endl;

		// 데이터를 받고 난 후.
		// 1. 받아 올 패킷의 정보를 알아내야 한다. (고정 길이)
		packet_info packetinfo;
		{
			ZeroMemory(&packetinfo, sizeof(packetinfo));
			memcpy(&packetinfo, socketInfo->buf, sizeof(packetinfo));
		}
		// 2. 패킷 타입을 통해 알맞는 처리를 해 준다.
		{
			switch (packetinfo.type) {
			case cs_put_player:
			{
				if(packetinfo.id == 0){
					// 클라 0 : connect 완료했으니까 나 들어왔다는 패킷도 보낼게
					// 서버 : 그럼 g_clients[0].m_pos 에 니 위치 정보를 갱신할게!! 
					{
						XMFLOAT3 pos;
						memcpy(&pos, socketInfo->buf + sizeof(packetinfo), sizeof(pos));
						g_clients[0].m_pos = pos;
					}
					
				}
				else {
					// 클라 1 : connect 완료했으니까 나 들어왔다는 패킷도 보낼게
					// 서버 : 그럼 g_clients[1].m_pos 에 니 위치 정보를 갱신할게!! 
					{
						XMFLOAT3 pos;
						memcpy(&pos, socketInfo->buf + sizeof(packetinfo), sizeof(pos));
						g_clients[1].m_pos = pos;
					}
					// 서버 : 클라 0한테 너의 접속 정보 보낼게!
					{
						// 1. 고정 길이
						packet_info packetinfo;
						packetinfo.type = sc_put_player;
						packetinfo.id = 1;
						packetinfo.size = sizeof(XMFLOAT3);
						/// SOCKETINFO 할당, 초기화.
						SOCKETINFO *p = new SOCKETINFO;
						ZeroMemory(&p->overlapped, sizeof(p->overlapped));
						p->sock = g_clients[1].m_sock;
						p->recvbytes = p->sendbytes = 0;
						memcpy(p->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
						p->wsabuf.len = BUFSIZE;

						DWORD sendBytes;
						if (WSASend(g_clients[0].m_sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
						// 2. 가변 길이
						XMFLOAT3 pos = g_clients[1].m_pos;
						/// SOCKETINFO 할당, 초기화.
						ZeroMemory(&p, sizeof(p));
						ZeroMemory(&(p->overlapped), sizeof(p->overlapped));
						p->sock = g_clients[1].m_sock;
						p->recvbytes = p->sendbytes = 0;
						memcpy(p->wsabuf.buf, &(pos), sizeof(pos));
						p->wsabuf.len = BUFSIZE;
						// 새 클라이언트에게 다른 클라이언트의 정보 리턴.
						if (WSASend(g_clients[0].m_sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
					}
					// 서버 : 너한테 클라 0 정보 보낼게!
					{
						// 1. 고정 길이
						packet_info packetinfo;
						packetinfo.type = sc_put_player;
						packetinfo.id = 0; // 0의 정보를 보내는 거니께,,
						packetinfo.size = sizeof(XMFLOAT3);
						/// SOCKETINFO 할당, 초기화.
						SOCKETINFO *p = new SOCKETINFO;
						ZeroMemory(&p->overlapped, sizeof(p->overlapped));
						p->sock = g_clients[0].m_sock;
						p->recvbytes = p->sendbytes = 0;
						memcpy(p->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
						p->wsabuf.len = BUFSIZE;

						DWORD sendBytes;
						if (WSASend(g_clients[1].m_sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
						// 2. 가변 길이
						XMFLOAT3 pos = g_clients[0].m_pos;
						/// SOCKETINFO 할당, 초기화.
						ZeroMemory(&p, sizeof(p));
						ZeroMemory(&p->overlapped, sizeof(p->overlapped));
						p->sock = g_clients[0].m_sock;
						p->recvbytes = p->sendbytes = 0;
						memcpy(p->wsabuf.buf, &(pos), sizeof(pos));
						p->wsabuf.len = BUFSIZE;
						// 새 클라이언트에게 다른 클라이언트의 정보 리턴.
						if (WSASend(g_clients[1].m_sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
					}
				}
			}
			case cs_move:
			{
				// 해당 클라이언트의 이동 완료된 좌표를 받아온다.
				XMFLOAT3 pos;
				char buf[BUFSIZE];
				memcpy(buf, socketInfo->buf + sizeof(packetinfo), sizeof(buf));
				// 일단 서로 띄우는거부터 하고!!
			}
			break;
			case cs_move_left:
			{
				//g_clients[packetinfo.id].m_x -= g_clients[packetinfo.id].m_velocity;
			}
			break;
			case cs_move_top:
			{
				// g_clients[packetinfo.id].m_y -= g_clients[packetinfo.id].m_velocity;
			}
			break;
			case cs_move_right:
			{

			}
			break;
			case cs_move_bottom:
			{

			}
			break;
			}
		}
		


		// ------------------------------------------------
		// 여기부터는 생략해도 될 것 같아. 일단 주석 처리 한다.

		//// send, recv 하기 전에 무조건 0으로 초기화 해 줘야.
		//memset(&(socketInfo->overlapped), 0x00, sizeof(WSAOVERLAPPED)); // overlapped IO가 종료하고, recv가 종료되었으므로 또 recv를 하려면 overlapped 구조체를 0으로 초기화한다. 미리 함.

		//// 클라이언트에서 준 것을 그대로 리턴 하는 것.
		//if (WSASend(socketInfo->sock, &(socketInfo->wsabuf), 1, &sendBytes, 0, &(socketInfo->overlapped), CompletionRoutine) == SOCKET_ERROR)
		//{
		//	if (WSAGetLastError() != WSA_IO_PENDING)
		//	{
		//		printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
		//	}
		//}
	}
	else // send callback
	{
		// WSASend(응답에 대한)의 콜백일 경우
		socketInfo->sendbytes += dataBytes;
		socketInfo->recvbytes = 0;
		socketInfo->wsabuf.len = BUFSIZE;
		socketInfo->wsabuf.buf = socketInfo->buf;

		cout << "TRACE - Send message : " << socketInfo->buf << "( " << dataBytes << "bytes)" << endl;

		// ------------------------------------------------
		// 여기부터는 생략해도 될 것 같아. 일단 주석 처리 한다.
		//memset(&(socketInfo->overlapped), 0x00, sizeof(WSAOVERLAPPED)); // overlapped IO가 종료하고, recv가 종료되었으므로 또 recv를 하려면 overlapped 구조체를 0으로 초기화한다. 미리 함.

		//if (WSARecv(socketInfo->sock, &socketInfo->wsabuf, 1, &receiveBytes, &flags, &(socketInfo->overlapped), CompletionRoutine) == SOCKET_ERROR)
		//{
		//	if (WSAGetLastError() != WSA_IO_PENDING)
		//	{
		//		printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
		//	}
		//}
	}
}

void CALLBACK PutPlayerPos(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	struct SOCKETINFO *socketInfo;
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	socketInfo = (struct SOCKETINFO *)overlapped; 


	if (dataBytes == 0) { // 접속된 클라이언트에서 소켓 접속을 끈 것 
		closesocket(socketInfo->sock);
		free(socketInfo);
		return;
	}

	// 최초 callback인 경우.
	if (socketInfo->recvbytes == 0) {
	}
	else // send callback
	{
		// WSASend(응답에 대한)의 콜백일 경우
		socketInfo->sendbytes += dataBytes;
		socketInfo->recvbytes = 0;
		socketInfo->wsabuf.len = BUFSIZE;
		socketInfo->wsabuf.buf = socketInfo->buf;

		cout << "TRACE - Send message : " << socketInfo->buf << "( " << dataBytes << "bytes)" << endl;

		// ------------------------------------------------
		// 여기부터는 생략해도 될 것 같아. 일단 주석 처리 한다.
		//memset(&(socketInfo->overlapped), 0x00, sizeof(WSAOVERLAPPED)); // overlapped IO가 종료하고, recv가 종료되었으므로 또 recv를 하려면 overlapped 구조체를 0으로 초기화한다. 미리 함.

		//if (WSARecv(socketInfo->sock, &socketInfo->wsabuf, 1, &receiveBytes, &flags, &(socketInfo->overlapped), CompletionRoutine) == SOCKET_ERROR)
		//{
		//	if (WSAGetLastError() != WSA_IO_PENDING)
		//	{
		//		printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
		//	}
		//}
	}
}

void SendPacket(int id, void *ptr)
{
	//unsigned char *packet = reinterpret_cast<unsigned char *>(ptr);
	//EXOVER *s_over = new EXOVER;
	//s_over->event_type = EVT_SEND;
	//memcpy(s_over->m_iobuf, packet, packet[0]);
	//s_over->m_wsabuf.buf = s_over->m_iobuf;
	//s_over->

	//int retval;
	//DWORD sendbytes;
	//retval = WSASend(g_clients[id].m_sockinfo.sock, (g_clients[id].m_sockinfo.wsabuf), 1, &sendbytes, 0, &(g_clients[id].m_sockinfo.overlapped), CompletionRoutine);
	//if (retval == SOCKET_ERROR) {
	//	if (WSAGetLastError() != WSA_IO_PENDING) {
	//		err_display("WSASend()");
	//		return;
	//	}
	//}
}
// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}