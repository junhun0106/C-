#pragma once


// Client Protocol define //


// 

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

// define animate state

const auto ANIMATE_IDLE = 0;
const auto ANIMATE_MOVE = 1;
const auto ANIMATE_SHOT = 2;
const auto ANIMATE_RELOAD = 3;
const auto ANIMATE_DEATH = 4;

const auto MAX_USER = 10;
const auto MAX_BUFF_SIZE = 4000;
const auto MAX_STR_SIZE = 100;

const auto WORLD_WIDTH = 2000; // 2km
const auto WORLD_HEIGHT = 2000; // 2km

const auto SERVERPORT = 9000;

// Player KeyState define

const int IDLE = 0;
const int VK_UPBUTTON = 1;
const int VK_RIGHTBUTTON = 2;
const int VK_LEFTBUTTON = 3;
const int VK_DOWNBUTTON = 4;
const auto VK_MOUSERIGHTBUTTON = 5;
const auto VK_MOUSELEFTBUTOON = 6;
const auto VK_SPACEBAR = 7;


// Type Define //

// Player type Define

const auto PLAYER_EVENT_STATE = 100;
const auto PLAYER_EVENT_POS = 101;
const auto PLAYER_EVENT_PUT = 102;
const auto PLAYER_EVENT_POP = 103;


// Objects type Define

const auto MAX_ENEMY = 4000;
const auto MAX_STEERING = 300;

const auto STATIC_OBJECTS = 200;


const auto DYNAMIC_OBJECTS = 50;
const auto ENEMY_EVENT_MOVE = 51;
const auto ENEMY_EVENT_VIEW_STATE = 52;
const auto ENEMY_EVENT_HEART_BEAT = 53;
const auto ENEMY_EVENT_DEATH = 54;
const auto ENEMY_EVENT_ASTAR = 55;
const auto ENEMY_EVENT_ATTACK = 56;

const auto FLOCK_EVENT_MOVE = 70;
const auto FLOCK_EVENT_VIEW = 71;

const auto SUNSET_EVENT = 80;

const auto OP_RECV = 1;
const auto OP_SEND = 2;
const auto OP_NPC_MOVE = 3;
const auto OP_NPC_HEATBEAT = 4;
const auto OP_NPC_ASTAR = 5;
const auto OP_SUNSET = 6;

const auto PLAYER_PARTY_CHAT_MESSAGE = 83;
// struct define

// Server -> Client

struct sc_packet_static_object {
	BYTE size;
	BYTE model_name;
	BYTE model_EA;
	BYTE *xPos; // array[model_EA]
	BYTE *yPos; // array[modea_EA]
};
struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
	float look_x;
	float look_y;
	float look_z;
	//bool view_state;
};
struct sc_packet_pop_player {
	BYTE size;
	BYTE type;
	WORD id;
	//bool view_state;
};
struct sc_packet_player_position {
	BYTE size;
	BYTE type;
	WORD id;
	float xPos;
	float yPos;
	float zPos;
	float x_lock;
	float y_look;
	float z_lock;
};
struct sc_packet_player_anim {
	BYTE size;
	BYTE type;
	WORD id;
	WORD anim_state;
};
struct sc_packet_npc_position {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
	float look_x;
	float look_y;
	float look_z;
};
struct sc_packet_npc_view {
	BYTE size;
	BYTE type;
	WORD id;
	bool view_state;
};
struct sc_packet_steer_position {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
	float look_x;
	float look_y;
	float look_z;
};
struct sc_packet_steer_view {
	BYTE size;
	BYTE type;
	WORD id;
	bool state;
};

// Client -> Server
struct cs_packet_player_state {
	BYTE size;
	BYTE type;
	DWORD direction;
	float look_x;
	float look_y;
	float look_z;
	float right_x;
	float right_y;
	float right_z;
};
struct cs_packet_charater_animate {
	BYTE size;
	BYTE type;
	WORD anim_state;
};
struct cs_packet_npc_state {
	BYTE size;
	BYTE type;
	WORD id;
	bool state;
};




////////////// 수정. 8월 15일

const auto PLAYER_EVENT_LEVEL_UP = 110;
const auto PLAYER_EVENT_GET_EXP = 105;
const auto PLAYER_EVENT_DECREASE_HP = 106;
const auto PLAYER_EVENT_DEATH = 107;

const auto PLAYER_EVENT_ATTACK = 108;

const auto PLAYER_EVENT_CHAT_MESSAGE = 109;
const auto PLAYER_ANIMATE_STATE = 104;

const auto ENEMY_EVENT_HP = 57;

struct sc_packet_player_level {
	BYTE size;
	BYTE type;
	WORD level;
	unsigned short id;
};
struct sc_packet_player_exp {
	BYTE size;
	BYTE type;
	WORD exp;
	unsigned short id;
};
struct sc_packet_player_hp {
	BYTE size;
	BYTE type;
	WORD hp;
	unsigned short id;
};
struct sc_packet_player_death {
	BYTE size;
	BYTE type;
	WORD id;
	bool state;
};

struct sc_packet_enemy_hp {
	BYTE size;
	BYTE type;
	unsigned short id;
	unsigned short hp;
};
struct sc_packet_enemy_animate {
	BYTE size;
	BYTE type;
	unsigned short id;
	unsigned short anim;
};

struct cs_packet_chat_message {
	BYTE size;
	BYTE type;
	unsigned short id;
	wchar_t mesaage[MAX_STR_SIZE];
};
struct cs_packet_player_attack {
	BYTE size;
	BYTE type;
	unsigned short id;
};
struct sc_packet_chat_message {
	BYTE size;
	BYTE type;
	wchar_t message[MAX_STR_SIZE];
};

struct sc_packet_sunset_position {
	BYTE size;
	BYTE type;
	float x;
	float y;
	float z;
};



















//ksh7
struct cs_packet_db {	// 누가 누구한테, 초대 걸었는지  // 초대 승락을 할때, 누구+누구 승락되는지.
	BYTE size;
	BYTE type;
	WORD xPos;
	WORD yPos;
	WORD plevel;
	WORD pexp;
	WORD php;
	bool pconnect;
	WORD pclass;
	WORD pmodel;
	bool pselect;
	WCHAR db_name[50];
};

struct cs_packet_party {	// 누가 누구한테, 초대 걸었는지  // 초대 승락을 할때, 누구+누구 승락되는지.
	BYTE size;
	BYTE type;
	BYTE bool_invite;
	BYTE bool_join;
	WCHAR invite_name[50];
	WCHAR join_name[50];
};

struct sc_packet_party_player {		// 누구한테, 수락하는지
	BYTE size;
	BYTE type;
	BYTE bool_invite;
	BYTE bool_party_success;
	WCHAR invite_name[50];
	WCHAR join_name[50];
};

struct sc_packet_party_info {		// 누구한테, 수락하는지
	BYTE size;
	BYTE type;
	BYTE xPos;
	BYTE yPos;
	BYTE plevel;
	BYTE hp;
	WCHAR member_name[50];
};

struct sc_packet_party_set {
	BYTE size;
	BYTE type;
	BYTE m_count;
	BYTE level;
	BYTE xPos;
	BYTE yPos;
	BYTE hp;
	WCHAR member_name[50];
};

//ksh7