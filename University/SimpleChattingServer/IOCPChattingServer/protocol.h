#pragma once


const auto SERVERIP = "127.0.0.1";
const auto SERVERPORT = 9000;

const auto MAX_USER = 3000;
const auto MAX_BUFF_SIZE = 4000;
const auto MAX_STR_SIZE = 100;

const auto OPERATION_RECV = 0;
const auto OPERATION_SEND = 1;


//

const auto CLIENT_EVENT_CHAT = 0;

struct sc_packet_player_chat {
	unsigned char size;
	unsigned char type;
};