#include "stdafx.h"
#include "ConnectServer.h"

CConnectServer::CConnectServer()
{
	my_player_collision = false;
}
CConnectServer::~CConnectServer()
{
}
void CConnectServer::ConnectServerInitialize(HWND hWnd, char* id, char* ip)
{
	WSAStartup(MAKEWORD(2, 2), &m_wsadata);
	m_client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	ZeroMemory(&m_server_addr, sizeof(SOCKADDR_IN));
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(SERVERPORT);
	m_server_addr.sin_addr.S_un.S_addr = inet_addr(ip);

	int retval = WSAConnect(m_client_sock, reinterpret_cast<sockaddr*>(&m_server_addr)
		, sizeof(m_server_addr), NULL, NULL, NULL, NULL);

	WSAAsyncSelect(m_client_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ);
	m_wsa_recv_buf.buf = m_crecv_buf;
	m_wsa_recv_buf.len = BUFF_SIZE;

	m_nCurrent_recvied = 0;
	m_nPrev_recived = 0;

	for (auto i = 0; i < MAX_USER; ++i) {
		other_player_pos[i].first = -50.0f;
		other_player_pos[i].second = -50.0f;
		other_player_view[i] = false;
	}

	for (auto i = 0; i < MAX_ENEMY; ++i) {
		enemy_view_state[i] = false;
		enemy_hp[i] = 0;
	}

}
void CConnectServer::SendPacket(unsigned char* packet)
{
	//memcpy(m_csend_buf, packet, packet[0]);
}
void CConnectServer::ProcessRecvPacket(char *ptr)
{
	static bool connect_server = true;
	switch (ptr[1]) // 1에는 항상 패킷 타입이 있다.
	{
	case STATE_MESAAGE: {
		sc_packet_state_message *packet = reinterpret_cast<sc_packet_state_message*>(ptr);
		wmemset(state_message.message, 0, 256);
		wcsncpy_s(state_message.message, packet->message, MAX_STR_SIZE);
		state_message.message_time = GetTickCount();
	}break;
	case PLAYER_ID: {
		sc_packet_player_id *packet = reinterpret_cast<sc_packet_player_id*>(ptr);
		int id = packet->id;
		if (id != m_nMy_client_id) {
			wcsncpy_s(other_player_id[id], packet->user_id, MAX_STR_SIZE);
		}
		break;
	}
	case PLAYER_COLLISION: {
		sc_packet_player_collision *packet = reinterpret_cast<sc_packet_player_collision*>(ptr);
		my_player_collision = packet->check;
		break;
	}
	case PLAYER_HP: {
		sc_packet_player_hp *packet = reinterpret_cast<sc_packet_player_hp*>(ptr);
		my_player_hp = packet->hp;
		break;
	}
	case PLAYER_PUT:
	{
		SC_Put_Player *packet = reinterpret_cast<SC_Put_Player*>(ptr);
		int id = packet->id;
		if (connect_server) {
			connect_server = false;
			m_nMy_client_id = id;
		}
		if (id == m_nMy_client_id) {
			my_player_pos.first = packet->x;
			my_player_pos.second = packet->y;
		}
		else {
			other_player_pos[id].first = packet->x;
			other_player_pos[id].second = packet->y;
			other_player_view[id] = true;
		}
		break;
	}
	case PLAYER_POP:
	{
		SC_POP_PLAYER *packet = reinterpret_cast<SC_POP_PLAYER*>(ptr);
		int id = packet->id;
		other_player_view[id] = false;
		break;
	}
	case PLAYER_POS: {
		SC_Player_Pos *packet = reinterpret_cast<SC_Player_Pos*>(ptr);
		int other_id = packet->id;
		if (other_id == m_nMy_client_id) {
			my_player_pos.first = packet->xPos;
			my_player_pos.second = packet->yPos;
		}
		else {
			other_player_pos[other_id].first = packet->xPos;
			other_player_pos[other_id].second = packet->yPos;
		}
		break;
	}
	case PLAYER_LEVEL: {
		sc_packet_player_level *packet = reinterpret_cast<sc_packet_player_level*>(ptr);
		int id = packet->id;
		if (id == m_nMy_client_id) {
			my_player_level = packet->level;
		}
		else {

		}
	}break;
	case PLAYER_EXP: {
		sc_packet_player_exp *packet = reinterpret_cast<sc_packet_player_exp*>(ptr);
		int id = packet->id;
		if (id == m_nMy_client_id) {
			my_player_exp = packet->exp;
		}
		else {

		}
	}break;
	case PLAYER_SPEACH: {
		sc_packet_player_speach *packet = reinterpret_cast<sc_packet_player_speach*>(ptr);
		int id = packet->id;
		if (id == m_nMy_client_id) {
			wmemset(my_player_speach.message, 0, 256);
			wcsncpy_s(my_player_speach.message, packet->message, 256);
			my_player_speach.message_time = GetTickCount();
		}
		else {
			wmemset(other_player_speach[id].message, 0, 256);
			wcsncpy_s(other_player_speach[id].message, packet->message, 256);
			other_player_speach[id].message_time = GetTickCount();
		}
	}break;
	case ENEMY_MOVE: {
		SC_NPC_POS *packet = reinterpret_cast<SC_NPC_POS*>(ptr);
		int id = packet->id;
		enemy_npc_pos[id].first = packet->x;
		enemy_npc_pos[id].second = packet->y;
		break;
	}
	case ENEMY_VIEW_STATE: {
		SC_NPC_VIEW *packet = reinterpret_cast<SC_NPC_VIEW*>(ptr);
		int id = packet->id;
		enemy_view_state[id] = packet->view_state;
		break;
	}
	case ENEMY_SPEACH: {
		SC_PACKET_NPC_SPEACH *packet = reinterpret_cast<SC_PACKET_NPC_SPEACH*>(ptr);
		int id = packet->id;
		wmemset(enemy_speach[id].message, 0, 256);
		wcsncpy_s(enemy_speach[id].message, packet->message, 256);
		enemy_speach[id].message_time = GetTickCount();
	}break;
	case ENEMY_HP: {
		sc_packet_npc_hp *packet = reinterpret_cast<sc_packet_npc_hp*>(ptr);
		int id = packet->id;
		enemy_hp[id] = packet->hp;
	}break;
	default: {

		break;
	}
	}
}
void CConnectServer::ReadRecvPacket(SOCKET sock)
{
	DWORD recivedbyte, flags = 0;
	int required = 0;

	int retval = WSARecv(sock, &m_wsa_recv_buf, 1, &recivedbyte, &flags, NULL, NULL);
	if (retval) {
		// process error
		if (retval != WSA_IO_PENDING) {
			int err_no = WSAGetLastError();
			while (true);
		}
	}
	BYTE *ptr = reinterpret_cast<BYTE*>(m_crecv_buf);
	while (0 != recivedbyte) {
		if (0 == m_nCurrent_recvied) m_nCurrent_recvied = ptr[0];

		required = m_nCurrent_recvied - m_nPrev_recived;

		if (required <= recivedbyte) {
			memcpy(m_packet_buf + m_nPrev_recived, ptr, required);
			ProcessRecvPacket(m_packet_buf);
			ptr += required;
			recivedbyte -= required;
			m_nCurrent_recvied = 0;
			m_nPrev_recived = 0;
		}
		else {
			memcpy(m_packet_buf + m_nPrev_recived, ptr, recivedbyte);
			m_nPrev_recived += recivedbyte;
			recivedbyte = 0;
		}
	}

}
void CConnectServer::DisConnectSerever()
{
	WSACleanup();
}