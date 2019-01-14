#include <winsock2.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER 1024
#define SERVER_PORT 3500

// 클라이언트와 연결을 관리할 때 필요한 데이터.
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;	 // overlapped IO 구조체
	WSABUF dataBuffer; // 버퍼 자체가 아닌 버퍼의 포인터
	SOCKET socket;
	char messageBuffer[MAX_BUFFER]; // 실제 버퍼가 들어가는 메시지 버퍼
	int receiveBytes; // 얼마 받았고
	int sendBytes; // 얼마 보냈는지
};

// overlapped : send, recv 값이 call by reference로 날라옴.
void CALLBACK callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD InFlags);

int main()
{
	WSADATA WSAData;

	// 0. Winsock Start
	{
		if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
			cout << "Error - Can not load 'winsock.dll' file" << endl;
			return 1;
		}
	}

	// 1. 사용할 소켓 생성
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	{
		if (listenSocket == INVALID_SOCKET) {
			cout << "Error - Invalid socket" << endl;
			return 1;
		}
	}

	/// 서버 정보 객체 설정
	SOCKADDR_IN serverAddr;
	{
		memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
		serverAddr.sin_family = PF_INET;
		serverAddr.sin_port = htons(SERVER_PORT);
		serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	}

	// 2. 소켓 설정
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		// 6. 소켓 종료
		cout << "Error - Fail bind" << endl;
		closesocket(listenSocket);
		// winsock end
		WSACleanup();
		return 1;
	}

	// 3. 수신 대기열 생성
	// 동접 2까지
	if (listen(listenSocket, 2) == SOCKET_ERROR) {
		// 6. 소켓 종료
		cout << "Error - Fail listen" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	/// 필요한 변수들
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	SOCKETINFO* socketInfo;
	DWORD receiveBytes;
	DWORD flags;

	// 루프를 통해서 계속 accept - 소켓 만들고 만들고..
	while (true)
	{
		// 소켓과 주소 구조체 넣기.
		/// accept하면 클라이언트와 연결 되고, 클라이언트의 주소 구조체 변수를 받아와서 주소 정보 얻어옴.
		clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Error - Accept Failure" << endl;
			return 1;
		}

		// 클라이언트의 정보를 만들어 와야 한다.
		socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
		ZeroMemory(socketInfo, sizeof(struct SOCKETINFO));
		socketInfo->socket = clientSocket;
		socketInfo->dataBuffer.len = MAX_BUFFER;
		socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
		flags = 0;

		// 넘겨 주고, 받아서 쓴다.
		socketInfo->overlapped.hEvent = (HANDLE)socketInfo->socket;
		
		// 중첩 소켓을 지정하고, 완료시 실행될 함수를 넘겨 준다.
		// recv 하고 받아온 데이터를 처리해야 하는데 바로 끝난다? -> overlapped IO 라서.
			// 실제 데이터가 버퍼에 들어오는 건 나중에.
		if (WSARecv(socketInfo->socket, &socketInfo->dataBuffer, 1, &receiveBytes, &flags,
			&(socketInfo->overlapped), callback)) {

			// overlapped IO는 pending 에러가 나와야 한다.
			// 위에서 데이터를 이미 읽어버린 것임.
			if (WSAGetLastError() != WSA_IO_PENDING) {
				cout << "Error - IO Pending Failure" << endl;
				return 1;
			}
		}
	}

	// 6-2. 리슨 소켓 종료.
	closesocket(listenSocket);

	// Winsock End.
	WSACleanup();

	return 0;
}


void CALLBACK callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD InFlags)
{
	struct SOCKETINFO *socketInfo;
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	// recv 했을 때, 몇 번째 소켓인지 등 정보를 overlapped 구조체를 통해 알아온다.
	/// 변경할 수 없는 overlapped 구조체를 타입 캐스팅 한다.
	/// SOCKETINFO의 구조체의 첫 멤버 변수가 overlapped이기 때문에..! 꼼수임.
	/// callback 함수의 인자로 들어오는 것 중에 정보가 없으므로.
	socketInfo = (struct SOCKETINFO*)overlapped;

	// recv 했을 때, 0 바이트를 읽었다면?
	//  -> 접속된 클라이언트에서 소켓 접속을 끈 것이다. 즉 CloseSocket() 호출 한 것.
	if (dataBytes == 0) {
		// 클라에서 끊었는데 굳이 잡고 있을 필요 없음. 해제해 준다.
		closesocket(socketInfo->socket);
		free(socketInfo);
		return;
	}
	

	if (socketInfo->receiveBytes == 0) {
		// WSARecv(최초 대기에 대한) 의 callback일 경우.
		/// '소켓 정보'에서 받은 데이터가 0의 크기라면 처음 받은 것임. 즉 최초 callback인 경우.
		socketInfo->receiveBytes = dataBytes; // 이만큼 받았으니까 이만큼 받음
		socketInfo->sendBytes = 0; // 아직 하나도 안 보냈음 ㅇㅅㅇ
		socketInfo->dataBuffer.buf = socketInfo->messageBuffer; // ㅇㅅㅇ
		socketInfo->dataBuffer.len = socketInfo->receiveBytes; // 실제로 받은 데이터 만큼만 보내야 한다.

		cout << "TRACE - Receive message : " << socketInfo->messageBuffer <<
			" ( " << dataBytes << "bytes )" << endl;

		// send, recv 하기 전에 무조건 0으로 초기화 해 주어야.
		/// overlapped IO가 종료하고, recv가 종료되었으므로, 또 recv를 하려면 overlapped 구조체를 0으로 미리 초기화.
		/// 나는 memset보다 ZeroMemory가 좋다.
		ZeroMemory(&(socketInfo->overlapped), sizeof(WSAOVERLAPPED));

		// 클라이언트에서 준 것을 그대로 리턴. -> 할 필요 없음.
		if (WSASend(socketInfo->socket, &(socketInfo->dataBuffer), 1, &sendBytes, 0, &(socketInfo->overlapped), callback) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				cout << "Error - Fail WSASend (error_code : " << WSAGetLastError() << ")" << endl;
			}
		}
	}
	else { // send callback
		
		// WSASend(응답에 대한) callback일 경우.
		socketInfo->sendBytes += dataBytes; 
		socketInfo->receiveBytes = 0;
		socketInfo->dataBuffer.len = MAX_BUFFER;
		socketInfo->dataBuffer.buf = socketInfo->messageBuffer;

		cout << "TRACE - Send Message : " << socketInfo->messageBuffer << "( " << dataBytes << "bytes )" << endl;

		// overlapped IO가 종료하고, recv가 종료되었으므로,
		// 또 recv를 하려면 overlapped 구조체를 0으로 미리 초기화 한다.
		ZeroMemory(&(socketInfo->overlapped), 0, sizeof(WSAOVERLAPPED));

		if (WSARecv(socketInfo->socket, &socketInfo->dataBuffer, 1, &receiveBytes,
			&flags, &(socketInfo->overlapped), callback) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				cout << "Error - Fail WSARecv (error_code : " << WSAGetLastError() << ")" << endl;
			}
		}
	}

}