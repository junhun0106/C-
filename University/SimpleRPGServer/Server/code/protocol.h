#pragma once


// Client Protocol define //


// 

const auto MAX_USER = 10000;
const auto MAX_BUFF_SIZE = 4000;
const auto MAX_STR_SIZE = 100;
const auto WORLD_WIDTH = 16000; // 2km
const auto WORLD_HEIGHT = 16000; // 2km
const auto MAX_NAME_SIZE = 10;

const auto SERVERPORT = 9000;

// World ID

const auto WORLD_MAP = 0;
const auto DUNGEON = 1;

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

// system type define

const auto STATE_MESAAGE = 10;

// Player type Define

const auto PLAYER_STATE = 100;
const auto PLAYER_POS = 101;
const auto PLAYER_PUT = 102;
const auto PLAYER_POP = 103;
const auto PLAYER_LEVEL = 104;
const auto PLAYER_EXP = 105;
const auto PLAYER_SPEACH = 106;
const auto PLAYER_COLLISION = 107;
const auto PLAYER_HP = 108;
const auto PLAYER_ID = 109;


// Objects type Define

const auto MAX_ENEMY = 5000;

const auto STATIC_OBJECTS = 200;


const auto MONSTER_STATIC_NO_ATTACK = 0;
const auto MONSTER_STATIC_ATTACK = 1;
const auto MONSTER_DYNAMIC_NO_ATTACK = 2;
const auto MONSTER_DYNAMIC_ATTACK = 3;
const auto MONSTER_DYNAMIC_LONG = 4;

const auto DYNAMIC_OBJECTS = 50;
const auto ENEMY_MOVE = 51;
const auto ENEMY_VIEW_STATE = 52;
const auto ENEMY_SPEACH = 53;
const auto ENEMY_REGEN = 54;
const auto ENEMY_HP = 55;
const auto ENEMY_ATTACK_MOVE = 56;

const auto OP_RECV = 1;
const auto OP_SEND = 2;
const auto OP_NPC_MOVE = 3;
const auto OP_EVENT_PLAYER_MOVE = 4;
const auto OP_EVENT_ENEMY_MOVE = 5;
const auto OP_NPC_REGEN = 6;
const auto OP_ENEMY_ATTACK_MOVE = 7;
const auto OP_PLAYER_HP_HEAL = 8;

// struct define

// Server -> Client

struct SC_STATIC_OBJECTS {
	BYTE size;
	BYTE model_name;
	BYTE model_EA;
	BYTE *xPos; // array[model_EA]
	BYTE *yPos; // array[modea_EA]
};
struct SC_Put_Player {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
};
struct SC_POP_PLAYER {
	BYTE size;
	BYTE type;
	WORD id;
};
struct SC_Player_Pos {
	BYTE size;
	BYTE type;
	WORD id;
	float xPos;
	float yPos;
};
struct sc_packet_player_level {
	byte size;
	byte type;
	WORD id;
	WORD level;
};
struct sc_packet_player_exp {
	byte size;
	byte type;
	WORD id;
	WORD exp;
};
struct sc_packet_player_speach {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};
struct sc_packet_player_collision {
	byte size;
	byte type;
	bool check;
};
struct sc_packet_player_hp {
	byte size;
	byte type;
	WORD hp;
};
struct sc_packet_player_id {
	byte size;
	byte type;
	WORD id;
	WCHAR user_id[MAX_STR_SIZE];
};
struct SC_NPC_POS {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
};
struct SC_NPC_VIEW {
	BYTE size;
	BYTE type;
	WORD id;
	bool view_state;
};
struct sc_packet_npc_hp {
	byte size;
	byte type;
	WORD id;
	WORD hp;
};
struct SC_PACKET_NPC_SPEACH {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_state_message {
	byte size;
	byte type;
	WCHAR message[MAX_STR_SIZE];
};

// Client -> Server
struct CS_Player_State {
	BYTE size;
	BYTE type;
};
struct cs_packet_player_speach {
	byte size;
	byte type;
	wchar_t message[MAX_STR_SIZE];
};