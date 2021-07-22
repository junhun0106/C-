#include "stdafx.h"
#include "ConnectServer.h"

//ksh7
#include "GameFramework.h"

extern cs_packet_db db_packet;
extern WCHAR in_name[200];
extern WCHAR jo_name[200];
CGameFramework part;

member party_members[4];
//ksh7
CConnectServer::CConnectServer()
{


}
CConnectServer::~CConnectServer()
{
}
void CConnectServer::ConnectServerInitialize(HWND hWnd)
{


   
	WSAStartup(MAKEWORD(2, 2), &m_wsadata);
	m_client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	ZeroMemory(&m_server_addr, sizeof(SOCKADDR_IN));
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(SERVERPORT);
	m_server_addr.sin_addr.S_un.S_addr = inet_addr(SERVERIP);
	int retval = WSAConnect(m_client_sock, reinterpret_cast<sockaddr*>(&m_server_addr)
		, sizeof(m_server_addr), NULL, NULL, NULL, NULL);

	//kshdb
	//클라 정보 전송 DB에 받은 캐릭터 정보들 서버로 보내기
	//send(m_client_sock, (char *)&db_packet, sizeof(db_packet), 0);
	//ksh


    // connect error if(retval ==... )
    WSAAsyncSelect(m_client_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ);

    m_wsa_recv_buf.buf = m_crecv_buf;
    m_wsa_recv_buf.len = BUFF_SIZE;

    m_nCurrent_recvied = 0;
    m_nPrev_recived = 0;

    for (auto i = 0; i < MAX_USER; ++i) {
        other_player_pos[i].first.first = 0.0f;
        other_player_pos[i].first.second = -500.0f;
        other_player_pos[i].second = 0.0f;
        other_player_look_vector[i].first.first = 0.0f;
        other_player_look_vector[i].first.second = 0.0;
        other_player_look_vector[i].second = 0.0;
        other_player_view_sate[i] = false;
    }

    for (auto i = 0; i < MAX_ENEMY; ++i) {
        enemy_view_state[i] = false;
        enemy_heat_beat_state[i] = true;
    }
	chat_message.resize(6);

	sunset_position.first.first = 0.0f;
	sunset_position.first.second = -1.0f;
	sunset_position.second = 0.0f;
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
    case PLAYER_EVENT_PUT:
    {
        sc_packet_put_player *packet = reinterpret_cast<sc_packet_put_player*>(ptr);
        int id = packet->id;
        if (connect_server) {
            connect_server = false;
            m_nMy_client_id = id;
        }
        if (id == m_nMy_client_id) {
            my_player_pos.first.first = packet->x;
            my_player_pos.first.second = packet->y;
            my_player_pos.second = packet->z;
        }
        else {
            other_player_pos[id].first.first = packet->x;
            other_player_pos[id].first.second = packet->y;
            other_player_pos[id].second = packet->z;
            other_player_look_vector[id].first.first = packet->look_x;
            other_player_look_vector[id].first.second = packet->look_y;
            other_player_look_vector[id].second = packet->look_z;
            other_player_view_sate[id] = true;
        }
        break;
    } break;
    case PLAYER_EVENT_POP:
    {
        sc_packet_pop_player *packet = reinterpret_cast<sc_packet_pop_player*>(ptr);
        int id = packet->id;
        other_player_view_sate[id] = false;
        break;
    } break;
    case PLAYER_EVENT_POS: {
        sc_packet_player_position *packet = reinterpret_cast<sc_packet_player_position*>(ptr);
        int other_id = packet->id;
        if (other_id == m_nMy_client_id) {
            my_player_pos.first.first = packet->xPos;
            my_player_pos.first.second = packet->yPos;
            my_player_pos.second = packet->zPos;
        }
        else {
            other_player_pos[other_id].first.first = packet->xPos;
            other_player_pos[other_id].first.second = packet->yPos;
            other_player_pos[other_id].second = packet->zPos;
            other_player_look_vector[other_id].first.first = packet->x_lock;
            other_player_look_vector[other_id].first.second = packet->y_look;
            other_player_look_vector[other_id].second = packet->z_lock;
            other_player_view_sate[other_id] = true;
        }
        break;
    } break;
    case PLAYER_ANIMATE_STATE: {
	      sc_packet_player_anim *packet = reinterpret_cast<sc_packet_player_anim*>(ptr);
        int id = packet->id;
        unsigned int state = packet->anim_state;
        other_player_animate_sate[id] = state;
        //cout << " recv " <<id << " client do " << other_player_animate_sate[id] << endl;
        break;
    } break;
	case PLAYER_EVENT_DECREASE_HP: {
		sc_packet_player_hp *packet = reinterpret_cast<sc_packet_player_hp*>(ptr);
		my_player_hp = packet->hp;
	}
								   break;
	case PLAYER_EVENT_GET_EXP: {
		sc_packet_player_exp *packet = reinterpret_cast<sc_packet_player_exp*>(ptr);
		my_player_exp = packet->exp; }
							   break;
	case PLAYER_EVENT_LEVEL_UP: {
		sc_packet_player_level *packet = reinterpret_cast<sc_packet_player_level*>(ptr);
		my_player_level = packet->level; }
								break;

	case FLOCK_EVENT_MOVE: {
		sc_packet_steer_position* packet = reinterpret_cast<sc_packet_steer_position*>(ptr);
		int id = packet->id;
		steer_npc_position[id].first = packet->x;
		steer_npc_position[id].second = packet->z;
		steer_npc_look_vector[id].first.first = packet->look_x;
		steer_npc_look_vector[id].first.second = packet->look_y;
		steer_npc_look_vector[id].second = packet->look_z;
	} break;
	case FLOCK_EVENT_VIEW: {
		sc_packet_steer_view* packet = reinterpret_cast<sc_packet_steer_view*>(ptr);
		int id = packet->id;
		steer_npc_view_state[id] = packet->state;
	}break;
	case PLAYER_EVENT_CHAT_MESSAGE: {
	sc_packet_chat_message *packet = reinterpret_cast<sc_packet_chat_message*>(ptr);
		for (auto i = 5; i > 0; --i)
			chat_message[i] = chat_message[i - 1];
		chat_message[0] = packet->message;

	}break;
	case SUNSET_EVENT: {
		sc_packet_sunset_position *packet = reinterpret_cast<sc_packet_sunset_position*>(ptr);
		sunset_position.first.first = packet->x;
		sunset_position.first.second = (packet->y);
		sunset_position.second = packet->z;
	}break;
	case PLAYER_EVENT_DEATH: {
		sc_packet_player_death *packet = reinterpret_cast<sc_packet_player_death*>(ptr);
		int id = packet->id;
		if (id == m_nMy_client_id) {
			my_player_state = false;
		}
		else {
			other_player_view_sate[id] = packet->state;
		}
	} break;
	case ENEMY_EVENT_ATTACK: {
		player_attack = true;
	}break;
	case ENEMY_EVENT_HP: {
		sc_packet_enemy_hp *packet = reinterpret_cast<sc_packet_enemy_hp*>(ptr);
		enemy_hp[packet->id] = packet->hp;
	} break;
	case ENEMY_EVENT_MOVE: {
		sc_packet_npc_position *packet = reinterpret_cast<sc_packet_npc_position*>(ptr);
		int id = packet->id;
		enemy_npc_pos[id].first.first = packet->x;
		enemy_npc_pos[id].first.second = packet->y;
		enemy_npc_pos[id].second = packet->z;
		enemy_npc_look_vector[id].first.first = packet->look_x;
		enemy_npc_look_vector[id].first.second = packet->look_y;
		enemy_npc_look_vector[id].second = packet->look_z;
		break;
	} break;
	case ENEMY_EVENT_VIEW_STATE: {
		sc_packet_npc_view *packet = reinterpret_cast<sc_packet_npc_view*>(ptr);
		int id = packet->id;
		enemy_view_state[id] = packet->view_state;
		break;
	} break;
	case ENEMY_EVENT_DEATH: {
		sc_packet_npc_view *packet = reinterpret_cast<sc_packet_npc_view*>(ptr);
		int id = packet->id;
		enemy_heat_beat_state[id] = packet->view_state;
		break;
	} break;
		//ksh7
	case PLAYER_PARTY_CHAT_MESSAGE: {
		sc_packet_chat_message *packet = reinterpret_cast<sc_packet_chat_message*>(ptr);
		//거절 한거 시스템 공지
		//띄워야 하는데.
	}break;
		//ksh7
		//ksh
	case 87:
	{
		sc_packet_party_player *my_packet = reinterpret_cast<sc_packet_party_player *>(ptr);
		if (my_packet->bool_invite == true && my_packet->bool_party_success == false) {
			wcscpy_s(in_name, my_packet->invite_name);	//초대파장 메시지, 상대 이름
			wcscpy_s(jo_name, my_packet->join_name);	// 초대메시지니까, 내이름이다.
			party_window = true;
		}
		break;
	} break;
	case 86:	// 파티원의 정보 갱신 받기.
	{
		sc_packet_party_info *my_packet = reinterpret_cast<sc_packet_party_info *>(ptr);

		//ksh7
		for (int i = 0; i < 3; i++) {
			if (&party_members[i] != NULL)
				if (wcscmp(db_packet.db_name, my_packet->member_name))
					if (!wcscmp(party_members[i].party_member, my_packet->member_name))
					{
						party_members[i].pHP = my_packet->hp;
						party_members[i].pLevel = my_packet->plevel;
						party_members[i].pXpos = my_packet->xPos;
						party_members[i].pYpos = my_packet->yPos;
					}
		}
		//ksh7
	}break;
	//ksh7
	case 85:	// 파티구조 변화 정보 받기.
	{
		setParty = true;
		sc_packet_party_set *my_packet = reinterpret_cast<sc_packet_party_set *>(ptr);
		for (int i = 0; i < 3; i++) {
			//if (wcscmp(party_members[i].party_member, my_packet->member_name))
					if (party_members[i].pHP == -5) {
						wcscpy_s(party_members[i].party_member, 50, my_packet->member_name);
						party_members[i].pCount = my_packet->m_count;
						party_members[i].pHP = my_packet->hp;
						party_members[i].pLevel = my_packet->level;
						party_members[i].pXpos = my_packet->xPos;
						party_members[i].pYpos = my_packet->yPos;
						break;
					}
		}
		// 정렬할까?
	}break;
	case 76:	//파티원이 탈퇴 할 경우
	{
		sc_packet_party_set *my_packet = reinterpret_cast<sc_packet_party_set *>(ptr);
		for (int i = 0; i < 3; i++) {
			//if (wcscmp(party_members[i].party_member, my_packet->member_name))
				//if (wcscmp(db_packet.db_name, my_packet->member_name))
				//{
					party_members[i].pHP = -5;
					break;
				//}
		}
	}
	//ksh7
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