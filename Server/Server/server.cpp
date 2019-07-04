#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../../Headers/Include.h"
#include <map>
#pragma comment(lib, "ws2_32")

using namespace std;


// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped; // overlapped 구조체
	WSABUF wsabuf;	// WSABUF 구조체. 버퍼 자체가 아닌, 버퍼의 포인터
	SOCKET sock;
	char buf[BUFSIZE + 1];	// 응용 프로그램 버퍼. 실제 버퍼가 들어갈 메시지 버퍼.
	player_info playerinfo = {};
	bool connected = false;
	bool is_recv = true;
};

// map<SOCKET, SOCKETINFO> clients;
SOCKETINFO clients[NUM_OF_PLAYER];

// 비동기 입출력 시작과 처리 함수
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_csputplayer_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

// 함수들
void send_packet(char, char*);
void do_recv(char id);

void err_quit(const char *msg);
void err_display(const char *msg);
void show_allplayer();



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
		cout << "[클라이언트 접속] IP 주소 (" << inet_ntoa(client_addr.sin_addr) << "), 포트 번호 (" << ntohs(client_addr.sin_port) << ")" << endl;
		// 아이디 부여.
		int new_id = -1;
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (false == clients[i].connected) {
				clients[i].connected = true;
				new_id = i;
				break;
			}
		}
		// 소켓의 옵션을 변경.
		bool NoDelay = TRUE;
		setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (const char FAR*)&NoDelay, sizeof(NoDelay));

		// 클라이언트 구조체의 정보 입력.
		{
			clients[new_id] = SOCKETINFO{};
			memset(&clients[new_id], 0x00, sizeof(SOCKETINFO));
			clients[new_id].playerinfo.id = new_id;
			clients[new_id].playerinfo.connected = true;
			clients[new_id].sock = client_sock;
			clients[new_id].playerinfo.x = INITPOSITION_X;
			clients[new_id].playerinfo.z = INITPOSITION_Z;
			// clients[new_id].playerinfo.type = player_type(new_id % 2);
			clients[new_id].playerinfo.animationSet = 0;
			clients[new_id].wsabuf.len = BUFSIZE;
			clients[new_id].wsabuf.buf = clients[new_id].buf;
			clients[new_id].overlapped.hEvent = (HANDLE)clients[new_id].sock;
			clients[new_id].connected = true;
			flags = 0;
		}
		


		// 로그인 되었다는 패킷을 보낸다.
		{
			char buf[BUFSIZE];
			// (고정)
			packet_info packetinfo;
			packetinfo.id = new_id;
			packetinfo.size = sizeof(player_info);
			packetinfo.type = sc_notify_yourinfo;
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			// (가변)
			memcpy(buf + sizeof(packetinfo), &(clients[new_id].playerinfo), sizeof(player_info));
			// 전송
			send_packet(new_id, buf);
		}

		// new_id의 접속을 다른 클라이언트에게 알린다.
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (!clients[i].connected) continue;  // 연결된 클라이언트에게만 보낸다.
			if (i == new_id) continue;
			char buf[BUFSIZE];
			// (고정)
			packet_info packetinfo;
			packetinfo.id = new_id;
			packetinfo.size = sizeof(player_info);
			packetinfo.type = sc_put_player;
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			// (가변)
			memcpy(buf + sizeof(packetinfo), &(clients[new_id].playerinfo), sizeof(player_info));
			// 전송
			send_packet(new_id, buf);

		}

		// 다른 클라이언트의 존재를 new_id 클라이언트에게 알린다.
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (false == clients[i].connected) continue;
			if (i == new_id) continue; // 나에게는 보내지 않는다.

			char buf[BUFSIZE];
			// (고정)
			packet_info packetinfo;
			packetinfo.id = i;
			packetinfo.size = sizeof(player_info);
			packetinfo.type = sc_put_player;
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			// (가변)
			memcpy(buf + sizeof(packetinfo), &(clients[i].playerinfo), sizeof(player_info));
			// 전송
			send_packet(new_id, buf);
		}

		do_recv(new_id);
		show_allplayer();
	}

	// closesocket()
	closesocket(listen_sock);
	// 윈속 종료
	WSACleanup();

	return 0;
}


void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	// 오버랩드에서 소켓을 읽어온다.
	SOCKETINFO *socketInfo = (struct SOCKETINFO *)overlapped;
	SOCKET client_sock = reinterpret_cast<int>(overlapped->hEvent);

	int id = socketInfo->playerinfo.id;

	if (id > NUM_OF_PLAYER)
		return;

	if (dataBytes == 0) {
		// 클라이언트 접속 종료.
		closesocket(clients[id].sock);
		clients[id].connected = false;
		return;
	}


	// 고정 길이 패킷.
	player_info playerinfo;
	packet_info packetinfo;
	memcpy(&packetinfo, clients[id].buf, sizeof(packetinfo));
	int fromid = packetinfo.id;


	// 가변 길이 패킷.
	switch (packetinfo.type) {
	case cs_move:
	{
		// 1. cs_move 패킷을 보내온 클라이언트의 정보를 받아온다.
		{
			char buf[BUFSIZE];
			memcpy(&playerinfo, clients[fromid].buf + sizeof(packetinfo), sizeof(player_info));
			clients[fromid].playerinfo = playerinfo;
		}

		// 2. 다른 접속 중인 클라이언트에게 움직인 클라이언트의 정보를 보낸다.
		{
			char buf[BUFSIZE];
			// (고정)
			memset(&packetinfo, 0x00, sizeof(packetinfo));
			packetinfo.id = fromid;
			packetinfo.size = sizeof(playerinfo);
			packetinfo.type = sc_notify_playerinfo;
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			// (가변)
			memcpy(buf + sizeof(packetinfo), &(clients[fromid].playerinfo), sizeof(player_info));
			// 전송
			for (int i = 0; i < NUM_OF_PLAYER; ++i) {
				if (!clients[i].connected) continue;
				if (i == fromid) continue;
				send_packet(i, buf);
			}

		}

		// 3. 보내온 클라이언트의 소켓은 다시 Recv를 시작한다.
		{
			do_recv(fromid);
		}

		// 4. 현재 접속 중인 클라이언트 플레이어 정보 출력.
		show_allplayer();
	}
	break;
	case cs_put_playertype:
	{
		// 1. cs_put_playertype 패킷을 보내온 클라이언트의 "type" 정보를 받아온다.
		{
			char buf[BUFSIZE];
			memcpy(&playerinfo, clients[fromid].buf + sizeof(packetinfo), sizeof(player_info));
			clients[fromid].playerinfo.type = playerinfo.type;
		}
		// 2. 보내온 클라이언트의 소켓은 다시 Recv를 시작한다.
		{
			do_recv(fromid);
		}
	}
	break;
	case cs_snake_is_dead:
	{
		// 1. 죽은 몬스터의 id를 받아온다.
		int monsterId = -1;
		char buf[BUFSIZE];
		memcpy(&monsterId, clients[fromid].buf + sizeof(packetinfo), sizeof(int));
		
		// 2. 다른 클라이언트에게도 죽은 몬스터의 id를 알린다.
		memset(&packetinfo, 0x00, sizeof(packetinfo));
		packetinfo.id = fromid;
		packetinfo.size = sizeof(int);
		packetinfo.type = sc_snake_is_dead;
		memcpy(buf, &packetinfo, sizeof(packetinfo));
		memcpy(buf + sizeof(packetinfo), &(monsterId), sizeof(int));
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (!clients[i].connected) continue;
			if (i == fromid) continue;
			send_packet(i, buf);
		}

		// 3. 보내온 클라이언트 소켓은 다시 recv를 시작한다.
		do_recv(fromid);

	}
	break;
	case cs_notify_snakeinfo:
	{
		// 1. 죽은 몬스터의 info를 받아온다.
		snake_info s_info;
		char buf[BUFSIZE];
		memcpy(&s_info, clients[fromid].buf + sizeof(packetinfo), sizeof(snake_info));

		// 2. 다른 클라이언트에게도 몬스터의 info를 알린다.
		memset(&packetinfo, 0x00, sizeof(packetinfo));
		packetinfo.id = fromid;
		packetinfo.size = sizeof(snake_info);
		packetinfo.type = sc_notify_snakeinfo;
		memcpy(buf, &packetinfo, sizeof(packetinfo));
		memcpy(buf + sizeof(packetinfo), &(s_info), sizeof(snake_info));
		for (int i = 0; i < NUM_OF_PLAYER; ++i) {
			if (!clients[i].connected) continue;
			if (i == fromid) continue;
			send_packet(i, buf);
		}

		// 3. 보내온 클라이언트 소켓은 다시 recv를 시작한다.
		do_recv(fromid);

	}
	break;
	default:
		do_recv(fromid);
		break;
	}


}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKETINFO *socketInfo = (struct SOCKETINFO *)overlapped;
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);
	int id = socketInfo->playerinfo.id;

	delete socketInfo;

	if (dataBytes == 0) {
		cout << id << "번째 소켓을 닫습니다." << endl;
		closesocket(clients[id].sock);
		clients[id].connected = false;
		return;
	}

}

void CALLBACK send_csputplayer_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
}

void disconnect_client(char id) {

}

void send_packet(char client, char* buf)
{
	SOCKETINFO* socketinfo = new SOCKETINFO;
	socketinfo->playerinfo = clients[client].playerinfo;
	socketinfo->wsabuf.len = BUFSIZE;
	socketinfo->wsabuf.buf = socketinfo->buf;
	memcpy(socketinfo->buf, buf, sizeof(socketinfo->buf));
	ZeroMemory(&(socketinfo->overlapped), sizeof(socketinfo->overlapped));
	DWORD recvBytes = 0;

	if (WSASend(clients[client].sock, &(socketinfo->wsabuf), 1, &recvBytes, 0, &(socketinfo->overlapped),
		send_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
		}
	}
}




void do_recv(char id)
{
	DWORD flags = 0;

 	clients[id].is_recv = true;
	if (WSARecv(clients[id].sock, &clients[id].wsabuf, 1,
		NULL, &flags, &(clients[id].overlapped), recv_callback))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - IO pending Failure\n";
			while (true);
		}
	}
	//else {
	//	cout << "Non Overlapped Recv return.\n";
	//	while (true);
	//}
}


void show_allplayer()
{
	cout << endl << "------------ 접속 플레이어 정보 --------------" << endl;
	int count = 0;

	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		if (clients[i].connected == true) {
			count++;
			cout << clients[i].playerinfo.id << "번째 클라이언트 : "
				<< "좌표( "
				<< clients[i].playerinfo.x << ", " << clients[i].playerinfo.y << ", " << clients[i].playerinfo.z << ")"
				// << ", 애니메이션셋(" << clients[i].playerinfo.animationSet << ")"
				// << ", xmf3Look(" << clients[i].playerinfo.l_x << ", " << clients[i].playerinfo.l_y << ", " << clients[i].playerinfo.l_z << ")"
				// << ", xmf3Right(" << clients[i].playerinfo.r_x << ", " << clients[i].playerinfo.r_y << ", " << clients[i].playerinfo.r_z << ")"
				// << ", piggybackstate(" << clients[i].playerinfo.piggybackstate << ")"
				<< ", playertype(" << clients[i].playerinfo.type << ")"
				<< endl;
		}
	}
	cout << "현재 연결 된 클라이언트 수 : " << count << endl;
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

