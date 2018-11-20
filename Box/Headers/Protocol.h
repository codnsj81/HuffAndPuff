#include <DirectXMath.h>


#define MAX_USER 2
#define BUFSIZE 512
#define SERVERPORT 9000

typedef struct PlayerInfo {
	short id;
	DirectX::XMVECTOR pos;
	bool connected;
}PLAYERINFO;
