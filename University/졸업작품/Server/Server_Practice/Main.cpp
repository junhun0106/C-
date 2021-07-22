//ksh
#define WIN32_LEAN_AND_MEAN  
#define INITGUID
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//ksh
#include "ServerFramework.h"

CServerFramework g_ServerFramework;
CSpacePartition g_SpaceParition;

HANDLE g_hIocp;
//Concurrency::concurrent_queue<Timee_Queue> event_que;
priority_queue<Timee_Queue> event_que;
mutex event_que_lock;
mutex g_mtx;


atomic<float> sunset_x = 10;
atomic<float> sunset_z = 10;
atomic<float> sunset_y = -10;

atomic<float> x_pos = 1;
atomic<float> y_pos = 1;
atomic<float> z_pos = -1;

//ksh
extern Party party[1000];
int party_count = 0;
//ksh

volatile int test_number = -1;
void ProcessPacket(int id, unsigned char* packet);
void ProcessNPCPakcet(int id, unsigned char type);

void err_display(char *msg, int err_no)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
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
	sc_packet_put_player packet;
	packet.id = player;
	packet.size = sizeof(sc_packet_put_player);
	packet.type = PLAYER_EVENT_PUT;
	packet.x = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetPosX();
	packet.y = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetPosY();
	packet.z = g_ServerFramework.GetClientInfo(player)->GetPlayerInfo()->GetPosZ();
	packet.look_x = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetLookX();
	packet.look_y = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetLookY();
	packet.look_z = g_ServerFramework.GetPlayerInfo(player)->GetPlayerInfo()->GetLookZ();

	SendPacket(client, reinterpret_cast<unsigned char*>(&packet));
}
void SendRemovePlayerPacket(int client, int player) {
	sc_packet_pop_player packet;
	packet.id = player;
	packet.size = sizeof(sc_packet_pop_player);
	packet.type = PLAYER_EVENT_POP;
	SendPacket(client, reinterpret_cast<unsigned char*>(&packet));
}
void SendMessagePacket(int player_id, wchar_t *text, int event_type) {
	sc_packet_chat_message packet;
	wcscpy_s(packet.message, text);
	packet.size = sizeof(sc_packet_chat_message);
	packet.type = event_type;
	SendPacket(player_id, reinterpret_cast<unsigned char*>(&packet));
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
		client_sock = WSAAccept(accept_socket, reinterpret_cast<SOCKADDR*>(&client_addr), &client_size, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			closesocket(client_sock);
		}
		for (int i = 0; i < MAX_USER; ++i) {
			if (!g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) {
				new_id = i;
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
		//kshdbdbdb
		cs_packet_db db_packet;
		//recv(client_sock, (char *)&db_packet, sizeof(db_packet), 0);	// 캐릭 기본위치 받기			
		//player->SetPosition(db_packet.xPos, 0.0f, db_packet.yPos);
		player->SetPosition(50.0f, 0.0f, 50.0f);
		//ksh
		//player->SetPosition(-10.0f, 0.0f, -10.0f);
		//2222222
		player->SetStatus(1, 0, 100);
		//2222222
		player->SetPlayerDamage(3);

		g_ServerFramework.GetPlayerInfo(new_id)->SetInfo(client_sock, player, new_id);
		g_ServerFramework.GetPlayerInfo(new_id)->SetIsConnected(true);
		_asm mfence;

		sc_packet_put_player put_player;
		put_player.id = new_id;
		put_player.size = sizeof(sc_packet_put_player);
		put_player.type = PLAYER_EVENT_PUT;
		PlayerInfo *info = g_ServerFramework.GetPlayerInfo(new_id);
		//ksh
		// 이름을 찾아서 저기다가 넣어야 한다.
		wcscpy_s(info->GetPlayerInfo()->GetName_W(), 50, db_packet.db_name);
		info->GetPlayerInfo()->SetPartyNumber(-1);	// 파티번호 초기화
		//ksh
		float x = info->GetPlayerInfo()->GetPosX(), y = info->GetPlayerInfo()->GetPosY();
		float z = info->GetPlayerInfo()->GetPosZ();
		put_player.x = x;
		put_player.y = y;
		put_player.z = z;
		put_player.look_x = info->GetPlayerInfo()->GetLookX();
		put_player.look_y = info->GetPlayerInfo()->GetLookY();
		put_player.look_z = info->GetPlayerInfo()->GetLookZ();


		sc_packet_player_level player_level;
		player_level.size = sizeof(sc_packet_player_level);
		player_level.type = PLAYER_EVENT_LEVEL_UP;
		player_level.id = new_id;
		player_level.level = 1;

		sc_packet_player_hp player_hp;
		player_hp.size = sizeof(sc_packet_player_hp);
		player_hp.type = PLAYER_EVENT_DECREASE_HP;
		player_hp.id = new_id;
		player_hp.hp = 100;

		sc_packet_player_exp player_exp;
		player_exp.size = sizeof(sc_packet_player_exp);
		player_exp.type = PLAYER_EVENT_GET_EXP;
		player_exp.id = new_id;
		player_exp.exp = 0;

		SendPacket(new_id, reinterpret_cast<unsigned char*>(&put_player));
		SendPacket(new_id, reinterpret_cast<unsigned char*>(&player_level));
		SendPacket(new_id, reinterpret_cast<unsigned char*>(&player_hp));
		SendPacket(new_id, reinterpret_cast<unsigned char*>(&player_exp));


		for (auto i = 0; i < MAX_USER; ++i) {// 접속해 있는 플레이어에게 나의 위치 전송
			if (i == new_id) continue;
			if (g_ServerFramework.GetPlayerInfo(i)->GetIsConnect() == false)  continue;
			if (false == g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->
				Distance(g_ServerFramework.GetPlayerInfo(new_id)->GetPlayerInfo())) continue;

			g_ServerFramework.GetPlayerInfo(i)->m_view_list.Add(new_id);
			SendPacket(i, reinterpret_cast<unsigned char*>(&put_player));
		}
		for (auto i = 0; i < MAX_USER; ++i) { // 나에게 접속해 있는 플레이어들 위치 전송
			if (g_ServerFramework.GetPlayerInfo(i)->GetIsConnect() == false) continue;
			if (i == new_id) continue;
			if (false == g_ServerFramework.GetPlayerInfo(new_id)->GetPlayerInfo()->
				Distance(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo())) continue;

			g_ServerFramework.GetPlayerInfo(new_id)->m_view_list.Add(i);
			put_player.id = i;
			put_player.x = g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetPosX();
			put_player.y = g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetPosY();
			put_player.z = g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->GetPosZ();
			put_player.look_x = g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->GetLookX();
			put_player.look_y = g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->GetLookY();
			put_player.look_z = g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->GetLookZ();
			SendPacket(new_id, reinterpret_cast<unsigned char*>(&put_player));
		}
		// 보이는 NPC 위치 전송
		for (auto i = 0; i < MAX_ENEMY; ++i) {
			if (false == g_ServerFramework.GetClientInfo(new_id)->GetPlayerInfo()->Distance(g_ServerFramework.GetEnemyObject(i))) continue;
			if (false == g_ServerFramework.GetEnemyObject(i)->GetHeartBeat()) {
				sc_packet_npc_view npc_view;
				npc_view.id = i;
				npc_view.size = sizeof(sc_packet_npc_view);
				npc_view.type = ENEMY_EVENT_DEATH;
				npc_view.view_state = false;

				SendPacket(new_id, reinterpret_cast<unsigned char*>(&npc_view));
				continue;
			}

			if (false == g_ServerFramework.GetClientInfo(new_id)->m_npc_view_list.Add(i)) cout << "npc도 안들어간다~~" << endl;

			sc_packet_npc_view npc_view;
			npc_view.id = i;
			npc_view.size = sizeof(sc_packet_npc_view);
			npc_view.type = ENEMY_EVENT_DEATH;
			npc_view.view_state = true;

			SendPacket(new_id, reinterpret_cast<unsigned char*>(&npc_view));

			
			npc_view.type = ENEMY_EVENT_VIEW_STATE;
			npc_view.view_state = true;

			sc_packet_npc_position npc_pos;
			npc_pos.id = i;
			npc_pos.size = sizeof(sc_packet_npc_position);
			npc_pos.type = ENEMY_EVENT_MOVE;
			npc_pos.x = g_ServerFramework.GetEnemyObject(i)->GetPosX();
			npc_pos.y = g_ServerFramework.GetEnemyObject(i)->GetPosY();
			npc_pos.z = g_ServerFramework.GetEnemyObject(i)->GetPosZ();

			sc_packet_enemy_hp enemy_hp;
			enemy_hp.size = sizeof(sc_packet_enemy_hp);
			enemy_hp.type = ENEMY_EVENT_HP;
			enemy_hp.id = i;
			enemy_hp.hp = g_ServerFramework.GetEnemyObject(i)->GetHP();

			SendPacket(new_id, reinterpret_cast<unsigned char*>(&enemy_hp));
			SendPacket(new_id, reinterpret_cast<unsigned char*>(&npc_view));
			SendPacket(new_id, reinterpret_cast<unsigned char*>(&npc_pos));
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
	CPlayer *player = g_ServerFramework.GetClientInfo(id)->GetPlayerInfo();
	unsigned char packet_type = packet[1];

	switch (packet_type) {
	case PLAYER_EVENT_ATTACK: {
		cs_packet_player_attack *buf = reinterpret_cast<cs_packet_player_attack*>(packet);
		int monster_id = buf->id;

		CPlayer *player = g_ServerFramework.GetClientInfo(id)->GetPlayerInfo();
		CDynamicObject *monster = g_ServerFramework.GetEnemyObject(monster_id);

		int player_damage = player->GetDamage();

		// 몬스터 사망
		if (true == monster->DecreaseHP(player_damage)) {
			ProcessNPCPakcet(monster_id, ENEMY_EVENT_DEATH);
			sc_packet_player_exp player_exp;

			int exp = monster->GetGiveEXP();
			// 레벨 업 
			if (true == player->UpExp(exp)) {
				sc_packet_player_level player_level;
				player_level.size = sizeof(sc_packet_player_level);
				player_level.type = PLAYER_EVENT_LEVEL_UP;
				player_level.level = player->GetLevel();
				player_level.id = id;
				
				cout << "Player Level Up!!\n";
				
				SendPacket(id, reinterpret_cast<unsigned char*>(&player_level));
			}

			player_exp.size = sizeof(sc_packet_player_exp);
			player_exp.type = PLAYER_EVENT_GET_EXP;
			player_exp.exp = player->GetTakeExp();
			player_exp.id = id;
			SendPacket(id, reinterpret_cast<unsigned char*>(&player_exp));
		}
		// 몬스터 hp 전송
		for (auto i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			// 거리 계산 false continue -> 몬스터 첫 대면시 hp 전송으로 수정
			sc_packet_enemy_hp enemy_hp;
			enemy_hp.size = sizeof(sc_packet_enemy_hp);
			enemy_hp.type = ENEMY_EVENT_HP;
			enemy_hp.id = monster_id;
			enemy_hp.hp = monster->GetHP();
			SendPacket(id, reinterpret_cast<unsigned char*>(&enemy_hp));
		}

	} break;
	case PLAYER_EVENT_DEATH: {

	} break;
	case PLAYER_EVENT_CHAT_MESSAGE:{
		for (auto i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			SendPacket(i, packet);
		}
	} break;
	case PLAYER_EVENT_POS: {
		float x = player->GetPosX();
		float y = player->GetPosY();
		float z = player->GetPosZ();
		float deltaX = 0, deltaY = 0, deltaZ = 0;
		cs_packet_player_state *buf = reinterpret_cast<cs_packet_player_state*>(packet);
		DWORD driection = buf->direction;
		float look_x = buf->look_x, look_y = buf->look_y, look_z = buf->look_z;
		float right_x = buf->right_x, right_y = buf->right_y, right_z = buf->right_z;
		if (driection & DIR_FORWARD)
		{
			deltaX += look_x;
			deltaY += look_y;
			deltaZ += look_z;
		}
		if (driection & DIR_BACKWARD) {
			deltaX -= look_x;
			deltaY -= look_y;
			deltaZ -= look_z;
		}
		if (driection & DIR_RIGHT) {
			deltaX += right_x;
			deltaY += right_y;
			deltaZ += right_z;
		}
		if (driection & DIR_LEFT) {
			deltaX -= right_x;
			deltaY -= right_y;
			deltaZ -= right_z;
		}
		deltaX *= 0.3; deltaY *= 0.3; deltaZ *= 0.3f;
		x += deltaX, y += deltaY, z += deltaZ;
		if (g_SpaceParition.Check_Collision(x, y, z, 0.2f)) {
			x -= deltaX, y -= deltaY, z -= deltaZ;
			deltaX = deltaZ = 0;

		}
		if (x < 0 || z < 0 || x > 2000 || z > 2000) {
			x -= deltaX, y -= deltaY, z -= deltaZ;
			deltaX = deltaZ = 0;
		}
		player->SetPosition(x, y, z);
		player->SetLookVector(look_x, look_y, look_z);

		sc_packet_player_position player_pos;
		player_pos.size = sizeof(sc_packet_player_position);
		player_pos.type = PLAYER_EVENT_POS;
		player_pos.id = id;
		player_pos.xPos = x;
		player_pos.yPos = y;
		player_pos.zPos = z;
		player_pos.x_lock = look_x;
		player_pos.y_look = look_y;
		player_pos.z_lock = look_z;

		SendPacket(id, reinterpret_cast<unsigned char*>(&player_pos));

		// new view list process
		set<int> new_view_list;
		for (auto i = 0; i < MAX_USER; ++i) {
			if (i == id) continue;
			if (g_ServerFramework.GetClientInfo(i)->GetIsConnect() == false) continue;
			if (player-> Distance(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()))
				new_view_list.insert(i);
		}

		// put player view process
		for (auto i : new_view_list) {
			bool new_one = (false == clientinfo->m_view_list.Contains(i));

			if (new_one)
			{
				clientinfo->m_view_list.Add(i);
				SendPutPlayerPacket(id, i);
			}
			if (false == g_ServerFramework.GetClientInfo(i)->m_view_list.Contains(id)) {
				g_ServerFramework.GetClientInfo(i)->m_view_list.Add(id);
				SendPutPlayerPacket(i, id);
			}
			else {
				SendPacket(i, reinterpret_cast<unsigned char*>(&player_pos));
			}
		}
		// remove player view process
		for (auto i : new_view_list) {
			if (true == clientinfo->m_view_list.Contains(i)) continue;
			clientinfo->m_view_list.Remove(i);
			SendRemovePlayerPacket(id, i);
			PlayerInfo *other_player = g_ServerFramework.GetClientInfo(i);
			if (true == other_player->m_view_list.Contains(id)) continue;
			other_player->m_view_list.Remove(id);
			SendRemovePlayerPacket(i, id);
		} 

		// enemy process
		for (auto i = 0; i < MAX_ENEMY; ++i) {
			CDynamicObject *monster = g_ServerFramework.GetEnemyObject(i);
			if (false == monster->GetHeartBeat()) continue;
			if (true == player->Distance(monster)) {
				if (false == clientinfo->m_npc_view_list.Contains(i))
				{
					if (false == clientinfo->m_npc_view_list.Add(i)) {
						cout << i << " monster error" << endl;
					}
					sc_packet_npc_view npc_view;
					npc_view.id = i;
					npc_view.size = sizeof(sc_packet_npc_view);
					npc_view.type = ENEMY_EVENT_VIEW_STATE;
					npc_view.view_state = true;

					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_view));

					sc_packet_npc_position npc_pos;
					npc_pos.id = i;
					npc_pos.size = sizeof(sc_packet_npc_position);
					npc_pos.type = ENEMY_EVENT_MOVE;
					npc_pos.x = monster->GetPosX();
					npc_pos.y = monster->GetPosY();
					npc_pos.z = monster->GetPosZ();
					npc_pos.look_x = monster->GetLookX();
					npc_pos.look_y = monster->GetLookY();
					npc_pos.look_z = monster->GetLookZ();

					SendPacket(id, reinterpret_cast<unsigned char*>(&npc_pos));

					sc_packet_enemy_hp enemy_hp;
					enemy_hp.size = sizeof(sc_packet_enemy_hp);
					enemy_hp.type = ENEMY_EVENT_HP;
					enemy_hp.id = i;
					enemy_hp.hp = g_ServerFramework.GetEnemyObject(i)->GetHP();
					SendPacket(id, reinterpret_cast<unsigned char*>(&enemy_hp));

					if (true == monster->GetTargetMode()) continue;
					if (true == monster->GetMoveMode()) continue;
						 
					monster->SetMoveMode(true);

					Timee_Queue enemy;
					enemy.add_timer(i, ENEMY_EVENT_MOVE);
					event_que_lock.lock();
					event_que.push(enemy);
					event_que_lock.unlock();

				}
				continue;
			}
		}
	} break;
	case PLAYER_ANIMATE_STATE: {
		cs_packet_charater_animate *buf_ptr = reinterpret_cast<cs_packet_charater_animate*>(packet);
		int anim_state = buf_ptr->anim_state;

		sc_packet_player_anim player_anim;
		player_anim.anim_state = anim_state;
		player_anim.id = id;
		player_anim.size = sizeof(sc_packet_player_anim);
		player_anim.type = PLAYER_ANIMATE_STATE;

		auto begin = g_ServerFramework.GetClientInfo(id)->m_view_list.begin()->next[0];
		auto end = g_ServerFramework.GetClientInfo(id)->m_view_list.end();
		//cout << id << " client do " << anim_state << endl;
		while (begin != end) {
			int other_id = (*begin).key;
			SendPacket(other_id, reinterpret_cast<unsigned char*>(&player_anim));
			begin = (*begin).next[0];
		}

	} break;
		//ksh
	case 88:
	{
		WCHAR join_name[50];// 클라에서 서버로 받은 패킷안에 JOIN NAME 저장하는 곳
		WCHAR invite_name[50];
		cs_packet_party *cs = reinterpret_cast<cs_packet_party *>(packet);

		int join_id;
		int invite_id;

		if (cs->bool_invite == true && cs->bool_join == false) {	//초대 할때
			wcscpy_s(join_name, 50, cs->join_name);
			wcscpy_s(invite_name, 50, g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W());
		}
		if (cs->bool_invite == false && cs->bool_join == true) {	//파티 할떄
			wcscpy_s(join_name, 50, g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W());
			wcscpy_s(invite_name, 50, cs->invite_name);
		}
		if (cs->bool_invite == true && cs->bool_join == true) {		//거절 할때.
			wcscpy_s(join_name, 50, g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W());
			wcscpy_s(invite_name, 50, cs->invite_name);
		}

		//이름 없으면 걍 꺼져라
		for (int i = 0; i < MAX_USER; i++) {
			if (false == wcscmp(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetName_W(), join_name))
				if (false == wcscmp(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetName_W(), invite_name))
					break;
		}

		for (int i = 0; i < MAX_USER; i++) {
			if (false == g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			if (false == wcscmp(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetName_W(), join_name))
				join_id = i;
		}

		for (int i = 0; i < MAX_USER; i++) {
			if (false == g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			if (false == wcscmp(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetName_W(), invite_name))
				invite_id = i;
		}


		// 
		// 1. invite 아이디가 join 아이디에게 초대 보내기
		//
		if (cs->bool_invite == true && cs->bool_join == false) {
			wcscpy_s(invite_name, 50, g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W());

			for (int i = 0; i < MAX_USER; i++) {
				if (false == g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) continue;	//접속 아니면 꺼지고
				if (false == wcscmp(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetName_W(), join_name)) {
					sc_packet_party_player packet;

					packet.size = sizeof(packet);
					packet.type = 87;// SC_PARTY_PLAYER;
					wcscpy_s(packet.invite_name, invite_name);
					wcscpy_s(packet.join_name, join_name);
					packet.bool_invite = true;	//초대 메시지다
					packet.bool_party_success = false;	//성공은 아니다.	// 이건 아직 상관없다.

					SendPacket(join_id, reinterpret_cast<unsigned char *>(&packet));
				}
			}
		}

		//파티수락창 거절했을때 보내는거다.
		//ksh6
		if (cs->bool_invite == true && cs->bool_join == true) {
			wcscpy_s(invite_name, 50, g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W());

			sc_packet_chat_message *packet = reinterpret_cast<sc_packet_chat_message *>(&packet);
			packet->size = sizeof(sc_packet_chat_message);
			packet->type = PLAYER_PARTY_CHAT_MESSAGE;
			wstring str = L"[시스템] 파티가 거절되었습니다.";
			wcsncpy_s(packet->message, str.c_str(), MAX_STR_SIZE);

			SendPacket(invite_id, reinterpret_cast<unsigned char *>(&packet));
			// invite_id 에게 이 패킷은 메시지 패킷으로 처리 할거다.

		}
		//ksh6
		// 
		// 2. 파티하기
		//
		// 파티 완료 // 만약 파티가 있는 파티원이 초대 했으면 그 파티에 들어가야 한다.
		if (cs->bool_invite == false && cs->bool_join == true) {

			//순서를 바꾸자
			// 1. 조인유저가 파티가 되어 있는지 확인하고.
			// 1-1. 파티가 되어있으면, ㅈㅈ
			if (g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo()->GetPartyNumber() != -1)
				goto immi;

			// 4?. 인바이트 유저가 파티가 있으면, 파티번호를 받아오고
			// 5. 조인유저가 인바이트 유저파티에 없는 자리에 차지한다.
			// 6. 자리가 없으면, ㅈㅈ
			if (g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo()->GetPartyNumber() != -1)
			{
				int exist_party_number = g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo()->GetPartyNumber();

				for (int j = 0; j < 4; ++j) {
					if (party[exist_party_number].client[j] == NULL) {
						party[exist_party_number].client[j] = g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo();
						g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo()->SetPartyNumber(exist_party_number);
						party[exist_party_number].mm_count++;
						break;
					}
				}

				//원래 있는 파티에 들어가서 정보 넘기기
				for (int i = 0; i < 4; ++i) {
					for (int j = 0; j < 4; ++j) {
						if (party[exist_party_number].client[i] != NULL && party[exist_party_number].client[j] != NULL) {
							int s_id, r_id;
							// 여기 개 줄일수 있는데인데 하...
							for (int k = 0; k < MAX_USER; k++) {
								if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
								if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exist_party_number].client[i]->GetName_W()))
									s_id = k;
							}
							for (int k = 0; k < MAX_USER; k++) {
								if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
								if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exist_party_number].client[j]->GetName_W()))
									r_id = k;
							}
							sc_packet_party_set party_set;

							party_set.size = sizeof(sc_packet_party_set);
							party_set.type = 85;
							party_set.xPos = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetPosX();
							party_set.yPos = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetPosZ();
							party_set.hp = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetHP();
							party_set.m_count = party[exist_party_number].mm_count;
							wcscpy_s(party_set.member_name, 50, g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetName_W());
							if (wcscmp(g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W(), g_ServerFramework.GetPlayerInfo(r_id)->GetPlayerInfo()->GetName_W())) {
								SendPacket(r_id, reinterpret_cast<unsigned char*>(&party_set));
							}
						}
					}
				}
				goto immi;
			}
			// 2. 인바이트 유저가 파티가 없으면, 인바이트유저가 파티번호 만들고. 파티를 만들고, 0번 자리 차지한다.
			// 3. 조인유저가 인바이트유저 파티에 1번 자리를 차지한다.

			int new_party = -1;	// 파티 넘버 초기화
			for (auto i = 0; i < MAX_USER; ++i) {
				if (party[i].is == false) {
					new_party = i;
					party[new_party].is = true;	// 파티가 있다.
					party_count++;
					break;
				}
			}

			g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo()->SetPartyNumber(new_party);
			g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo()->SetPartyNumber(new_party);
			party[new_party].client[0] = g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo();
			party[new_party].mm_count++;
			party[new_party].client[1] = g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo();
			party[new_party].mm_count++;


			// 파티가 되면, 정보를 쏴줘야 한다!! 
			// join_id정보 > invite_id
			sc_packet_party_set party_set;

			party_set.size = sizeof(sc_packet_party_set);
			party_set.type = 85;
			party_set.xPos = g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo()->GetPosX();
			party_set.yPos = g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo()->GetPosZ();
			party_set.hp = g_ServerFramework.GetPlayerInfo(join_id)->GetPlayerInfo()->GetHP();
			party_set.m_count = party[new_party].mm_count;
			wcscpy_s(party_set.member_name, 50, join_name);

			SendPacket(invite_id, reinterpret_cast<unsigned char*>(&party_set));

			// join_id정보 > invite_id
			party_set.size = sizeof(sc_packet_party_set);
			party_set.type = 85;
			party_set.xPos = g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo()->GetPosX();
			party_set.yPos = g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo()->GetPosZ();
			party_set.hp = g_ServerFramework.GetPlayerInfo(invite_id)->GetPlayerInfo()->GetHP();
			party_set.m_count = party[new_party].mm_count;
			wcscpy_s(party_set.member_name, 50, invite_name);

			SendPacket(join_id, reinterpret_cast<unsigned char*>(&party_set));
		}
	immi:

		// 
		// 3. 접속 유저가 파티 탈퇴하기
		//
		if (cs->bool_invite == false && cs->bool_join == false) 
		{

			//ksh7

			//int exitp = g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetPartyNumber(); 	// 여기부터 파티탈퇴 처리하자.	

			//if (exitp != -1) {				// 파티가 끝났다는 정보를 쏴줘야 한다. 
			//	for (int i = 0; i < 4; ++i)
			//	{
			//		for (int j = 0; j < 4; ++j)
			//		{
			//			if (party[exitp].client[i] != NULL && party[exitp].client[j] != NULL)
			//			{
			//				int s_id, r_id;
			//				// 여기 개 줄일수 있는데인데 하...
			//				for (int k = 0; k < MAX_USER; k++)
			//				{
			//					if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			//					if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exitp].client[i]->GetName_W()))
			//						s_id = k;
			//				}
			//				for (int k = 0; k < MAX_USER; k++)
			//				{
			//					if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			//					if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exitp].client[j]->GetName_W()))
			//						r_id = k;
			//				}
			//				sc_packet_party_set party_set;

			//				party_set.size = sizeof(sc_packet_party_set);
			//				party_set.type = 85;
			//				party_set.xPos = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetPosX();
			//				party_set.yPos = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetPosZ();
			//				party_set.hp = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetHP();
			//				party_set.m_count = party[exitp].mm_count;
			//				wcscpy_s(party_set.member_name, 50, g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetName_W());

			//				if (true == g_ServerFramework.GetPlayerInfo(r_id)->GetIsConnect() || wcscmp(g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W(), g_ServerFramework.GetPlayerInfo(r_id)->GetPlayerInfo()->GetName_W()))
			//				{
			//					SendPacket(r_id, reinterpret_cast<unsigned char*>(&party_set));
			//				}
			//			}
			//		}
			//	}


			//	// 모두 나가면 파티 bool 값 false
			//	if (party[exitp].client[0] == NULL && party[exitp].client[1] == NULL && party[exitp].client[2] == NULL && party[exitp].client[3] == NULL)
			//	{
			//		party[exitp].is = false;
			//		party_count--;
			//	}

				// 여기서 또 처리 해줘야 하는데 						

				//for (int i = 0; i < 4; ++i) {
				//	if (wcscmp(party[exitp].client[i]->GetName_W(), g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W())) {// 파티원들 i 알아내기
				//		/////////??????????????????????????????????????????????????????????????? 
				//		int e_id;
				//		// 여기 개 줄일수 있는데인데 하...
				//		for (int k = 0; k < MAX_USER; k++)
				//		{
				//			if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
				//			if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exitp].client[i]->GetName_W()))
				//				e_id = k;
				//		}
				//		sc_packet_party_set party_set;

				//		party_set.size = sizeof(sc_packet_party_set);
				//		party_set.type = 76;
				//		party_set.xPos = 0;
				//		party_set.yPos = 0;
				//		party_set.hp = 0;
				//		party_set.m_count = 0;
				//		wcscpy_s(party_set.member_name, 50, g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W());

				//		if (true == g_ServerFramework.GetPlayerInfo(e_id)->GetIsConnect())
				//		{
				//			SendPacket(e_id, reinterpret_cast<unsigned char*>(&party_set));
				//		}

				//	}
				//}

				//for (int j = 0; j < 4; j++)
				//{
				//	if (party[exitp].client[j] != NULL)
				//		if (!wcscmp(party[exitp].client[j]->GetName_W(), g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W()))
				//		{//key,j 같으면 그 파티원 파티 탈퇴시키고 
				//			party[exitp].client[j] = NULL;
				//			party[exitp].mm_count--;
				//			g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->SetPartyNumber(-1);
				//		}
				//}
			//}
			//ksh7
		}
	}
	break;
	//ksh
	default:
		cout << " Unknown Player Packet Type" << endl;
		break;
	}
	//ksh
	//파티원들에게 자신 정보 갱신해서 보내기
	int exist_party_number = g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetPartyNumber();;	// 이 클라에 파티번호 저장하고
	if (exist_party_number != -1) {
		for (int j = 0; j < 4; ++j)
			if (party[exist_party_number].client[j] != NULL && wcscmp(party[exist_party_number].client[j]->GetName_W(), g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W())) {	// 그 파티에서 널이 아닌 파티원들에게 정보를 보내는데 ... 
																																																// 파티번호, 파티원 이름 좌표 hp 쏴주자.
				sc_packet_party_info pos_packet;
				pos_packet.size = sizeof(sc_packet_party_info);
				pos_packet.type = 86;
				pos_packet.xPos = g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetPosX();
				pos_packet.yPos = g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetPosZ();
				pos_packet.plevel = g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetLevel();
				pos_packet.hp = 0;
				wcscpy_s(pos_packet.member_name, 50, g_ServerFramework.GetPlayerInfo(id)->GetPlayerInfo()->GetName_W());

				int p_id;
				for (int i = 0; i < MAX_USER; i++) {
					if (false == g_ServerFramework.GetPlayerInfo(i)->GetIsConnect()) continue;	//접속 아니면 꺼지고
					if (false == wcscmp(g_ServerFramework.GetPlayerInfo(i)->GetPlayerInfo()->GetName_W(), party[exist_party_number].client[j]->GetName_W()))
						p_id = i;
				}

				SendPacket(p_id, reinterpret_cast<unsigned char*>(&pos_packet));
			}
	}
	//ksh
}
void ProcessNPCPakcet(int id, unsigned char type) {
	switch (type) {
	case ENEMY_EVENT_MOVE: {
		if (true == g_ServerFramework.GetEnemyObject(id)->GetTargetMode()) break;
		CDynamicObject *monster = g_ServerFramework.GetEnemyObject(id);
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			if (false == g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->Distance(g_ServerFramework.GetEnemyObject(id))) {
				if (g_ServerFramework.GetClientInfo(i)->m_npc_view_list.Contains(id)) 
					g_ServerFramework.GetClientInfo(i)->m_npc_view_list.Remove(id);
				g_ServerFramework.GetEnemyObject(id)->SetMoveMode(false);
				sc_packet_npc_view npc_view;
				npc_view.id = id;
				npc_view.size = sizeof(sc_packet_npc_view);
				npc_view.type = ENEMY_EVENT_VIEW_STATE;
				npc_view.view_state = false;
				SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
				continue;
			}
			if (g_ServerFramework.GetClientInfo(i)->GetPlayerInfo()->Astar_Distance(monster))
			{
				monster->SetTargetMode(true);
				monster->SetMoveMode(false);
				monster->SetTargetID(i);
				OverlapEx *overlap_ex = new OverlapEx;
				memset(overlap_ex, 0, sizeof(OverlapEx));
				overlap_ex->m_nOperation = OP_NPC_ASTAR;
				PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), id,
					reinterpret_cast<LPOVERLAPPED>(overlap_ex));
				continue;
			}

			float x, y, z;
			x = monster->GetPosX();
			y = monster->GetPosY();
			z = monster->GetPosZ();

			monster->Enemy_Move();

			float next_x, next_y, next_z, look_x, look_y, look_z;

			next_x = monster->GetPosX();
			next_y = monster->GetPosY();
			next_z = monster->GetPosZ();

			look_x = monster->GetLookX();
			look_y = monster->GetLookY();
			look_z = monster->GetLookZ();

			if (g_SpaceParition.Check_Collision(next_x, next_y, next_z, 0.5f)) {
				next_x = x;
				next_y = y;
				next_z = z;
			}
			//
			if (next_x < 0 || next_z < 0 || next_x > 2000 || next_z > 2000) {
				next_x = x;
				next_y = y;
				next_z = z;
			}

			sc_packet_npc_position npc_pos;
			npc_pos.id = id;
			npc_pos.size = sizeof(sc_packet_npc_position);
			npc_pos.type = ENEMY_EVENT_MOVE;
			npc_pos.x = next_x;
			npc_pos.y = next_y;
			npc_pos.z = next_z;
			npc_pos.look_x = look_x;
			npc_pos.look_y = look_y;
			npc_pos.look_z = look_z;

			SendPacket(i, reinterpret_cast<unsigned char*>(&npc_pos));
		}
		Timee_Queue enemy;
		enemy.add_timer(id, ENEMY_EVENT_MOVE);
		event_que_lock.lock();
		event_que.push(enemy);
		event_que_lock.unlock();
	}
		break;
		///// 8월 15일 수정 
	case ENEMY_EVENT_DEATH: {
		sc_packet_npc_view npc_view;
		int npc_id = id;
		npc_view.size = sizeof(sc_packet_npc_view);
		npc_view.type = ENEMY_EVENT_DEATH;
		npc_view.view_state = false;
		npc_view.id = npc_id;
		CDynamicObject *monster = g_ServerFramework.GetEnemyObject(npc_id);

		monster->SetHeartBeat(false);
		monster->SetMoveMode(false);
		monster->SetTargetMode(false);

		for (int i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			//if (i == id) continue;
			if (g_ServerFramework.GetClientInfo(i)->m_npc_view_list.Contains(npc_id)) {
				g_ServerFramework.GetClientInfo(i)->m_npc_view_list.Remove(npc_id);
			}
			SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
		}
		Timee_Queue enemy;
		enemy.add_timer(npc_id, ENEMY_EVENT_HEART_BEAT);
		event_que_lock.lock();
		event_que.push(enemy);
		event_que_lock.unlock();
	}break;
	case ENEMY_EVENT_HEART_BEAT: {
		CDynamicObject *monster = g_ServerFramework.GetEnemyObject(id);
		if (false == monster->GetHeartBeat()) {
			monster->SetHeartBeat(true);
			monster->SetTargetMode(false);
			monster->SetMoveMode(false);
			monster->Reset();
			for (int i = 0; i < MAX_USER; ++i) {
				if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
				sc_packet_npc_view npc_view;
				npc_view.id = id;
				npc_view.size = sizeof(sc_packet_npc_view);
				npc_view.type = ENEMY_EVENT_DEATH;
				npc_view.view_state = true;
				
				sc_packet_npc_position npc_pos;
				npc_pos.id = id;
				npc_pos.size = sizeof(sc_packet_npc_position);
				npc_pos.type = ENEMY_EVENT_MOVE;
				npc_pos.x = monster->GetPosX();
				npc_pos.y = monster->GetPosY();
				npc_pos.z = monster->GetPosZ();
				npc_pos.look_x = monster->GetLookX();
				npc_pos.look_y = monster->GetLookY();
				npc_pos.look_z = monster->GetLookZ();

				SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
				// 플레이어에 처음 보는 것이라면
				if (false == g_ServerFramework.GetClientInfo(i)->m_npc_view_list.Contains(id)) {
					g_ServerFramework.GetClientInfo(i)->m_npc_view_list.Add(id);
					npc_view.type = ENEMY_EVENT_VIEW_STATE;
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_view));
					SendPacket(i, reinterpret_cast<unsigned char*>(&npc_pos));

					monster->SetMoveMode(true);

					Timee_Queue enemy;
					enemy.add_timer(id, ENEMY_EVENT_MOVE);
					event_que_lock.lock();
					event_que.push(enemy);
					event_que_lock.unlock();
				}

			}
		}
	}
		break;
	case ENEMY_EVENT_ASTAR: {
		if (g_ServerFramework.GetEnemyObject(id)->GetTargetMode() == false) break;
		if (g_ServerFramework.GetEnemyObject(id)->GetHeartBeat() == false) break;
		if (g_ServerFramework.GetEnemyObject(id)->GetTargetID() < 0) break;
		if (false == g_ServerFramework.GetClientInfo(g_ServerFramework.GetEnemyObject(id)->GetTargetID())->GetIsConnect()) {
			CDynamicObject *monster = g_ServerFramework.GetEnemyObject(id);
			monster->SetTargetID(-1);
			monster->SetTargetMode(false);
			monster->SetMoveMode(true);
			monster->Reset();

			Timee_Queue enemy;
			enemy.add_timer(id, ENEMY_EVENT_MOVE);
			event_que_lock.lock();
			event_que.push(enemy);
			event_que_lock.unlock();
		}
		CDynamicObject *monster = g_ServerFramework.GetEnemyObject(id);
		int target_id = monster->GetTargetID();
		if (target_id == -1) break;
		CPlayer *player = g_ServerFramework.GetClientInfo(target_id)->GetPlayerInfo();

		if (false == player->Astar_Distance(monster)) {
			monster->SetTargetID(-1);
			monster->SetTargetMode(false);
			monster->SetMoveMode(true);
			monster->Reset();

			Timee_Queue enemy;
			enemy.add_timer(id, ENEMY_EVENT_MOVE);
			event_que_lock.lock();
			event_que.push(enemy);
			event_que_lock.unlock();
		}
		else {
			if (false == player->attack_area(monster)) {
				int mons_x = monster->GetPosX();
				int mons_y = monster->GetPosZ();
				PATHNODE* node = new PATHNODE(mons_x, mons_y, PATH);
				int target_x = player->GetPosX();
				int target_y = player->GetPosZ();
				pair<int, int> position = g_ServerFramework.StartAstar(node, target_x, target_y);
				delete node;
				monster->SetPosition(position.first, 0.0f, position.second);
				int normal_x = player->GetPosX() - monster->GetPosX();
				int normal_y = player->GetPosY() - monster->GetPosY();
				int normal_z = player->GetPosZ() - monster->GetPosZ();

				sc_packet_npc_position npc_pos;
				npc_pos.id = id;
				npc_pos.size = sizeof(sc_packet_npc_position);
				npc_pos.type = ENEMY_EVENT_MOVE;
				npc_pos.x = position.first;
				npc_pos.z = position.second;
				npc_pos.y = monster->GetPosY();
				npc_pos.look_x = normal_x;
				npc_pos.look_y = normal_y;
				npc_pos.look_z = normal_z;

				monster->SetLookVector(normal_x, normal_y, normal_z);

				sc_packet_npc_view npc_view;
				npc_view.id = id;
				npc_view.size = sizeof(sc_packet_npc_view);
				npc_view.type = ENEMY_EVENT_VIEW_STATE;
				npc_view.view_state = true;

				for (auto i = 0; i < MAX_USER; ++i) {
					if (g_ServerFramework.GetClientInfo(i)->GetIsConnect()) {
						SendPacket(target_id, reinterpret_cast<unsigned char*>(&npc_pos));
						SendPacket(target_id, reinterpret_cast<unsigned char*>(&npc_view));
					}
				}
			}
			// 공격
			else
			{
				// 플레이어 Hp 감소
				int damage = monster->GetDamage();
				// 플레이어 죽음
				if (true == player->DecreaseHP(damage)) {
					sc_packet_player_death player_death;
					player_death.size = sizeof(sc_packet_player_death);
					player_death.type = PLAYER_EVENT_DEATH;
					player_death.state = false;
					player_death.id = target_id;

					player->SetPosition(50, 0, 50);

					sc_packet_player_position player_position;
					player_position.id = target_id;
					player_position.size = sizeof(sc_packet_player_position);
					player_position.type = PLAYER_EVENT_POS;
					player_position.xPos = 0;
					player_position.yPos = 0;
					player_position.zPos = 0;
					player_position.x_lock = 0;
					player_position.y_look = 0;
					player_position.z_lock = 0;

					SendPacket(target_id, reinterpret_cast<unsigned char*>(&player_death));
					SendPacket(target_id, reinterpret_cast<unsigned char*>(&player_position));
				
				}
				// 플레이어 hp 전송
				int hp	=	player->GetHP();
				sc_packet_player_hp player_hp;
				player_hp.hp = hp;
				player_hp.id = target_id;
				player_hp.size = sizeof(sc_packet_player_hp);
				player_hp.type = PLAYER_EVENT_DECREASE_HP;
				SendPacket(target_id, reinterpret_cast<unsigned char*>(&player_hp));

				// 공격 애니메이션
				sc_packet_enemy_animate enemy_anim;
				enemy_anim.anim = 1; // enemy attack;
				enemy_anim.id = id;
				enemy_anim.size = sizeof(sc_packet_enemy_animate);
				enemy_anim.type = ENEMY_EVENT_ATTACK;

				SendPacket(target_id, reinterpret_cast<unsigned char*>(&enemy_anim));
			}
			Timee_Queue enemy;
			enemy.add_timer(id, ENEMY_EVENT_ASTAR);
			event_que_lock.lock();
			event_que.push(enemy);
			event_que_lock.unlock();
		}
	}
		break;
	default:
		cout << id << " NPC UnKnown Type" << endl;
		break;
	}
}
void ProcessSunset() {

	int x = 1;
	if (sunset_x == 10) x_pos = -0.01;
	else if (sunset_x == -10) x_pos = 0.01;
	sunset_x = sunset_x + x_pos;

	if (sunset_y == -10) y_pos = 0.01;
	else if (sunset_y == 0) y_pos = -0.01;
	sunset_y = sunset_y + y_pos;

	sc_packet_sunset_position packet;
	packet.size = sizeof(sc_packet_sunset_position);
	packet.type = SUNSET_EVENT;
	packet.x = sunset_x;
	packet.y = sunset_y;
	packet.z = sunset_z;

	for (auto i = 0; i < MAX_USER; ++i) {
		if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
		SendPacket(i, reinterpret_cast<unsigned char*>(&packet));
	}

	Timee_Queue timee;
	timee.add_timer(0, SUNSET_EVENT);
	event_que_lock.lock();
	event_que.push(timee);
	event_que_lock.unlock();
}
void TimerThread() {
	// NPC_CREATE
	for (auto i = 0; i < MAX_ENEMY; ++i) {
		g_ServerFramework.GetEnemyObject(i)->SetNPCID(i);
	}
	
	Timee_Queue enemy;
	enemy.add_timer(0, SUNSET_EVENT);
	event_que.push(enemy);

	while (true) {
		while (true) {
			event_que_lock.lock();
			if (0 == event_que.size()) {
				event_que_lock.unlock();
				continue;	
			}
			auto k = event_que.top();

			event_que_lock.unlock();
			if (k.GetEvent() == ENEMY_EVENT_MOVE) {
				chrono::seconds sec(1);
				if (k.Gettime() <= sec.count()) {
					continue;
				}
				OverlapEx *overlap_ex = new OverlapEx;
				memset(overlap_ex, 0, sizeof(OverlapEx));
				overlap_ex->m_nOperation = OP_NPC_MOVE;
				PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
					reinterpret_cast<LPOVERLAPPED>(overlap_ex));
			}
			else if (k.GetEvent() == ENEMY_EVENT_HEART_BEAT) {
				chrono::seconds sec(10);
				if (k.Gettime() <= sec.count())
				{				
					event_que_lock.lock();
					event_que.pop();
					event_que.push(k);
					event_que_lock.unlock();
					continue;
				}
				OverlapEx *overlap_ex = new OverlapEx;
				memset(overlap_ex, 0, sizeof(OverlapEx));
				overlap_ex->m_nOperation = OP_NPC_HEATBEAT;
				int key = k.GetNPCID();

				PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), key,
					reinterpret_cast<LPOVERLAPPED>(overlap_ex));
			}
			else if (k.GetEvent() == ENEMY_EVENT_ASTAR) {
				chrono::seconds sec(1);
				if (k.Gettime() <= sec.count()) {
					continue;
				}
				OverlapEx *overlap_ex = new OverlapEx;
				memset(overlap_ex, 0, sizeof(OverlapEx));
				overlap_ex->m_nOperation = OP_NPC_ASTAR;
				PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
					reinterpret_cast<LPOVERLAPPED>(overlap_ex));
			}
			else if (k.GetEvent() == SUNSET_EVENT) {
				chrono::seconds sec(1);
				if (k.Gettime() <= sec.count()) continue;
				OverlapEx *overlap_ex = new OverlapEx;
				memset(overlap_ex, 0, sizeof(OverlapEx));
				overlap_ex->m_nOperation = OP_SUNSET;
				PostQueuedCompletionStatus(g_hIocp, sizeof(overlap_ex), k.GetNPCID(),
					reinterpret_cast<LPOVERLAPPED>(overlap_ex));
			}
			else {
				cout << "UnKnown Type in Timer Queue" << endl;
			}
			event_que_lock.lock();
			event_que.pop();
			event_que_lock.unlock();
		}
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

			////ksh7
			//SOCKET login_socket;

			//WSABUF	send_wsabuf;
			//char 	send_buffer[1024];

			//send_wsabuf.buf = send_buffer;
			//send_wsabuf.len = 1024;

			//login_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);	// 소켓만들고

			//SOCKADDR_IN ServerAddr;
			//ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
			//ServerAddr.sin_family = AF_INET;
			//ServerAddr.sin_port = htons(8000);
			//ServerAddr.sin_addr.s_addr = inet_addr("192.168.43.197");

			//int Result = WSAConnect(login_socket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

			//cs_packet_db *my_packet = reinterpret_cast<cs_packet_db *>(send_buffer);	// 키가 눌러졌으면 패킷을 정리하고
			//my_packet->size = sizeof(cs_packet_db);
			//send_wsabuf.len = sizeof(cs_packet_db);
			//DWORD iobyte;

			//my_packet->type = 2;	// 타입만 바꿔서 보내면 된다.
			//my_packet->pexp = g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetTakeExp(); // 
			//my_packet->plevel = g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetLevel();;// 
			//my_packet->xPos = g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetPosX();;
			//my_packet->yPos = g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetPosZ();;
			////2222222
			//my_packet->php = g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetHP();// 
			////2222222
			//my_packet->pclass = 0;	// 클래스
			//my_packet->pselect = 0;	// 캐릭터 선택했는지
			//my_packet->pmodel = 0;	// 모델이 안쓸거 같고.

			//my_packet->pconnect = 0;
			//wcscpy_s(my_packet->db_name, 50, g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W());
			//int ret = WSASend(login_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);	// 아까 만든 소켓에다가 보낸다. 1=버퍼는 1개다, 버퍼의크기는 my_packet사이즈,	
			//if (ret) {
			//	int error_code = WSAGetLastError();
			//	printf("Error while sending packet [%d]", error_code);
			//}

			//ksh7
			//int exitp = g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetPartyNumber(); 	// 여기부터 파티탈퇴 처리하자.	
			//
			//if (exitp != -1) {				// 파티가 끝났다는 정보를 쏴줘야 한다. 
			//	for (int i = 0; i < 4; ++i)
			//	{
			//		for (int j = 0; j < 4; ++j)
			//		{
			//			if (party[exitp].client[i] != NULL && party[exitp].client[j] != NULL)
			//			{
			//				int s_id, r_id;
			//				// 여기 개 줄일수 있는데인데 하...
			//				for (int k = 0; k < MAX_USER; k++)
			//				{
			//					if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			//					if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exitp].client[i]->GetName_W()))
			//						s_id = k;
			//				}
			//				for (int k = 0; k < MAX_USER; k++)
			//				{
			//					if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			//					if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exitp].client[j]->GetName_W()))
			//						r_id = k;
			//				}
			//				sc_packet_party_set party_set;

			//				party_set.size = sizeof(sc_packet_party_set);
			//				party_set.type = 85;
			//				party_set.xPos = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetPosX();
			//				party_set.yPos = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetPosZ();
			//				party_set.hp = g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetHP();
			//				party_set.m_count = party[exitp].mm_count;
			//				wcscpy_s(party_set.member_name, 50, g_ServerFramework.GetPlayerInfo(s_id)->GetPlayerInfo()->GetName_W());

			//				if (true == g_ServerFramework.GetPlayerInfo(r_id)->GetIsConnect() || wcscmp(g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W(), g_ServerFramework.GetPlayerInfo(r_id)->GetPlayerInfo()->GetName_W()))
			//				{
			//					SendPacket(r_id, reinterpret_cast<unsigned char*>(&party_set));
			//				}
			//			}
			//		}
			//	}
			//		

			//	// 모두 나가면 파티 bool 값 false
			//	if (party[exitp].client[0] == NULL && party[exitp].client[1] == NULL && party[exitp].client[2] == NULL && party[exitp].client[3] == NULL) 
			//	{
			//		party[exitp].is = false;
			//		party_count--;
			//	}

			//	// 여기서 또 처리 해줘야 하는데 						
			//	
			//	//for (int i = 0; i < 4; ++i) {
			//	//	if (wcscmp(party[exitp].client[i]->GetName_W(), g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W())) {// 파티원들 i 알아내기
			//	//		/////////??????????????????????????????????????????????????????????????? 
			//	//		int e_id;
			//	//		// 여기 개 줄일수 있는데인데 하...
			//	//		for (int k = 0; k < MAX_USER; k++)
			//	//		{
			//	//			if (false == g_ServerFramework.GetPlayerInfo(k)->GetIsConnect()) continue;	//접속 아니면 꺼지고
			//	//			if (false == wcscmp(g_ServerFramework.GetPlayerInfo(k)->GetPlayerInfo()->GetName_W(), party[exitp].client[i]->GetName_W()))
			//	//				e_id = k;
			//	//		}
			//	//		sc_packet_party_set party_set;

			//	//		party_set.size = sizeof(sc_packet_party_set);
			//	//		party_set.type = 76;
			//	//		party_set.xPos = 0;
			//	//		party_set.yPos = 0;
			//	//		party_set.hp = 0;
			//	//		party_set.m_count = 0;
			//	//		wcscpy_s(party_set.member_name, 50, g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W());

			//	//		if (true == g_ServerFramework.GetPlayerInfo(e_id)->GetIsConnect())
			//	//		{
			//	//			SendPacket(e_id, reinterpret_cast<unsigned char*>(&party_set));
			//	//		}

			//	//	}
			//	//}

			//	for (int j = 0; j < 4; j++)
			//	{
			//		if (party[exitp].client[j] != NULL)
			//			if (!wcscmp(party[exitp].client[j]->GetName_W(), g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->GetName_W()))
			//			{//key,j 같으면 그 파티원 파티 탈퇴시키고 
			//				party[exitp].client[j] = NULL;
			//				party[exitp].mm_count--;
			//				g_ServerFramework.GetPlayerInfo(key)->GetPlayerInfo()->SetPartyNumber(-1);
			//			}
			//	}
			//}
			//ksh7
			sc_packet_pop_player pop_player;
			pop_player.size = sizeof(sc_packet_pop_player);
			pop_player.type = PLAYER_EVENT_POP;
			pop_player.id = key;
			for (auto i = 0; i < MAX_USER; ++i) {
				if (g_ServerFramework.GetPlayerInfo(i)->GetIsConnect() == false) continue;
				if (i == key) continue;
				if (g_ServerFramework.GetClientInfo(i)->m_view_list.Contains(key)) {
					g_ServerFramework.GetClientInfo(i)->m_view_list.Remove(key);
					SendPacket(i, reinterpret_cast<unsigned char*>(&pop_player));
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
			ProcessNPCPakcet(key, ENEMY_EVENT_MOVE);
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_NPC_HEATBEAT) {
			ProcessNPCPakcet(key, ENEMY_EVENT_HEART_BEAT);
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_NPC_ASTAR) {
			ProcessNPCPakcet(key, ENEMY_EVENT_ASTAR);
			delete overlap;
		}
		else if (overlap->m_nOperation == OP_SUNSET) {
			ProcessSunset();
			delete overlap;
		}
		else {
			cout << "UnKnown Event on worker Thread \n";
		}
	}
}
int main()
{

	g_SpaceParition.Initialize_space_division(2048,5,1);

	g_SpaceParition.BuildWorldSpace();
	vector<multimap<float, CObjects*>> v = g_SpaceParition.GetSpace();
	// 서버 초기화
	g_ServerFramework.InitializeServer(&g_SpaceParition);
	cout << "build up" << endl;
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	std::vector<std::thread*> worker_threads;
	for (auto i = 0; i < NUM_THREADS; ++i) {
		worker_threads.push_back(new std::thread{ WorkerThreadFunc });
	}
	std::thread Accept = std::thread{ AcceptThread };
	thread Timer = thread{ TimerThread };

	while (1)
	{
		Sleep(500);
		g_ServerFramework.FlockRun();
		for (auto i = 0; i < MAX_USER; ++i) {
			if (false == g_ServerFramework.GetClientInfo(i)->GetIsConnect()) continue;
			PlayerInfo *client = g_ServerFramework.GetClientInfo(i);
			CPlayer *player = client->GetPlayerInfo();
			vector<BOID*> boids = g_ServerFramework.GetFlock().GetBoid();
			for (auto j = 0; j < MAX_STEERING; ++j) {
				//if (false == player->Steering_Distance(boids[j])) continue;
				VECTOR2D position = boids[j]->GetPositon();
				VECTOR2D velocity = boids[j]->GetVelocity();

				sc_packet_steer_position steer_position;
				steer_position.size = sizeof(sc_packet_steer_position);
				steer_position.type = FLOCK_EVENT_MOVE;
				steer_position.id = j;
				steer_position.x = position.x_pos;
				steer_position.y = 150.0f;
				steer_position.z = position.z_pos;
				steer_position.look_x = velocity.x_pos;
				steer_position.look_y = 0.0f;
				steer_position.look_z = velocity.z_pos;

				SendPacket(i, reinterpret_cast<unsigned char*>(&steer_position));
			}
		}

	}

	for (auto th : worker_threads) {
		th->join();
		delete th;
	}

	Accept.join();
	Timer.join();

	g_ServerFramework.CleanUpServer();
}