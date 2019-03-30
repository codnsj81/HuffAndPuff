#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../../Headers/Include.h"
#include <map>
using namespace std;
#pragma comment(lib, "ws2_32")


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
	WSABUF wsabuf;	// WSABUF 구조체. 버퍼 자체가 아닌, 버퍼의 포인터
	SOCKET sock;
	char buf[BUFSIZE + 1];	// 응용 프로그램 버퍼. 실제 버퍼가 들어갈 메시지 버퍼.
	int recvbytes;	// 송, 수신 바이트 수
	int sendbytes;
	player_info playerinfo;
};

bool g_connected[2];
map<SOCKET, SOCKETINFO> clients;

// SOCKET client_sock; // accept 리턴 값, 두 스레드가 접근하기 때문에 전역으로.
HANDLE hReadEvent, hWriteEvent; // client_sock 보호하기 위한.

// 비동기 입출력 시작과 처리 함수
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_nonrecv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_scputplayer_1_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_yesrecv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

// 함수들
void show_allplayer();

// 오류 출력 함수
void err_quit(const char *msg);
void err_display(const char *msg);

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
	if (listen_sock == INVALID_SOCKET)
		err_quit("[오류] Invalid socket");

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
	retval = listen(listen_sock, NUM_OF_PLAYER);
	if (retval == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		err_quit("[오류] listen()");
	}


	SOCKADDR_IN client_addr;
	int addr_length = sizeof(SOCKADDR_IN);
	memset(&client_addr, 0, addr_length);
	SOCKET client_sock;
	DWORD flags;

	int id{ -1 };
	while (1) {
		// accept 
		client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &addr_length);
		if (client_sock == INVALID_SOCKET) {
			err_display("[오류] accept()");
			return 1;
		}
		// 접속한 클라이언트 정보 출력
		cout << "[클라이언트 접속] IP 주소 (" << inet_ntoa(client_addr.sin_addr) <<
			"), 포트 번호 (" << ntohs(client_addr.sin_port) << ")" << endl;
		//WSARecv 준비.
		clients[client_sock] = SOCKETINFO{};
		memset(&clients[client_sock], 0x00, sizeof(struct SOCKETINFO));
		clients[client_sock].sock = client_sock;
		clients[client_sock].wsabuf.len = BUFSIZE;
		clients[client_sock].wsabuf.buf = clients[client_sock].buf;
		flags = 0;
		// 중첩 소켓을 지정.
		clients[client_sock].overlapped.hEvent = (HANDLE)clients[client_sock].sock;

		if (WSARecv(clients[client_sock].sock, &clients[client_sock].wsabuf,
			1, NULL, &flags, &(clients[client_sock].overlapped), recv_callback)) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				err_display("IO pending Failure");
				return 0;
			}
		}
		else {
			cout << ("Non overlapped recv return") << endl;
		}

		cout << "현재 연결 된 클라이언트 수 : " << clients.size() << endl;

	}

	// closesocket()
	closesocket(listen_sock);
	// 윈속 종료
	WSACleanup();

}


// 비동기 입출력 처리 함수(입출력 완료 루틴)
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	// 오버랩드에서 소켓을 읽어온다.
	SOCKET client_sock = reinterpret_cast<int>(overlapped->hEvent);

	// 다시 recv 시작해야 함.
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;

	if (dataBytes == 0) {
		// 클라이언트 접속 종료.
		closesocket(clients[client_sock].sock);
		clients.erase(client_sock);
		return;
	}

	packet_info packetinfo;
	player_info playerinfo;

	// 고정 길이.
	memcpy(&packetinfo, clients[client_sock].buf, sizeof(packetinfo));
	// 가변 길이.
	switch (packetinfo.type) {
	case cs_put_player:
	{
		// 클라이언트가 초기에 설정하여 send 한 playerinfo를 받아온다.
		memcpy(&playerinfo, clients[client_sock].buf + sizeof(packetinfo),
			sizeof(playerinfo));
		// id를 부여하고, clients[client_sock]에 playerinfo를 갱신한다.
		{
			int id = -1;
			for (int i = 0; i < NUM_OF_PLAYER; ++i) {
				if (false == g_connected[i]) {
					g_connected[i] = true;
					id = i;
					break;
				}
			}
			playerinfo.id = id; // 설정한 id를 playerinfo에 채우고.
			clients[client_sock].playerinfo = playerinfo; // clients[client_sock]에 playerinfo를 넣는다.
		}
		// 해당 클라이언트에게 id가 설정된 자신의 playerinfo를 send한다.
		{
			// 보낼 버퍼 준비. 고정 길이 + 가변 길이 데이터 이어 붙임.
			char buf[BUFSIZE];
			memset(&(packetinfo), 0x00, sizeof(packet_info));
			// packetinfo 채워줌.
			packetinfo.id = playerinfo.id;
			packetinfo.size = sizeof(packet_info);
			packetinfo.type = sc_notify_yourinfo;
			// buf에 packetinfo + playerinfo 복사하고.
			memcpy(buf, &packetinfo, sizeof(packet_info));
			memcpy(buf + sizeof(packet_info), &playerinfo, sizeof(player_info));
			// buf를 messageBuffer에 복사하고.
			memcpy(clients[client_sock].buf, buf, sizeof(clients[client_sock].buf));
			// 길이는 고정+가변 크기.
			clients[client_sock].wsabuf.len = sizeof(packetinfo) + sizeof(playerinfo);
			// 오버랩드 구조체 재사용. 초기화.
			memset(&(clients[client_sock].overlapped), 0x00, sizeof(WSAOVERLAPPED));
			clients[client_sock].overlapped.hEvent = (HANDLE)client_sock;
			if (WSASend(client_sock, &(clients[client_sock].wsabuf), 1, &dataBytes, 0, &(clients[client_sock].overlapped),
				send_scputplayer_1_callback) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
				}
			}
		}

		// 다른 클라이언트가 있다면, 이 클라이언트의 접속 정보와 + playerinfo를 send 한다.
		{
			// 보낼 버퍼 준비. 고정 길이 + 가변 길이 데이터 이어 붙임.
			char buf[BUFSIZE];
			memset(&(packetinfo), 0x00, sizeof(packet_info));
			// packetinfo 채워줌.
			packetinfo.id = playerinfo.id;
			packetinfo.size = sizeof(packet_info);
			packetinfo.type = sc_put_player;
			// buf에 packetinfo + playerinfo 복사하고.
			memcpy(buf, &packetinfo, sizeof(packet_info));
			memcpy(buf + sizeof(packet_info), &playerinfo, sizeof(player_info));
			// map 컨테이너를 돌며 접속 중인 클라이언트에게 모두 돌린다.
			for (map<SOCKET, SOCKETINFO>::iterator i_b = clients.begin(); i_b != clients.end(); i_b++)
			{
				SOCKET other_sock = i_b->second.sock;
				if (other_sock == client_sock) // 나에게는 다시 안 보내도 됨.
					continue;

				// -------------------------------------------------
				// send, recv 중일지도 모르는 other_sock의 overlappd 구조체와 wsabuf를 건드려서는 안 된다.
				// -> 새로 할당한다.

				//WSASend 준비.
				// 방법1)
				WSAOVERLAPPED* pOver = new WSAOVERLAPPED;
				pOver->hEvent = (HANDLE)other_sock;
				WSABUF* pWsabuf = new WSABUF;
				char* pBuf = new char[BUFSIZE];
				pWsabuf->len = BUFSIZE;
				pWsabuf->buf = pBuf;
				memcpy(pBuf, buf, BUFSIZE);
				DWORD recvBytes = 0;
				if (WSASend(other_sock, pWsabuf, 1, &recvBytes, 0, pOver,
					send_callback) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
					}
				}
	
				////방법2)
				//player_info other_playerinfo = clients[other_sock].playerinfo;
				//clients[other_sock] = SOCKETINFO{};
				//memset(&clients[other_sock], 0x00, sizeof(struct SOCKETINFO));
				//clients[other_sock].playerinfo = other_playerinfo;
				//clients[other_sock].sock = other_sock;
				//clients[other_sock].wsabuf.len = BUFSIZE;
				//clients[other_sock].wsabuf.buf = clients[other_sock].buf;
				//int other_flags = 0;
				//clients[other_sock].overlapped.hEvent = (HANDLE)clients[other_sock].sock;
				// memcpy(clients[other_sock].buf, buf, sizeof(clients[other_sock].buf));
				//// 오버랩드 구조체 재사용. 초기화. 
				//memset(&(clients[other_sock].overlapped), 0x00, sizeof(WSAOVERLAPPED));
				//clients[other_sock].overlapped.hEvent = (HANDLE)(i_b->second.sock);
				//if (WSASend(other_sock, pWsabuf, 1, &dataBytes, 0, pOver,
				//	send_callback) == SOCKET_ERROR)
				//{
				//	if (WSAGetLastError() != WSA_IO_PENDING)
				//	{
				//		printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
				//	}
				//}
			}
		}
	}
	break;
	case cs_move:
	{
		// 움직인 클라이언트의 정보를 받아온다.
		{
			char buf[BUFSIZE];
			memcpy(&playerinfo, clients[client_sock].buf + sizeof(packetinfo), sizeof(player_info));
			clients[client_sock].playerinfo = playerinfo;
		}
		// 다른 접속 중인 클라이언트에게 움직인 클라이언트의 정보를 보낸다.
		{
			char buf[BUFSIZE];
			playerinfo.id = packetinfo.id; // 움직인 클라이언트의 아이디.
			// 해당 클라이언트에게 새로 갱신된 정보 전송. (고정)
			memset(&packetinfo, 0x00, sizeof(packetinfo));
			packetinfo.id = playerinfo.id;
			packetinfo.size = sizeof(playerinfo);
			packetinfo.type = sc_notify_playerinfo;
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			// (가변)
			memcpy(buf + sizeof(packetinfo), &(clients[client_sock].playerinfo), sizeof(player_info));

			if (clients.size() > 1) {

				for (map<SOCKET, SOCKETINFO>::iterator i_b = clients.begin(); i_b != clients.end(); i_b++)
				{
					SOCKET other_sock = i_b->second.sock;
					if (other_sock == client_sock) // 나에게는 다시 안 보내도 됨.
						continue;
					//WSASend 준비.
					// 방법1)
					WSAOVERLAPPED* pOver = new WSAOVERLAPPED;
					ZeroMemory(pOver, sizeof(WSAOVERLAPPED));
					pOver->hEvent = (HANDLE)other_sock;
					WSABUF* pWsabuf = new WSABUF;
					char* pBuf = new char[BUFSIZE];
					pWsabuf->len = BUFSIZE;
					pWsabuf->buf = pBuf;
					memcpy(pBuf, buf, BUFSIZE);
					DWORD recvBytes = 0;
					if (WSASend(other_sock, pWsabuf, 1, &recvBytes, 0, pOver,
						send_callback) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSA_IO_PENDING)
						{
							printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
						}
					}
					// 방법2)
					//player_info other_playerinfo = clients[other_sock].playerinfo;
					//clients[other_sock] = SOCKETINFO{};
					//memset(&clients[other_sock], 0x00, sizeof(struct SOCKETINFO));
					//clients[other_sock].playerinfo = other_playerinfo;
					//clients[other_sock].sock = other_sock;
					//clients[other_sock].wsabuf.len = BUFSIZE;
					//clients[other_sock].wsabuf.buf = clients[other_sock].buf;
					//int other_flags = 0;
					//clients[other_sock].overlapped.hEvent = (HANDLE)clients[other_sock].sock;
					//memcpy(clients[other_sock].buf, buf, sizeof(clients[other_sock].buf));
					//clients[other_sock].wsabuf.len = sizeof(packetinfo) + sizeof(playerinfo);
					//// 오버랩드 구조체 재사용. 초기화. 
					//memset(&(clients[other_sock].overlapped), 0x00, sizeof(WSAOVERLAPPED));
					//clients[other_sock].overlapped.hEvent = (HANDLE)(i_b->second.sock);
					//if (WSASend(other_sock, pWsabuf, 1, &dataBytes, 0, pOver,
					//	send_callback) == SOCKET_ERROR)
					//{
					//	if (WSAGetLastError() != WSA_IO_PENDING)
					//	{
					//		printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
					//	}
					//}
				}
			}
			else
			{
				DWORD flags = 0;
				// WSASend(응답에 대한)의 콜백일 경우
				clients[client_sock].wsabuf.len = BUFSIZE;
				clients[client_sock].wsabuf.buf = clients[client_sock].buf;

				memset(&(clients[client_sock].overlapped), 0x00, sizeof(WSAOVERLAPPED));
				clients[client_sock].overlapped.hEvent = (HANDLE)client_sock;


				if (WSARecv(client_sock, &clients[client_sock].wsabuf, 1,
					NULL/* 오버랩드: 우리 null 해 주기로 했잖아,, */, &flags, &(clients[client_sock].overlapped), recv_callback) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
					}
				}
			}
		}
	}
	break;
	}
	show_allplayer();
}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);


	if (dataBytes == 0)
	{
		closesocket(clients[client_s].sock);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우

		// WSASend(응답에 대한)의 콜백일 경우
	clients[client_s].wsabuf.len = BUFSIZE;
	clients[client_s].wsabuf.buf = clients[client_s].buf;

	memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].overlapped.hEvent = (HANDLE)client_s;


	if (WSARecv(client_s, &clients[client_s].wsabuf, 1,
		NULL/* 오버랩드: 우리 null 해 주기로 했잖아,, */, &flags, &(clients[client_s].overlapped), recv_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
		}
	}

}


void CALLBACK send_scputplayer_1_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].sock);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우

	// 3번 단계 실행 or WSARecv 실행
	// 해당 클라이언트에게 다른 클라이언트의 playerinfo를 send 한다.
	int count = clients.size();
	if (clients.size() > 1) {
		for (map<SOCKET, SOCKETINFO>::iterator i_b = clients.begin(); i_b != clients.end(); i_b++) {
			if (clients[client_s].playerinfo.id == i_b->second.playerinfo.id)
				continue;
			char buf[BUFSIZE];
			// 고정
			packet_info other_packetinfo;
			other_packetinfo.id = i_b->second.playerinfo.id;
			other_packetinfo.size = sizeof(i_b->second.playerinfo);
			other_packetinfo.sock = i_b->second.sock;
			other_packetinfo.type = sc_put_player;
			// 가변
			player_info other_playerinfo = i_b->second.playerinfo;
			// buf에 packetinfo + playerinfo 복사하고.
			memcpy(buf, &other_packetinfo, sizeof(packet_info));
			memcpy(buf + sizeof(other_packetinfo), &other_playerinfo, sizeof(player_info));
			// buf를 messageBuffer에 복사하고.
			memcpy(clients[client_s].buf, buf, sizeof(clients[client_s].buf));
			// 길이는 고정+가변 크기.
			clients[client_s].wsabuf.len = sizeof(packet_info) + sizeof(player_info);
			// 오버랩드 구조체 재사용. 초기화.
			memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
			clients[client_s].overlapped.hEvent = (HANDLE)client_s;

			if (count > 2) {
				if (WSASend(client_s, &(clients[client_s].wsabuf), 1, &dataBytes, 0, &(clients[client_s].overlapped),
					send_nonrecv_callback) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
					}
				}
				count--;
			}
			else {
				if (WSASend(client_s, &(clients[client_s].wsabuf), 1, &dataBytes, 0, &(clients[client_s].overlapped),
					send_yesrecv_callback) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
					}
				}
			}
		}
	}
	else {
		clients[client_s].wsabuf.len = BUFSIZE;
		clients[client_s].wsabuf.buf = clients[client_s].buf;

		memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
		clients[client_s].overlapped.hEvent = (HANDLE)client_s;

		if (WSARecv(client_s, &clients[client_s].wsabuf, 1,
			NULL/* 오버랩드: 우리 null 해 주기로 했잖아,, */, &flags, &(clients[client_s].overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
			}
		}
	}

}
void CALLBACK send_yesrecv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].sock);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우


	// WSASend(응답에 대한)의 콜백일 경우
	clients[client_s].wsabuf.len = BUFSIZE;
	clients[client_s].wsabuf.buf = clients[client_s].buf;

	memset(&(clients[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].overlapped.hEvent = (HANDLE)client_s;

	if (WSARecv(client_s, &clients[client_s].wsabuf, 1,
		NULL/* 오버랩드: 우리 null 해 주기로 했잖아,, */, &flags, &(clients[client_s].overlapped), recv_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
		}
	}
}

void CALLBACK send_nonrecv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

}

void show_allplayer()
{
	cout << endl << "------------ 접속 플레이어 정보 --------------" << endl;
	cout << "현재 연결 된 클라이언트 수 : " << clients.size() << endl;
	map<SOCKET, SOCKETINFO>::iterator i_b = clients.begin();
	map<SOCKET, SOCKETINFO>::iterator i_e = clients.end();
	for (map<SOCKET, SOCKETINFO>::iterator i_b = clients.begin();
		i_b != clients.end(); i_b++) {
		cout << i_b->second.playerinfo.id << "번째 클라이언트 : "
			<< "좌표( "
			<< i_b->second.playerinfo.x << ", " << i_b->second.playerinfo.y << ", " << i_b->second.playerinfo.z << ")" << endl;
	}
	cout << "------------------------------------------------" << endl;
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
