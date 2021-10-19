#pragma once


struct enemy_char {
	WCHAR message[256];
	DWORD message_time;
};

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

	std::pair<float, float> my_player_pos;
	int my_player_level;
	int my_player_exp;
	enemy_char my_player_speach;
	bool my_player_collision;
	int my_player_hp;
	wchar_t my_player_id[20];

	enemy_char state_message;

	std::pair<float, float> other_player_pos[MAX_USER];
	bool other_player_view[MAX_USER];
	enemy_char other_player_speach[MAX_USER];
	wchar_t (other_player_id[MAX_STR_SIZE])[MAX_USER];

	std::pair<float, float> enemy_npc_pos[MAX_ENEMY];
	bool	enemy_view_state[MAX_ENEMY];
	int		enemy_hp[MAX_ENEMY];
	enemy_char	enemy_speach[MAX_ENEMY];

public:
	CConnectServer();
	~CConnectServer();

	void ConnectServerInitialize(HWND hWnd, char* id, char* ip);
	void SendPacket(unsigned char* packet);
	void ReadRecvPacket(SOCKET sock);
	void ProcessRecvPacket(char *ptr);
	void DisConnectSerever();
	SOCKET GetSocket() { return m_client_sock; }
	int GetMYid() { return m_nMy_client_id; }


	std::pair<float, float> GetMyPlayerPos() { return my_player_pos; }
	int GetMyPlayerLevel() { return my_player_level; }
	int GetMyPlayerExp() { return my_player_exp; }
	int GetMyPlayerHp() { return my_player_hp; }
	enemy_char GetMyPlayerSpeach() { return my_player_speach; }
	bool GetCollisionCheck() { return my_player_collision; }
	wchar_t* GetUserID() { return my_player_id; }

	std::pair<float, float> GetOtherPlayerPos(int index) { return other_player_pos[index]; }
	bool GetOtherPlayerView(int index) { return other_player_view[index]; }
	enemy_char GetOtherPlayerSpeach(int index) { return other_player_speach[index]; }
	wchar_t* GetOhterPlayerID(int index) { return other_player_id[index]; }


	std::pair<float, float> GetEnemyNPCPos(int index) { return enemy_npc_pos[index]; }
	enemy_char GetEnemySpeach(int index) { return enemy_speach[index]; }
	bool GetEnemyNPCView(int index) { return enemy_view_state[index]; }
	int GetEnemyHP(int index) { return enemy_hp[index]; }


	enemy_char GetStateMessage() { return state_message; }
};

