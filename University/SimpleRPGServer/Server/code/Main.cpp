
#include "ServerFramework.h"

CServerFramework g_ServerFramework;
CCollisionCheck **g_SpacePartition;

HANDLE g_hIocp;
mutex event_queue_lock;

priority_queue<Timee_Queue> event_que;

void lua_err(lua_State *L) {
	cout << lua_tostring(L, -1) << endl; // 에러가 나면 무슨 에러인지 알고
	lua_pop(L, 1); // 그것을 꺼낸다.

}
void err_display(char *msg, int err_no)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %ws", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE  RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];


	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType,
		hHandle,
		++iRec,
	wszState,
		&iError,
	wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
		(SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}
void SendPacket(int id, unsigned char* packet) {
	OverlapEx *send_overlap = new OverlapEx;
	memset(send_overlap, 0, sizeof(OverlapEx));
	send_overlap->m_nOperation = OP_SEND;
	send_overlap->m_wsa_buf.buf = reinterpret_cast<char*>(send_overlap->m_socket_buf);
	send_overlap->m_wsa_buf.len = packet[0];

	memcpy(send_overlap->m_socket_buf, packet, packet[0]);

	int retval = WSASend(g_ServerFramework.GetPlayerInfo(id)->GetSocketInfo(), &send_overlap->m_wsa_buf, 1,
		NULL, 0, &send_overlap->m_original_overlapped, NULL);
	if (retval != 0) {
		int error_no = WSAGetLastError();
		if (WSA_IO_PENDING != error_no) {
			err_display("SendPacket WSASend : ", error_no);
		}
	}
}
void SendPutPlayerPacket(int client, int player) {
	SC_Put_Player packet;
	packet.id = player;
	packet.size = sizeof(SC_Put_Player);
	packet.type = PLAYER_PUT;
	packet.x = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetPosX();
	packet.y = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetPosY();

	SendPacket(client, reinterpret_cast<unsigned char*>(&packet));
}
void SendRemovePlayerPacket(int client, int player) {
	SC_POP_PLAYER packet;
	packet.id = player;
	packet.size = sizeof(SC_POP_PLAYER);
	packet.type = PLAYER_POP;
	SendPacket(client, reinterpret_cast<unsigned char*>(&packet));
}
void SendChatPacket(int enemy_id, int player_id, wchar_t *text) {
	SC_PACKET_NPC_SPEACH packet;
	packet.id = enemy_id;
	wcscpy_s(packet.message, text);
	packet.size = sizeof(SC_PACKET_NPC_SPEACH);
	packet.type = ENEMY_SPEACH;
	SendPacket(player_id, reinterpret_cast<unsigned char*>(&packet));
}
void SendMessagePacket(int player_id, wchar_t *text) {
	sc_packet_state_message packet;
	wcscpy_s(packet.message, text);
	packet.size = sizeof(sc_packet_state_message);
	packet.type = STATE_MESAAGE;
	SendPacket(player_id, reinterpret_cast<unsigned char*>(&packet));
}
int API_SendMessage(lua_State *L) {
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char *mess = (char*)lua_tostring(L, -1);
	lua_pop(L, 4);

	wchar_t wmess[MAX_STR_SIZE];
	size_t wlen, len = strlen(mess) + 1;
	len = ((MAX_STR_SIZE - 1) < len) ? MAX_STR_SIZE - 1 : len;
	mbstowcs_s(&wlen, wmess, len, mess, _TRUNCATE);
	wmess[MAX_STR_SIZE - 1] = (wchar_t)0;


	SendChatPacket(my_id, user_id, wmess);

	return 0;
}
int API_Collision(lua_State *L) {
	int my_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	g_ServerFramework.GetEnemyObject(my_id)->StartTime();
	g_ServerFramework.GetEnemyObject(my_id)->StartTimeBool();
	return 0;
}
int API_PLAYER_GET_x(lua_State *L) {
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = g_ServerFramework.GetClientInfo(user_id)->GetPlayerInfo()->GetPosX();
	lua_pushnumber(L, x);
	return 1;
}
int API_PLAYER_GET_y(lua_State *L) {
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = g_ServerFramework.GetClientInfo(user_id)->GetPlayerInfo()->GetPosY();
	lua_pushnumber(L, y);
	return 1;
}
int API_GET_x(lua_State *L) {
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = g_ServerFramework.GetEnemyObject(user_id)->GetPosX();
	lua_pushnumber(L, x);
	return 1;
}
int API_GET_y(lua_State *L) {
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = g_ServerFramework.GetEnemyObject(user_id)->GetPosY();
	lua_pushnumber(L, y);
	return 1;
}
void AcceptThread()
{
	struct sockaddr_in listen_addr;
	SOCKET accept_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(SERVERPORT);
	ZeroMemory(&listen_addr.sin_zero, 8);
	::bind(accept_socket, reinterpret_cast<SOCKADDR*>(&listen_addr), sizeof(listen_addr));
	::listen(accept_socket, MAX_USER);


	while (1)
	{
		SOCKADDR_IN client_addr;
		int client_size = sizeof(client_addr);
		SOCKET client_sock = NULL;
		int new_id = -1;
		cout << "accept ready \n";
		client_sock = WSAAccept(accept_socket, reinterpret_cast<SOCKADDR*>(&client_addr), &client_size, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			cout << "socket error" << endl;
			closesocket(client_sock);
		}

				for (int i = 0; i < MAX_USER; ++i) {
					if (!g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) {
						new_id = i;
						cout << new_id << endl;
						break;
					}
				}
				if (-1 == new_id) {
					std::cout << " FULL SERVER " << endl;
					closesocket(client_sock);
					continue;
				}

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), g_hIocp, new_id, 0);

				CPlayer *player = new CPlayer();
				player->SetPosition(20.0f * 40, 200.0f * 40 + 40.0f);
				g_ServerFramework.GetPlayerInfo(new_id)->SetInfo(client_sock, player, new_id);
				g_ServerFramework.GetPlayerInfo(new_id)->SetIsConnected(true);

				_asm mfence;
				SC_Put_Player put_player;
				put_player.id = new_id;
				put_player.size = sizeof(SC_Put_Player);
				put_player.type = PLAYER_PUT;
				PlayerInfo *info = g_ServerFramework.GetPlayerInfo(new_id);
				float x = info->GetPlayerInfo()->GetPosX(), y = info->GetPlayerInfo()->GetPosY();
				put_player.x = x;
				put_player.y = y;

				sc_packet_player_level player_level;
				player_level.size = sizeof(sc_packet_player_level);
				player_level.type = PLAYER_LEVEL;
				player_level.id = new_id;
				player_level.level = 0;

				sc_packet_player_exp player_exp;
				player_exp.size = sizeof(sc_packet_player_exp);
				player_exp.type = PLAYER_EXP;
				player_exp.id = new_id;
				player_exp.exp = 0;

				sc_packet_player_hp player_hp;
				player_hp.size = sizeof(sc_packet_player_hp);
				player_hp.type = PLAYER_HP;
				player_hp.hp = 0 * 100;

				sc_packet_player_id player_id;
				player_id.size = sizeof(sc_packet_player_id);
				wcsncpy_s(player_id.user_id, L"ID", MAX_STR_SIZE);
				player_id.id = new_id;
				player_id.type = PLAYER_ID;

				for (auto i = 0; i < MAX_USER; ++i) { // 접속해 있는 플레이어에게 나의 위치 전송
					if (g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) {
						if (false == g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->
							Distance(g_ServerFramework.GetPlayerInfo(new_id)->GetPlayerInfo())) continue;

						g_ServerFramework.GetPlayerInfo(i)->m_vl_lock.lock();
						if (i != new_id) g_ServerFramework.GetPlayerInfo(i)->m_view_list.insert(new_id);
						g_ServerFramework.GetPlayerInfo(i)->m_vl_lock.unlock();

						SendPacket(i, reinterpret_cast<unsigned char*>(&put_player));

						if (i != new_id) continue;

						SendPacket(i, reinterpret_cast<unsigned char*>(&player_level));
						SendPacket(i, reinterpret_cast<unsigned char*>(&player_exp));
						SendPacket(i, reinterpret_cast<unsigned char*>(&player_hp));
						//SendPacket(i, reinterpret_cast<unsigned char*>(&player_id));
					}
				}
				for (auto i = 0; i < MAX_USER; ++i) { // 나에게 접속해 있는 플레이어들 위치 전송
					if (g_ServerFramework.GetPlayerInfo(i)->GetIsConnect() && i != new_id) {

						if (false == g_ServerFramework.GetPlayerInfo(new_id)->GetPlayerInfo()->Distance(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo())) continue;


						g_ServerFramework.GetPlayerInfo(new_id)->m_vl_lock.lock();
						g_ServerFramework.GetPlayerInfo(new_id)->m_view_list.insert(new_id);
						g_ServerFramework.GetPlayerInfo(new_id)->m_vl_lock.unlock();

						put_player.id = i;
						float x = g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetPosX(), y = g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetPosY();
						put_player.x = x;
						put_player.y = y;

						wcsncpy_s(player_id.user_id, g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->GetName(), MAX_STR_SIZE);


						SendPacket(new_id, reinterpret_cast<unsigned char*>(&put_player));
						//SendPacket(new_id, reinterpret_cast<unsigned char*>(&player_id));
					}
				}

				DWORD flags = 0;
				int retval = WSARecv(client_sock, &info->m_OverlappedEx.m_wsa_buf, 1, NULL,
					&flags, &info->m_OverlappedEx.m_original_overlapped, NULL);
				if (0 != retval) {
					int error_no = WSAGetLastError();
					if (WSA_IO_PENDING != error_no) {
						err_display(" AcceptThread:WSARecv : ", error_no);
					}
				}
			}

}
void ProcessPacket(int id, unsigned char* packet) {
	PlayerInfo *clientinfo = g_ServerFramework.GetPlayerInfo(id);
	unsigned char packet_type = packet[1];
	float x = clientinfo->GetPlayerInfo()->GetPosX();
	float y = clientinfo->GetPlayerInfo()->GetPosY();

	bool attack = false;
	bool chat = false;

	int pred_x = x; 
	int pred_y = y;

	switch (packet_type) {
	case VK_UPBUTTON: 
		if(y > 50.0f) y -= 40.0f;
		break;
	case VK_RIGHTBUTTON: 
		if(x < WORLD_WIDTH - 50.0f ) x += 40.0f;
		break;
	case VK_LEFTBUTTON: 
		if(x > 50.0f) x -= 40.0f;
		break;
	case VK_DOWNBUTTON: 
		if(y < WORLD_HEIGHT - 50.0f) y += 40.0f;
		break;
	case VK_SPACEBAR:
		attack = true;
		break;
	case PLAYER_SPEACH:
		chat = true;
		break;
	default:
		cout << id << "Client is UnKnown Packet Typed \n";
		// process error
		break;
	}
	sc_packet_player_collision player_collision;
	player_collision.size = sizeof(sc_packet_player_collision);
	player_collision.type = PLAYER_COLLISION;
	player_collision.check = false;
	if (g_SpacePartition[WORLD_MAP]->check_collision(x / 40, y / 40)) {
		player_collision.check = true;
		x = pred_x; y = pred_y;
	}

	clientinfo->GetPlayerInfo()->SetPosition(x, y);
	SC_Player_Pos player_pos;
	player_pos.size = sizeof(SC_Player_Pos);
	player_pos.type = PLAYER_POS;
	player_pos.id = id;
	player_pos.xPos = x;
	player_pos.yPos = y;

	sc_packet_player_id player_id;
	player_id.size = sizeof(sc_packet_player_id);
	wcsncpy_s(player_id.user_id, clientinfo->GetPlayerInfo()->GetName(), MAX_STR_SIZE);
	player_id.id = id;
	player_id.type = PLAYER_ID;

	//enemy process
	for (auto i = 0; i < MAX_ENEMY; ++i) {
		// 죽었다면
		if (false == g_ServerFramework.GetEnemyObject(i)->GetDeath()) {
			continue;
		}
		// 움직인 플레이어와의 거리가 멀다면
		if (false == g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->Distance(g_ServerFramework.GetEnemyObject(i))) continue;
		// 처음 만났다면
		if (false == g_ServerFramework.GetEnemyObject(i)->GetActive())
		{
			g_ServerFramework.GetEnemyObject(i)->WakeUp_NPC();
			if (MONSTER_STATIC_ATTACK == g_ServerFramework.GetEnemyObject(i)->GetType()) continue;
			if (MONSTER_STATIC_NO_ATTACK == g_ServerFramework.GetEnemyObject(i)->GetType()) continue;
			// 고정형 몹이 아니라면
			Timee_Queue enemy;
			enemy.add_timer(i, ENEMY_MOVE);
			event_queue_lock.lock();
			event_que.push(enemy);
			event_queue_lock.unlock();
		}
		// attack
		if (attack) {
			if (false == g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->Collision(g_ServerFramework.GetEnemyObject(i))) continue;
			g_ServerFramework.GetEnemyObject(i)->DecreaseHp();

			if (false == g_ServerFramework.GetEnemyObject(i)->GetTargetMode()) {
				if (g_ServerFramework.GetEnemyObject(i)->GetType() == MONSTER_STATIC_NO_ATTACK ||
					g_ServerFramework.GetEnemyObject(i)->GetType() == MONSTER_DYNAMIC_NO_ATTACK) {

					g_ServerFramework.GetEnemyObject(i)->SetTargetID(id);
					g_ServerFramework.GetEnemyObject(i)->SetTargetMode(true);

					SendChatPacket(i, id, L"ATTACK!!");

					Timee_Queue enemy;
					enemy.add_timer(i, ENEMY_ATTACK_MOVE);
					event_queue_lock.lock();
					event_que.push(enemy);
					event_queue_lock.unlock();
				}
			}
			wchar_t text[MAX_STR_SIZE];
			wmemset(text, 0, MAX_STR_SIZE);
			wsprintf(text, L" %d 몬스터에게 50 damage를 주었습니다.", i);

			SendMessagePacket(id, text);

			int hp = g_ServerFramework.GetEnemyObject(i)->GetHP();

			if (hp < 0) {
				g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->IncreaseEXP(g_ServerFramework.GetEnemyObject(i)->GetExp());

				wchar_t text[MAX_STR_SIZE];
				wmemset(text, 0, MAX_STR_SIZE);
				wsprintf(text, L" %d 경험치를 얻었습니다 ", g_ServerFramework.GetEnemyObject(i)->GetExp());

				SendMessagePacket(id, text);

				sc_packet_player_exp player_exp;
				player_exp.size = sizeof(sc_packet_player_exp);
				player_exp.type = PLAYER_EXP;
				player_exp.id = id;
				player_exp.exp = g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->GetPlayerEXP();
				SendPacket(id, reinterpret_cast<unsigned char*>(&player_exp));

				// level up packet
				if (g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->GetLevelUP()) {

					sc_packet_player_level player_level;
					player_level.size = sizeof(sc_packet_player_level);
					player_level.type = PLAYER_LEVEL;
					player_level.id = id;
					player_level.level = g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->GetLevel();
					SendPacket(id, reinterpret_cast<unsigned char*>(&player_level));

					wchar_t text[MAX_STR_SIZE];
					wmemset(text, 0, MAX_STR_SIZE);
					wsprintf(text, L" %d 레벨로 올랐습니다!! ", g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->GetLevel());

					SendMessagePacket(id, text);

					g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->SetLevelUp();
				}

				// monster death packet
				g_ServerFramework.GetEnemyObject(i)->Sleep_NPC();
				g_ServerFramework.GetEnemyObject(i)->SetDeath(false);
				SC_NPC_VIEW npc_view;
				npc_view.id = i;
				npc_view.type = ENEMY_VIEW_STATE;
				npc_view.size = sizeof(SC_NPC_VIEW);
				npc_view.view_state = false;

				for (auto user_id = 0; user_id < MAX_USER; ++user_id) {
					if (false == g_ServerFramework.GetClientInfo(user_id)->GetIsConnect()) continue;
					if (false == g_ServerFramework.GetClientInfo(user_id)->GetPlayerInfo()
						->Distance(g_ServerFramework.GetEnemyObject(i))) continue;
					SendPacket(user_id, reinterpret_cast<unsigned char*>(&npc_view));
				}
				Timee_Queue enemy;
				enemy.add_timer(i, ENEMY_REGEN);
				event_queue_lock.lock();
				event_que.push(enemy);
				event_queue_lock.unlock();
			}
			else {
				sc_packet_npc_hp npc_hp;
				npc_hp.hp = hp;
				npc_hp.size = sizeof(sc_packet_npc_hp);
				npc_hp.id = i;
				npc_hp.type = ENEMY_HP;

				for (auto user_id = 0; user_id < MAX_USER; ++user_id) {
					if (false == g_ServerFramework.GetClientInfo(user_id)->GetIsConnect()) continue;
					if (false == g_ServerFramework.GetClientInfo(user_id)->GetPlayerInfo()
						->Distance(g_ServerFramework.GetEnemyObject(i))) continue;
					SendPacket(user_id, reinterpret_cast<unsigned char*>(&npc_hp));
				}
			}
		}

		// 공격형 몬스터 AI
		if (false == g_ServerFramework.GetEnemyObject(i)->GetTargetMode()) {
			if (g_ServerFramework.GetEnemyObject(i)->GetType() == MONSTER_STATIC_ATTACK) {
				if (g_ServerFramework.GetEnemyObject(i)->Distance(g_ServerFramework.GetClientInfo(id)->GetPlayerInfo())) {
					SendChatPacket(i, id, L"ATTACK!!");
					// worker thread a* 알고리즘 pqcs
					if (!g_ServerFramework.GetEnemyObject(i)->GetTargetMode()) {
						g_ServerFramework.GetEnemyObject(i)->SetTargetID(id);
						g_ServerFramework.GetEnemyObject(i)->SetTargetMode(true);
					}
					Timee_Queue enemy;
					enemy.add_timer(i, ENEMY_ATTACK_MOVE);
					event_queue_lock.lock();
					event_que.push(enemy);
					event_queue_lock.unlock();
				}
			}
		}

		OverlapEx *overlapex = new OverlapEx;
		memset(overlapex, 0, sizeof(OverlapEx));
		overlapex->m_nOperation = OP_EVENT_PLAYER_MOVE;
		overlapex->param1 = id;
		PostQueuedCompletionStatus(g_hIocp, 1, i, &(overlapex->m_original_overlapped));

		// 고정형 몬스터 AI
		if (MONSTER_STATIC_ATTACK == g_ServerFramework.GetEnemyObject(i)->GetType()
			|| MONSTER_STATIC_NO_ATTACK == g_ServerFramework.GetEnemyObject(i)->GetType()) {
			SC_NPC_VIEW npc_view;
			npc_view.id = i;
			npc_view.size = sizeof(SC_NPC_VIEW);
			npc_view.type = ENEMY_VIEW_STATE;

			set<int> new_list;
			if (g_ServerFramework.GetClientInfo(id)->GetPlayerInfo()->Distance(g_ServerFramework.GetEnemyObject(i))) {
				// 뷰 리스트에 없는 경우 뷰 리스트에 추가 한다.
				SC_NPC_POS npc_pos;
				npc_pos.id = i;
				npc_pos.size = sizeof(SC_NPC_POS);
				npc_pos.type = ENEMY_MOVE;
				npc_pos.x = g_ServerFramework.GetEnemyObject(i)->GetPosX();
				npc_pos.y = g_ServerFramework.GetEnemyObject(i)->GetPosY();

				sc_packet_npc_hp npc_hp;
				npc_hp.hp = g_ServerFramework.GetEnemyObject(i)->GetHP();
				npc_hp.size = sizeof(sc_packet_npc_hp);
				npc_hp.id = i;
				npc_hp.type = ENEMY_HP;
				g_ServerFramework.GetClientInfo(id)->m_npc_vl_lock.lock();
				if (false == g_ServerFramework.GetClientInfo(id)->m_npc_view_list.count(i)) {
					g_ServerFramework.GetClientInfo(id)->m_npc_view_list.insert(i);
					g_ServerFramework.GetClientInfo(id)->m_npc_vl_lock.unlock();
					npc_view.view_state = true;
					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_view));
					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_pos));
					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_hp));
				}
				else { // 뷰 리스트에 있는 경우
					g_ServerFramework.GetClientInfo(id)->m_npc_vl_lock.unlock();
					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_pos));
				}
			}
			else {
				g_ServerFramework.GetClientInfo(id)->m_npc_vl_lock.lock();
				if (true == g_ServerFramework.GetClientInfo(id)->m_npc_view_list.count(i))
				{
					g_ServerFramework.GetClientInfo(id)->m_npc_view_list.erase(i);
					g_ServerFramework.GetClientInfo(id)->m_npc_vl_lock.unlock();
					npc_view.view_state = false;
					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_view));
				}
			}
		}
	}
	set<int> new_view_list;
	for (auto view_list_player = 0; view_list_player < MAX_USER; ++view_list_player) {
		if (g_ServerFramework.GetPlayerInfo(view_list_player)->GetIsConnect() &&
			g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->Distance(g_ServerFramework.GetPlayerInfo(view_list_player)->GetPlayerInfo()))
			new_view_list.insert(view_list_player);
	}
	// new view player process
	for (auto view_player : new_view_list) {

		if (view_player >= MAX_USER) {
			cout << " fuck " << endl;
			continue;
		}

		g_ServerFramework.GetPlayerInfo(id)->lock();
		bool new_one = (0 == g_ServerFramework.GetPlayerInfo(id)->m_view_list.count(view_player));
		if (new_one) g_ServerFramework.GetPlayerInfo(id)->m_view_list.insert(view_player);
		g_ServerFramework.GetPlayerInfo(id)->unlock();
		if (new_one) {
			SendPutPlayerPacket(id, view_player);
			//SendPacket(id, reinterpret_cast<unsigned char*>(&player_id));
		}

		g_ServerFramework.GetPlayerInfo(view_player)->lock();
		if (0 == g_ServerFramework.GetPlayerInfo(view_player)->m_view_list.count(id)) {
			g_ServerFramework.GetPlayerInfo(view_player)->m_view_list.insert(id);
			g_ServerFramework.GetClientInfo(view_player)->unlock();
			SendPutPlayerPacket(view_player, id);
			//SendPacket(id, reinterpret_cast<unsigned char*>(&player_id));
		}
		else {
			g_ServerFramework.GetClientInfo(view_player)->unlock();
			if (chat) {
				sc_packet_player_speach player_speach;
				player_speach.id = id;
				player_speach.size = sizeof(sc_packet_player_speach);
				player_speach.type = PLAYER_SPEACH;
				cs_packet_player_speach* cs_player_speach = reinterpret_cast<cs_packet_player_speach*>(packet);
				wcsncpy_s(player_speach.message, cs_player_speach->message, MAX_STR_SIZE);
				SendPacket(view_player, reinterpret_cast<unsigned char*>(&player_speach));
			}
			SendPacket(view_player, reinterpret_cast<unsigned char*>(&player_pos));
		}
	}
	// remove view player process
	vector<int> remove_list;
	remove_list.reserve(5);

	g_ServerFramework.GetClientInfo(id)->lock();
	for (auto remove_id : g_ServerFramework.GetClientInfo(id)->m_view_list) {
		if (0 != new_view_list.count(remove_id)) continue;
		remove_list.push_back(remove_id);
	}

	for (auto remove_list_id : remove_list) g_ServerFramework.GetClientInfo(id)->m_view_list.erase(remove_list_id);
	g_ServerFramework.GetClientInfo(id)->unlock();


	for (auto remove_player : remove_list) {
		SendRemovePlayerPacket(id, remove_player);
		g_ServerFramework.GetClientInfo(remove_player)->lock();
		if (0 == g_ServerFramework.GetClientInfo(remove_player)->m_view_list.count(id)) {
			g_ServerFramework.GetClientInfo(remove_player)->unlock();
			continue;
		}

		g_ServerFramework.GetClientInfo(remove_player)->m_view_list.erase(id);
		g_ServerFramework.GetClientInfo(remove_player)->unlock();
		SendRemovePlayerPacket(remove_player, id);
	}
}
void ProcessNPCPakcet(int id, unsigned char type) {
	switch (type) {
	case ENEMY_MOVE: {
		g_ServerFramework.GetEnemyObject(id)->Enemy_Move();
		bool player_view = false;
		bool attack_mode = false;
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			SC_NPC_VIEW npc_view;
			npc_view.id = id;
			npc_view.size = sizeof(SC_NPC_VIEW);
			npc_view.type = ENEMY_VIEW_STATE;

			set<int> new_list;
			if (g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->Distance(g_ServerFramework.GetEnemyObject(id))) {
				// 뷰 리스트에 없는 경우 뷰 리스트에 추가 한다.
				player_view = true;
				SC_NPC_POS npc_pos;
				npc_pos.id = id;
				npc_pos.size = sizeof(SC_NPC_POS);
				npc_pos.type = ENEMY_MOVE;
				npc_pos.x = g_ServerFramework.GetEnemyObject(id)->GetPosX();
				npc_pos.y = g_ServerFramework.GetEnemyObject(id)->GetPosY();

				sc_packet_npc_hp npc_hp;
				npc_hp.hp = g_ServerFramework.GetEnemyObject(id)->GetHP();
				npc_hp.size = sizeof(sc_packet_npc_hp);
				npc_hp.id = id;
				npc_hp.type = ENEMY_HP;

				g_ServerFramework.GetClientInfo(i)->m_npc_vl_lock.lock();
				if (false == g_ServerFramework.GetClientInfo(i)->m_npc_view_list.count(id)) {
					g_ServerFramework.GetClientInfo(i)->m_npc_view_list.insert(id);
					g_ServerFramework.GetClientInfo(i)->m_npc_vl_lock.unlock();
					npc_view.view_state = true;
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_pos));
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_hp));
				}
				else { // 뷰 리스트에 있는 경우
					// 이동형 선공 몹 AI
					if (false == g_ServerFramework.GetEnemyObject(id)->GetTargetMode()) {
						if (g_ServerFramework.GetEnemyObject(id)->GetType() == MONSTER_DYNAMIC_ATTACK) {
							if (g_ServerFramework.GetEnemyObject(id)->Distance(g_ServerFramework.GetClientInfo(i)->GetPlayerInfo())) {
								SendChatPacket(id, i, L"ATTACK!!");
								attack_mode = true;
								// worker thread a* 알고리즘 pqcs
								if (!g_ServerFramework.GetEnemyObject(id)->GetTargetMode()) {
									g_ServerFramework.GetEnemyObject(id)->SetTargetID(i);
									g_ServerFramework.GetEnemyObject(id)->SetTargetMode(true);
								}
								Timee_Queue enemy;
								enemy.add_timer(id, ENEMY_ATTACK_MOVE);
								event_queue_lock.lock();
								event_que.push(enemy);
								event_queue_lock.unlock();
							}
						}
					}
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_pos));
				}
				if (g_ServerFramework.GetEnemyObject(id)->GetTimeBool()) {
					if ((g_ServerFramework.GetEnemyObject(id)->GetTime() < GetTickCount() - 3000))
					{
						OverlapEx *overlapex = new OverlapEx;
						memset(overlapex, 0, sizeof(OverlapEx));
						overlapex->m_nOperation = OP_EVENT_ENEMY_MOVE;
						overlapex->param1 = i;
						PostQueuedCompletionStatus(g_hIocp, 1, id, &(overlapex->m_original_overlapped));
						g_ServerFramework.GetEnemyObject(id)->EndTimeBool();
					}
				}
			}
			else {
				g_ServerFramework.GetClientInfo(i)->m_npc_vl_lock.lock();
				if (true == g_ServerFramework.GetClientInfo(i)->m_npc_view_list.count(id))
				{
					g_ServerFramework.GetClientInfo(i)->m_npc_view_list.erase(id);
					g_ServerFramework.GetClientInfo(i)->m_npc_vl_lock.unlock();
					npc_view.view_state = false;
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
				}
			}
		}

		if (player_view && !attack_mode && g_ServerFramework.GetEnemyObject(id)->GetTargetMode() == false) {
			Timee_Queue enemy;
			enemy.add_timer(id, ENEMY_MOVE);
			event_queue_lock.lock();
			event_que.push(enemy);
			event_queue_lock.unlock();
		}
		else if(!player_view){
			g_ServerFramework.GetEnemyObject(id)->Sleep_NPC();
		}
	}
		break;
	case ENEMY_REGEN: {
		pair<float, float> pos = g_ServerFramework.GetEnemyObject(id)->GetRegenPos();

		g_ServerFramework.GetEnemyObject(id)->SetPosition(pos.first, pos.second);
		g_ServerFramework.GetEnemyObject(id)->SetDeath(true);
		g_ServerFramework.GetEnemyObject(id)->ReSetHp();

		for (auto i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			if (false == g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->Distance(g_ServerFramework.GetEnemyObject(id))) continue;
			
			g_ServerFramework.GetEnemyObject(id)->WakeUp_NPC();

			SC_NPC_VIEW npc_view;
			npc_view.type = ENEMY_VIEW_STATE;
			npc_view.size = sizeof(SC_NPC_VIEW);
			npc_view.id = id;
			npc_view.view_state = true;

			sc_packet_npc_hp npc_hp;
			npc_hp.hp = g_ServerFramework.GetEnemyObject(id)->GetHP();
			npc_hp.size = sizeof(sc_packet_npc_hp);
			npc_hp.id = id;
			npc_hp.type = ENEMY_HP;

			SC_NPC_POS npc_pos;
			npc_pos.id = id;
			npc_pos.size = sizeof(SC_NPC_POS);
			npc_pos.type = ENEMY_MOVE;
			npc_pos.x = g_ServerFramework.GetEnemyObject(id)->GetPosX();
			npc_pos.y = g_ServerFramework.GetEnemyObject(id)->GetPosY();

			SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
			SendPacket(i, reinterpret_cast<unsigned char*>(&npc_hp));
			SendPacket(i, reinterpret_cast<unsigned char*>(&npc_pos));
			SendChatPacket(id, i, L"RESPAWN!!");


			if (MONSTER_STATIC_ATTACK == g_ServerFramework.GetEnemyObject(i)->GetType()) continue;
			if (MONSTER_STATIC_NO_ATTACK == g_ServerFramework.GetEnemyObject(i)->GetType()) continue;

			Timee_Queue enemy;
			enemy.add_timer(id, ENEMY_MOVE);
			event_queue_lock.lock();
			event_que.push(enemy);
			event_queue_lock.unlock();
		}

	} break;
	default:
		cout << id << " NPC UnKnown Type" << endl;
		break;
	}
}
void ProcessASTAR(int id) {


	CDynamicObject *object = g_ServerFramework.GetEnemyObject(id);
	if (object->GetDeath() == false) return;

	int target_id = object->GetTargetID();
	if (target_id == -1) {
		cout << "Not Found Target " << endl;
		return;
	}
	CPlayer *target_player = g_ServerFramework.GetClientInfo(target_id)->GetPlayerInfo();

	if (false == target_player->Distance(object)) {
		object->SetTargetMode(false);
		return;
	}

	if (target_player->Collision(object) == false) {
		int x = object->GetPosX() / 40;
		int y = object->GetPosY() / 40;
		int target_x = target_player->GetPosX() / 40;
		int target_y = target_player->GetPosY() / 40;
		pair<int, int> point;
		PathNode *node = new PathNode(x, y, true);
		point = g_ServerFramework.StartAstar(node, target_x, target_y);
		delete node;
		point.first = point.first * 40 + 20;
		point.second = point.second * 40 + 20;
		object->SetPosition(point.first, point.second);

		SC_NPC_POS npc_pos;
		npc_pos.id = id;
		npc_pos.size = sizeof(SC_NPC_POS);
		npc_pos.type = ENEMY_MOVE;
		npc_pos.x = point.first;
		npc_pos.y = point.second;

		if (g_ServerFramework.GetClientInfo(target_id)->GetIsConnect()) {
			SendPacket(target_id, reinterpret_cast<unsigned char*>(&npc_pos));
		}
	}
	else {
		target_player->DecreaseHp(object->GetDamege());
		int hp = target_player->GetPlayerHp();
		if (hp <= 0) {
			sc_packet_player_hp player_hp;
			player_hp.size = sizeof(sc_packet_player_hp);
			player_hp.type = PLAYER_HP;
			player_hp.hp = target_player->GetLevel() * 50;

			SC_Player_Pos player_pos;
			player_pos.size = sizeof(SC_Player_Pos);
			player_pos.id = target_id;
			player_pos.type = PLAYER_POS;
			player_pos.xPos = 20.0f;
			player_pos.yPos = 20.0f;

			target_player->SetPosition(20.0f, 20.0f);
			target_player->SetPlayerHp(target_player->GetLevel() * 50);

			if (g_ServerFramework.GetClientInfo(target_id)->GetIsConnect()) {
				SendPacket(target_id, reinterpret_cast<unsigned char*>(&player_pos));
				SendPacket(target_id, reinterpret_cast<unsigned char*>(&player_hp));
			}
			if (target_player->GetFeedBack() == false) {
				target_player->SetFeedBack(true);
				Timee_Queue player;
				player.add_timer(id, PLAYER_HP);
				event_queue_lock.lock();
				event_que.push(player);
				event_queue_lock.unlock();
			}
		}
		else {
			sc_packet_player_hp player_hp;
			player_hp.size = sizeof(sc_packet_player_hp);
			player_hp.type = PLAYER_HP;
			player_hp.hp = target_player->GetPlayerHp();
			if (g_ServerFramework.GetClientInfo(target_id)->GetIsConnect()) {
				SendPacket(target_id, reinterpret_cast<unsigned char*>(&player_hp));
			}
			// HP 회복 타이머 저장
			if (target_player->GetFeedBack() == false) {
				target_player->SetFeedBack(true);
				Timee_Queue player;
				player.add_timer(target_id, PLAYER_HP);
				event_queue_lock.lock();
				event_que.push(player);
				event_queue_lock.unlock();
			}
		}
	}
	Timee_Queue enemy;
	enemy.add_timer(id, ENEMY_ATTACK_MOVE);
	event_queue_lock.lock();
	event_que.push(enemy);
	event_queue_lock.unlock();
}
void HealingPlayeR(int id) {
	CPlayer *player = g_ServerFramework.GetClientInfo(id)->GetPlayerInfo();
	if (g_ServerFramework.GetClientInfo(id)->GetIsConnect() == false) return;

	if (player->GetLevel() * 100 != player->GetPlayerHp()) {
		player->IncreaseHp();

		sc_packet_player_hp player_hp;
		player_hp.size = sizeof(sc_packet_player_hp);
		player_hp.type = PLAYER_HP;
		player_hp.hp = player->GetPlayerHp();

		SendPacket(id, reinterpret_cast<unsigned char*>(&player_hp));

		Timee_Queue player;
		player.add_timer(id, PLAYER_HP);
		event_queue_lock.lock();
		event_que.push(player);
		event_queue_lock.unlock();
	}
	else {
		player->SetFeedBack(false);
	}
}
void TimerThread() {
	// NPC_CREATE
	for (auto i = 0; i < MAX_ENEMY; ++i) {
		g_ServerFramework.GetEnemyObject(i)->SetNPCID(i);

		lua_State *L = g_ServerFramework.GetEnemyObject(i)->L = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadfile(L, "monster.lua");
		int error = lua_pcall(L, 0, 0, 0);
		if (error) lua_err(L);
		lua_getglobal(L, "set_uid");
		lua_pushnumber(L, i);
		lua_pcall(L, 1, 1, 0);
		lua_pop(L, 1);

		lua_register(L, "API_SendMessage", API_SendMessage);
		lua_register(L, "API_Collision", API_Collision);
		lua_register(L, "API_get_x", API_GET_x);
		lua_register(L, "API_get_y", API_GET_y);
		lua_register(L, "API_PLAYER_GET_x", API_PLAYER_GET_x);
		lua_register(L, "API_PLAYER_GET_y", API_PLAYER_GET_y);
	}
	while (true) {
		event_queue_lock.lock();
		if (0 == event_que.size()) {
			event_queue_lock.unlock();
			continue;
		}
		auto k = event_que.top();
		event_queue_lock.unlock();
		
		
		if (k.GetEvent() == ENEMY_MOVE) {
			chrono::seconds sec(1);
			if (k.Gettime() < sec.count()){
				continue;
			}
			OverlapEx *overlap_ex = new OverlapEx;
			memset(overlap_ex, 0, sizeof(OverlapEx));
			overlap_ex->m_nOperation = OP_NPC_MOVE;
			PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
				reinterpret_cast<LPOVERLAPPED>(overlap_ex));
		}
		else if (k.GetEvent() == ENEMY_REGEN) {
			chrono::seconds sec(10);
			if (k.Gettime() < sec.count()) {
				continue;
			}
			OverlapEx *overlap_ex = new OverlapEx;
			memset(overlap_ex, 0, sizeof(OverlapEx));
			overlap_ex->m_nOperation = OP_NPC_REGEN;
			PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
				reinterpret_cast<LPOVERLAPPED>(overlap_ex));
		}
		else if (k.GetEvent() == ENEMY_ATTACK_MOVE) {
			chrono::seconds sec(1);
			if (k.Gettime() < sec.count()) {
				continue;
			}
			OverlapEx *overlap_ex = new OverlapEx;
			memset(overlap_ex, 0, sizeof(OverlapEx));
			overlap_ex->m_nOperation = OP_ENEMY_ATTACK_MOVE;
			PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
				reinterpret_cast<LPOVERLAPPED>(overlap_ex));
		}
		else if (k.GetEvent() == PLAYER_HP) {
			chrono::seconds sec(5);
			if (k.Gettime() < sec.count()) continue;
			OverlapEx *overlap_ex = new OverlapEx;
			memset(overlap_ex, 0, sizeof(OverlapEx));
			overlap_ex->m_nOperation = OP_PLAYER_HP_HEAL;
			PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
				reinterpret_cast<LPOVERLAPPED>(overlap_ex));
		}
		event_queue_lock.lock();
		event_que.pop();
		event_queue_lock.unlock();
	}
	
}
void WorkerThreadFunc() {
	DWORD io_size, key;
	OverlapEx *overlap;
	bool b_result;

	while (true) {
		b_result = GetQueuedCompletionStatus(g_hIocp, &io_size, &key, reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);
		if (!b_result) {
			//process error
		}
		if (0 == io_size) {
			cout << "pop player" << endl;
			// exit player process			
			closesocket(g_ServerFramework.GetPlayerInfo(key)->GetSocketInfo());
			SC_POP_PLAYER pop_player;
			pop_player.size = sizeof(SC_POP_PLAYER);
			pop_player.type = PLAYER_POP;
			pop_player.id = (WORD)key;
			for (auto i = 0; i < MAX_USER; ++i) {
				if (g_ServerFramework.GetPlayerInfo(i)->GetIsConnect() &&
					i != key) {
					g_ServerFramework.GetClientInfo(i)->lock();
					if (g_ServerFramework.GetClientInfo(i)->m_view_list.count(key) != 0) {
						g_ServerFramework.GetClientInfo(i)->m_view_list.erase(key);
						g_ServerFramework.GetClientInfo(i)->unlock();
						SendPacket(i, reinterpret_cast<unsigned char*>(&pop_player));
					}
					else g_ServerFramework.GetClientInfo(i)->unlock();
				}
			}
			g_ServerFramework.GetPlayerInfo(key)->SetIsConnected(false);
			continue;
		}
		
		PlayerInfo *clientinfo = g_ServerFramework.GetPlayerInfo(key);
			if (overlap->m_nOperation == OP_RECV) {
			unsigned char* buf_ptr = overlap->m_socket_buf;
			int remained = io_size;
			int required = 0;

			while (0 < remained) {
				if (0 == clientinfo->m_OverlappedEx.m_nCurrent_paket_size) {
					clientinfo->m_OverlappedEx.m_nCurrent_paket_size = buf_ptr[0];
				}
				required = clientinfo->m_OverlappedEx.m_nCurrent_paket_size - clientinfo->GetPreviousData();

				if (remained >= required) {
					memcpy(clientinfo->m_store_packet + clientinfo->GetPreviousData(), buf_ptr, required);
					ProcessPacket(key, buf_ptr);
					remained -= required;
					buf_ptr += required;
					clientinfo->m_OverlappedEx.m_nCurrent_paket_size = 0;
					clientinfo->SetPreviosData(0);
				}
				else {
					memcpy(clientinfo->m_store_packet + clientinfo->GetPreviousData(), buf_ptr, remained);
					remained = 0;
					buf_ptr += remained;
				}
			}
			DWORD flags = 0;
			WSARecv(clientinfo->GetSocketInfo(), &clientinfo->m_OverlappedEx.m_wsa_buf, 1,
				NULL, &flags, reinterpret_cast<LPOVERLAPPED>(&clientinfo->m_OverlappedEx), NULL);
		}
		else if (overlap->m_nOperation == OP_SEND) {
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_NPC_MOVE) {
			ProcessNPCPakcet(key, ENEMY_MOVE);
			delete overlap;
		}
		else if (OP_NPC_REGEN == overlap->m_nOperation) {
			ProcessNPCPakcet(key, ENEMY_REGEN);
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_EVENT_PLAYER_MOVE) {
			lua_State *L = g_ServerFramework.GetEnemyObject(key)->L;
			lua_getglobal(L, "event_player_move");
			lua_pushnumber(L, overlap->param1);
			lua_pcall(L, 1, 0, 0);
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_EVENT_ENEMY_MOVE) {
			lua_State *L = g_ServerFramework.GetEnemyObject(key)->L;
			lua_getglobal(L, "event_enemy_move");
			lua_pushnumber(L, overlap->param1);
			lua_pcall(L, 1, 0, 0);
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_ENEMY_ATTACK_MOVE) {
			ProcessASTAR(key);
		}
		else if (overlap->m_nOperation == OP_PLAYER_HP_HEAL) {
			HealingPlayeR(key);
		}
		else {
			cout << "UnKnown Event on worker Thread \n";
		}
	}
}
int main()
{

	// 서버 초기화
	g_SpacePartition = new CCollisionCheck*[2];

	g_SpacePartition[WORLD_MAP] = new CCollisionCheck();
	g_SpacePartition[WORLD_MAP]->Initialize_space_division(500, 3, 1);
	g_SpacePartition[WORLD_MAP]->BuildWorld();

	g_ServerFramework.SetMapDataInAstar(g_SpacePartition[0]);
	g_ServerFramework.InitializeServer();
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);


	std::vector<std::thread*> worker_threads;
	for (auto i = 0; i < NUM_THREADS; ++i) {
		worker_threads.push_back(new std::thread{ WorkerThreadFunc });
	}
	std::thread Accept = std::thread{ AcceptThread };
	thread Timer = thread{ TimerThread };

	while (1)
	{
		Sleep(1000);
	}

	for (auto th : worker_threads) {
		th->join();
		delete th;
	}

	Accept.join();
	Timer.join();

	g_ServerFramework.CleanUpServer();
}