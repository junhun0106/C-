#pragma once
//#include "procol.h"

class CConnectServer
{
	WSADATA		m_wsadata;
	SOCKET		m_client_sock;
	SOCKADDR_IN m_server_addr;

	WSABUF		m_wsa_recv_buf;
	char		m_crecv_buf[BUFF_SIZE];
	int			m_nCurrent_recvied; // 현재 받은 크기 
	int			m_nPrev_recived; // 이전 남은 크기

	char		m_packet_buf[BUFF_SIZE];

	int			m_nMy_client_id;


	std::pair<pair<float, float>, float> my_player_pos;
	int my_player_level{ 0 };
	int my_player_exp{ 0 };
	int my_player_hp{ 0 };


	std::pair<pair<float, float>, float> other_player_pos[MAX_USER];
	std::pair<pair<float, float>, float> other_player_look_vector[MAX_USER];
	bool	other_player_view_sate[MAX_USER];
	unsigned int		other_player_animate_sate[MAX_USER];

	std::pair<pair<float, float>, float> enemy_npc_pos[MAX_ENEMY];
	std::pair<pair<float, float>, float> enemy_npc_look_vector[MAX_ENEMY];
	bool	enemy_view_state[MAX_ENEMY];
	bool	enemy_heat_beat_state[MAX_ENEMY];
	int enemy_hp[MAX_ENEMY];

	pair<float, float> steer_npc_position[MAX_STEERING];
	pair<pair<float, float>, float> steer_npc_look_vector[MAX_STEERING];
	bool steer_npc_view_state[MAX_STEERING];

	vector<wstring> chat_message;

	bool		my_player_state;
	bool		player_attack{ false };

	bool party_window{ false };
	//ksh7
	bool		joinbox{ false };
	bool		setParty{ false };	// 1이 되면 사라지자.


									//ksh7
	pair<pair<float, float>, float> sunset_position;

									//ksh7

public:
	CConnectServer();
	~CConnectServer();

	void ConnectServerInitialize(HWND hWnd);
	void SendPacket(unsigned char* packet);
	void ReadRecvPacket(SOCKET sock);
	void ProcessRecvPacket(char *ptr);
	void DisConnectSerever();
	SOCKET GetSocket() { return m_client_sock; }
	int GetMYid() { return m_nMy_client_id; }
	
	std::pair<pair<float, float>, float> GetMyPlayerPos() { return my_player_pos; }

	std::pair<pair<float, float>, float> GetOtherPlayerPos(int index) { return other_player_pos[index]; }
	std::pair<pair<float, float>, float> GetOtherPlayerLookVector(int index) { return other_player_look_vector[index]; }
	bool GetOhterPlayerViewState(int index) { return other_player_view_sate[index]; }
	unsigned int GetOtherPlayerAnimateState(int index) { return other_player_animate_sate[index]; }

	std::pair<pair<float, float>, float> GetEnemyNPCPos(int index) { return enemy_npc_pos[index]; }
	std::pair<pair<float, float>, float> GetEnemyNPCLookVector(int index) { return enemy_npc_look_vector[index]; }
	bool GetEnemyNPCView(int index) { return enemy_view_state[index]; }
	bool GetEnemyNPCHeartBeatState(int index) { return enemy_heat_beat_state[index]; }
	int GetEnemyHP(int index) { return enemy_hp[index]; }

	pair<float, float> GetSteerPosition(int index) { return steer_npc_position[index]; }
	pair<pair<float, float>, float> GetSteerLookVector(int index) { return steer_npc_look_vector[index]; }
	bool GetSteerViewState(int index) { return steer_npc_view_state[index]; }

	bool GetPlayerAttack() {
		return player_attack;
	}
	void SetPlayerAttack() { player_attack = false; }

	wstring GetChat(int index) { return chat_message[index]; }
	pair<pair<float, float>, float> GetMyPlayerStatus(){
		pair<pair<float, float>, float> status;
		status.first.first = my_player_level;
		status.first.second = my_player_hp;
		status.second = my_player_exp;
		return status;
	}

	bool GetPartyWindow() {
		return party_window;
	}
	void SetPartyWindow() {
		party_window = false;
	}
	//ksh7
	//ksh7
	bool GetJoinBox() { return joinbox; }
	bool GetParty() { return setParty; }


	//ksh7
	//ksh7
	pair<pair<float, float>,float>  GetSunSetPosition() { return sunset_position; }
};

//ksh7
struct member {
	WCHAR party_member[50];
	int pHP;
	int pLevel;
	int pXpos;
	int pYpos;
	int pCount;
};

