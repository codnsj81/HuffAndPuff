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
	cs_put_player,
	sc_put_player,
	cs_move_left, cs_move_top, cs_move_right, cs_move_bottom, cs_move,
	sc_notify_pos, sc_notify_remove_player
};
struct packet_info {
	BYTE size;
	packet_type type;
	WORD id;
};

enum player_type {
	player_ducky, player_doggy
};
struct player_info{
	bool connected = false;
	float x;
	float y;
	float z;
	player_type type;
};

#pragma pack (pop)