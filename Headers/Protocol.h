#pragma once
constexpr int CS_UP = 1;
constexpr int CS_DOWN = 2;
constexpr int CS_LEFT = 3;
constexpr int CS_RIGHT = 4;

constexpr int SC_LOGIN_OK = 1;
constexpr int SC_PUT_PLAYER = 2;
constexpr int SC_REMOVE_PLAYER = 3;
constexpr int SC_MOVE_PLAYER = 4;

#pragma pack (push, 1)

struct cs_packet_up {
	char size;
	char type;
};

struct cs_packet_down {
	char size;
	char type;
};

struct cs_packet_left {
	char size;
	char type;
};

struct cs_packet_right {
	char size;
	char type;
};



struct sc_packet_login_ok {
	char size;
	char type;
	char id;
};

struct sc_packet_put_player {
	char size;
	char type;
	char id;
	char x, y, z;
};

struct sc_packet_remove_player {
	char size;
	char type;
	char id;
};

struct sc_packet_move_player {
	char size;
	char type;
	char id;
	char x, y, z;
};


//
enum packet_type {
	sc_login_ok, sc_put_player, sc_notify_yourinfo, sc_notify_playerinfo,
	cs_put_player,
	cs_move_left, cs_move_top, cs_move_right, cs_move_bottom, cs_move,
};
enum player_type {
	player_doggy, player_ducky
};
enum networking_state {
	recv_none, recv_playerinfo, recv_otherinfo
};
struct packet_info {
	short size;
	packet_type type;
	WORD id;
	SOCKET sock;
};

struct player_info {
	int id = 10;
	bool connected = false;
	float x;
	float y;
	float z;
	player_type type;
	int animationSet = 0;
	float l_x;
	float l_y;
	float l_z;
	float r_x;
	float r_y;
	float r_z;
	int piggybackstate = 0;
	bool piggyback_master = false;
};

#pragma pack (pop)