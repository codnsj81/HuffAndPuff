#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../../Headers/Include.h"
#include <array>
using namespace std;
#pragma comment(lib, "ws2_32")


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
	char buf[BUFSIZE + 1];	// 응용 프로그램 버퍼. 실제 버퍼가 들어갈 메시지 버퍼.
	int recvbytes;	// 송, 수신 바이트 수
	int sendbytes;
	WSABUF wsabuf;	// WSABUF 구조체. 버퍼 자체가 아닌, 버퍼의 포인터
};

class Client
{
public:
	SOCKET m_sock;
	SOCKETINFO m_sockinfo;
	XMFLOAT3 m_pos;
	player_type m_type;
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

	// 사용할 소켓을 생성한다.
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) err_quit("[오류] Invalid socket");

	// 서버 정보 객체를 설정한다.
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = PF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// 소켓을 설정한다.
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		err_quit("[오류] bind()");
	}

	// 수신 대기열을 생성한다.
	retval = listen(listen_sock, 2);
	if (retval == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		err_quit("[오류] listen()");
	}

	//// accept thread 생성
	//HANDLE hThread = CreateThread(NULL, 0, AcceptThread, NULL, 0, NULL);
	//if (hThread == NULL) return 1;


	SOCKADDR_IN client_addr;
	int addr_length = sizeof(SOCKADDR_IN);
	memset(&client_addr, 0, addr_length);
	SOCKET client_sock;
	SOCKETINFO* socket_info;
	DWORD recv_bytes;
	DWORD flags;

	int id{ -1 };
	while (1) {
		// accept -> 클라이언트와 연결 -> 클라이언트의 주소 구조체 변수를 받음 -> 주소 정보 얻어옴.
		client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &addr_length);
		if (client_sock == INVALID_SOCKET) {
			err_display("[오류] accept()");
			return 1;
		}
		
		// 클라이언트 정보를 만들어 온다.
		socket_info = new SOCKETINFO;
		memset((void*)socket_info, 0x00, sizeof(SOCKETINFO));
		socket_info->sock = client_sock;
		socket_info->wsabuf.len = MAX_BUFSIZE;
		socket_info->wsabuf.buf = socket_info->buf;
		flags = 0;

		// 넘겨주고, 받아서 쓴다.
		// socket_info->overlapped.hEvent = (HANDLE)socket_info->sock;

		// 중첩 소켓 지정 -> 완료시 실행될 함수 넘겨줌
		// Recv 하고 바로 끝난다 -> overlapped IO라서. 실제 버퍼에 데이터가 들어오는 건 나중 일.
		if (WSARecv(socket_info->sock, &socket_info->wsabuf, 1, &recv_bytes, &flags, &(socket_info->overlapped), CompletionRoutine)) {
			// overlapped IO는 pending 에러가 나와야 함.
			if (WSAGetLastError() != WSA_IO_PENDING) {
				err_quit("[오류] IO Pending Fail");
			}
		}
		// 접속한 클라이언트의 정보를 띄운다.
		cout << "[알림] [" << id << "] 번째 클라이언트 접속 : IP 주소 = " << inet_ntoa(client_addr.sin_addr) << ", 포트 번호 = " << ntohs(client_addr.sin_port) << endl;


		// 클라이언트에 id를 부여한다.
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (false == g_clients[i].m_connected) {
				id = i;
				break;
			}
		}
		if (-1 == id) { /// 접속 유저 초과
			cout << ("Excceded Users") << endl;
			continue;
		}

		// g_clients[해당id] 배열 원소에 정보를 갱신한다.
		g_clients[id].m_connected = true;
		g_clients[id].m_sock = client_sock;
		memcpy(&(g_clients[id].m_sockinfo), socket_info, sizeof(SOCKETINFO));

	}
	closesocket(listen_sock);
	WSACleanup();
	return 0;

}

// 새로 접속해 오는 클라이언트를 받는 역할
DWORD WINAPI AcceptThread(LPVOID arg)
{


	return 0;
}

// 비동기 입출력 시작 
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;

	while (1) {
		while (1) {
			//DWORD result = WaitForSingleObjectEx(hWriteEvent, INFINITE, TRUE); // 여기서부터 alertable wait상태.
			// alertable wait 상태를 벗어나면
			//if (result == WAIT_OBJECT_0) break;  // 새 클라이언트가 접속한 경우 - 루프 벗어나야 함
			//if (result != WAIT_IO_COMPLETION) return 1; // 비동기 입출력 작업과 이에 따른 완료 루틴 호출이 끝난 경우 - 다시 alertable wait 상태에 진입
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
		//SetEvent(hReadEvent); // client_sock 변수 값을 읽어가는 즉시 hReadEvent를 신호 상태로
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


	// 접속된 클라이언트에서 소켓 접속을 끊었을 때.
	if (dataBytes == 0)
	{
		// recv로 0 바이트를 읽었을 때 ? -> 접속된 클라이언트에서 소켓 접속을 끈 것 (CloseSocket()을 호출 한 것.)
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

		// 데이터를 받고 난 후.
		// 1. 받아 올 패킷의 정보를 알아내야 한다. (고정 길이)
		packet_info packetinfo;
		{
			ZeroMemory(&packetinfo, sizeof(packetinfo));
			memcpy(&packetinfo, socketInfo->buf, sizeof(packetinfo));
		}
		cout << "Receive packet : " << packetinfo.type << " ( " << dataBytes << " bytes) " << endl;
		// 2. 패킷 타입을 통해 알맞는 처리를 해 준다.
		{
			switch (packetinfo.type) {
			case cs_put_player:
			{
				if (false == g_clients[1].m_connected) {
					// 클라 0 : connect 완료했으니까 나 들어왔다는 신호, 위치 정보 패킷 보낼게
					// 서버 : g_clients[0].m_pos에 니 위치 정보를 갱신할게!! 
					player_info playerinfo;
					{
						memcpy(&playerinfo, socketInfo->buf + sizeof(packetinfo), sizeof(playerinfo));
						g_clients[0].m_pos.x = playerinfo.x; g_clients[0].m_pos.y = playerinfo.y; g_clients[0].m_pos.z = playerinfo.z;
						g_clients[0].m_type = playerinfo.type;
						playerinfo.id = 0;
					}
					// 서버 : 너한테 id도 부여해서 갱신된 player_info를 알려 줄게!
					{
						// 1. 고정 길이
						packet_info packetinfo;
						packetinfo.type = sc_your_playerinfo;
						packetinfo.id = 0;
						packetinfo.size = sizeof(player_info);
						/*/// SOCKETINFO 할당, 초기화.
						SOCKETINFO *p = new SOCKETINFO;*/
						ZeroMemory(&(socketInfo->overlapped), sizeof(socketInfo->overlapped));
						//p->sock = g_clients[0].m_sock;
						//p->recvbytes = p->sendbytes = 0;
						memcpy(socketInfo->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
						//p->wsabuf.len = BUFSIZE;
						// 2. 가변 길이
						playerinfo.x = g_clients[0].m_pos.x;
						playerinfo.y = g_clients[0].m_pos.y;
						playerinfo.z = g_clients[0].m_pos.z;
						playerinfo.connected = true;
						/// SOCKETINFO 할당, 초기화.
						/*p->sock = g_clients[0].m_sock;
						p->recvbytes = p->sendbytes = 0;*/
						memcpy(socketInfo->wsabuf.buf + sizeof(packetinfo), &(playerinfo), sizeof(playerinfo));
						socketInfo->wsabuf.len = BUFSIZE;
						// 전송
						if (WSASend(socketInfo->sock, &(socketInfo->wsabuf), 1, &sendBytes, 0, &(socketInfo->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
					}

				}
				else if (true == g_clients[1].m_connected) {
					// 클라 1 : connect 완료했으니까 나 들어왔다는 패킷도 보낼게
					// 서버 : 그럼 g_clients[1]에 니 정보를 갱신할게!! 
					player_info playerinfo;
					{
						memcpy(&playerinfo, socketInfo->buf + sizeof(packetinfo), sizeof(playerinfo));
						g_clients[1].m_pos.x = playerinfo.x; g_clients[1].m_pos.y = playerinfo.y; g_clients[1].m_pos.z = playerinfo.z;
						g_clients[1].m_type = playerinfo.type;
						playerinfo.id = 1;
					}
					// 서버 : 너한테 id도 부여하고 갱신된 player_info를 보낼게!
					{
						// 1. 고정 길이
						packet_info packetinfo;
						packetinfo.type = sc_your_playerinfo;
						packetinfo.id = 1;
						packetinfo.size = sizeof(player_info);
						ZeroMemory(&(socketInfo->overlapped), sizeof(socketInfo->overlapped));
						memcpy(socketInfo->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
						// 2. 가변 길이
						playerinfo.x = g_clients[1].m_pos.x;
						playerinfo.y = g_clients[1].m_pos.y;
						playerinfo.z = g_clients[1].m_pos.z;
						playerinfo.connected = true;
						memcpy(socketInfo->wsabuf.buf + sizeof(packetinfo), &(playerinfo), sizeof(playerinfo));
						socketInfo->wsabuf.len = BUFSIZE;
						// 전송
						if (WSASend(socketInfo->sock, &(socketInfo->wsabuf), 1, &sendBytes, 0, &(socketInfo->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
					}
					// 서버 : 클라 0한테 너의 접속 정보 보낼게!
					{
						// 1. 고정 길이
						packet_info packetinfo;
						packetinfo.type = sc_put_player;
						packetinfo.id = 1;
						packetinfo.size = sizeof(player_info);
						/// SOCKETINFO 할당, 초기화.
						SOCKETINFO *p = new SOCKETINFO;
						memcpy(p, &(g_clients[0].m_sockinfo), sizeof(*p));
						ZeroMemory(&p->overlapped, sizeof(p->overlapped));
						p->recvbytes = p->sendbytes = 0;
						memcpy(p->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
						p->wsabuf.len = BUFSIZE;

						// 2. 가변 길이
						memcpy(p->wsabuf.buf + sizeof(packetinfo), &(playerinfo), sizeof(playerinfo));
						p->wsabuf.len = BUFSIZE;
						// 새 클라이언트에게 다른 클라이언트의 정보 리턴.
						if (WSASend(p->sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
					}
					// 서버 : 너한테는 클라 0 의 정보 보낼게!
					{
						// 1. 고정 길이
						packet_info packetinfo;
						packetinfo.type = sc_put_player;
						packetinfo.id = 0;
						packetinfo.size = sizeof(player_info);
						memcpy(socketInfo->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
						// 2. 가변 길이
						playerinfo.x = g_clients[0].m_pos.x;
						playerinfo.y = g_clients[0].m_pos.y;
						playerinfo.z = g_clients[0].m_pos.z;
						playerinfo.type = g_clients[0].m_type;
						playerinfo.connected = true;
						memcpy(socketInfo->wsabuf.buf + sizeof(packetinfo), &(playerinfo), sizeof(playerinfo));
						socketInfo->wsabuf.len = BUFSIZE;
						ZeroMemory(&(socketInfo->overlapped), sizeof(socketInfo->overlapped));
						// 전송
						if (WSASend(socketInfo->sock, &(socketInfo->wsabuf), 1, &sendBytes, 0, &(socketInfo->overlapped), PutPlayerPos) == SOCKET_ERROR) {
							if (WSAGetLastError() != WSA_IO_PENDING) {
								printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
							}
						}
					}
				}
			}
			break;
			case cs_move:
			{
				// 해당 클라이언트의 이동 완료된 좌표를 받아온다.
				player_info playerinfo;
				{
					memcpy(&playerinfo, socketInfo->buf + sizeof(packetinfo), sizeof(playerinfo));
					g_clients[playerinfo.id].m_pos.x = playerinfo.x;
					g_clients[playerinfo.id].m_pos.y = playerinfo.y;
					g_clients[playerinfo.id].m_pos.z = playerinfo.z;
				}

				// 0이면 1한테, 1이면 0한테 sc_notify_pos 패킷을 보낸다.
				{
					// 0. 받을 클라이언트의 id 구하기.
					int recvid{ -1 };
					if (playerinfo.id == 0)
						recvid = 1;
					else
						recvid = 0;

					// 1. 고정 길이
					packet_info packetinfo;
					packetinfo.type = sc_notify_pos;
					packetinfo.id = playerinfo.id;
					packetinfo.size = sizeof(player_info);
					/// SOCKETINFO 할당, 초기화.
					SOCKETINFO *p = new SOCKETINFO;
					memcpy(p, &(g_clients[recvid].m_sockinfo), sizeof(*p));
					ZeroMemory(&p->overlapped, sizeof(p->overlapped));
					p->recvbytes = p->sendbytes = 0;
					memcpy(p->wsabuf.buf, &(packetinfo), sizeof(packetinfo));
					p->wsabuf.len = BUFSIZE;

					// 2. 가변 길이
					memcpy(p->wsabuf.buf + sizeof(packetinfo), &(playerinfo), sizeof(playerinfo));
					p->wsabuf.len = BUFSIZE;
					// 새 클라이언트에게 다른 클라이언트의 정보 리턴.
					if (WSASend(p->sock, &(p->wsabuf), 1, &sendBytes, 0, &(p->overlapped), PutPlayerPos) == SOCKET_ERROR) {
						if (WSAGetLastError() != WSA_IO_PENDING) {
							printf("Error - F ail WSASend(error_code : %d)\n", WSAGetLastError());
						}
					}

				}
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