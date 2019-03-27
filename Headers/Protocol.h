#pragma pack (push, 1)

struct cs_packet_up {
	BYTE size;
	BYTE type;
};

struct cs_packet_down {
	BYTE size;
	BYTE type;
};

struct cs_packet_left {
	BYTE size;
	BYTE type;
};

struct cs_packet_right {
	BYTE size;
	BYTE type;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE x;
	BYTE y;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE x;
	BYTE y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

//
enum packet_type {
	sc_put_player, sc_notify_yourinfo, sc_notify_playerinfo,
	cs_put_player,
	cs_move_left, cs_move_top, cs_move_right, cs_move_bottom, cs_move,
};
enum player_type {
	player_ducky, player_doggy
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
	short id = 10;
	bool connected = false;
	float x;
	float y;
	float z;
	player_type type;
};

#pragma pack (pop)