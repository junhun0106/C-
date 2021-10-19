#pragma once

#include "stdafx.h"
#include "Player.h"
#include "LFLIST.h"
#include "CollisionCheck.h"
#include "ASTAR.h"

struct OverlapEx {
	WSAOVERLAPPED m_original_overlapped;
	int m_nOperation;
	WSABUF m_wsa_buf;
	unsigned char m_socket_buf[MAX_BUFF_SIZE];
	int m_nCurrent_paket_size;
	int param1;
};
class Timee_Queue {
	int m_id;
	int m_event;
	chrono::time_point<chrono::steady_clock> time;
	chrono::time_point<chrono::steady_clock> wake_up_time;
public:
	Timee_Queue() {
		m_id = -1;
		m_event = 0;
	}
	~Timee_Queue() {}

	void add_timer(int nid, int nevent) {
		m_id = nid;
		m_event = nevent;
		time = chrono::high_resolution_clock::now();
		chrono::nanoseconds nano;
		switch (nevent) {
		case ENEMY_MOVE: {chrono::seconds sec(1);  nano = sec; } break;
		case ENEMY_ATTACK_MOVE: {chrono::seconds sec(1); nano = sec; } break;
		case ENEMY_REGEN: {chrono::seconds sec(10); nano = sec; }break;
		case PLAYER_HP: {chrono::seconds sec(5); nano = sec; } break;
		default: break;
		}
		wake_up_time = time + nano;
	}
	void ResetTime(chrono::time_point<chrono::steady_clock> t) {
		time = t;
	}
	int GetNPCID() { return m_id; }
	int GetEvent() { return m_event; }
	chrono::time_point<chrono::steady_clock> GetTime() { return time; }
	auto Gettime() {
		auto du_time = chrono::high_resolution_clock::now() - time;
		return chrono::duration_cast<chrono::seconds>(du_time).count();
	}
	bool operator<=(const Timee_Queue& other) const {
		return (wake_up_time > other.wake_up_time);
	}
	bool operator>=(const Timee_Queue& other) const {
		return (wake_up_time <= other.wake_up_time);
	}

	friend std::less<Timee_Queue>;
};
namespace std {
	template<> class less<Timee_Queue> {
	public:
		bool operator()(const Timee_Queue& first, const Timee_Queue& second) const {
			return  (first.wake_up_time > second.wake_up_time);
		}
	};
}
class PlayerInfo {

	SOCKET socket;
	CPlayer *Player;
	int m_nPred_data_size;
	volatile bool m_bIs_Connected;
	int player_ID;
	char* name;
public:
	OverlapEx m_OverlappedEx;
	unsigned char m_store_packet[MAX_BUFF_SIZE];
	
	set<int> m_view_list;
	recursive_mutex m_vl_lock;

	set<int> m_npc_view_list;
	recursive_mutex m_npc_vl_lock;

public:
	PlayerInfo() { m_bIs_Connected = false; }
	PlayerInfo(const PlayerInfo& playerinfo) {
		m_bIs_Connected = false;
	}
	~PlayerInfo() { }

	void InitializeOverlapped() {
		memset(&m_OverlappedEx.m_original_overlapped, 0, sizeof(m_OverlappedEx.m_original_overlapped));
		memset(m_OverlappedEx.m_socket_buf, 0, MAX_BUFF_SIZE);
		m_OverlappedEx.m_wsa_buf.buf = reinterpret_cast<char*>(m_OverlappedEx.m_socket_buf);
		m_OverlappedEx.m_wsa_buf.len = MAX_BUFF_SIZE;
		m_OverlappedEx.m_nOperation = OP_RECV;
		Player = new CPlayer();
	}
	void SetInfo(SOCKET sock, CPlayer* player, int id) {
		socket = sock;
		player_ID = id;
		m_OverlappedEx.m_nOperation = OP_RECV;
		m_OverlappedEx.m_nCurrent_paket_size = 0;
		m_nPred_data_size = 0;
		Player = player;

		m_vl_lock.lock();
		m_view_list.clear();
		m_vl_lock.unlock();

		m_vl_lock.lock();
		m_npc_view_list.clear();
		m_vl_lock.unlock();
	}
	void SetPlayer_ID(int id) { player_ID = id; }
	void SetIsConnected(volatile bool state){ m_bIs_Connected = state; }
	void SetPreviosData(int size) { m_nPred_data_size = size; }
	void SetPlayer_Name(char *buf) { name = buf; }

	SOCKET GetSocketInfo() { return socket; }
	CPlayer* GetPlayerInfo() { return Player; }
	volatile bool GetIsConnect() { return m_bIs_Connected; }

	int GetPlayer_ID() { return player_ID; }
	int GetPreviousData() { return m_nPred_data_size; }
	char* GetPlayer_Name() { return name; }

	void lock() { m_vl_lock.lock(); }
	void unlock() { m_vl_lock.unlock(); }
};

class CServerFramework
{
private:
	ASTAR astar;

	WSADATA m_Wsadata;
	SOCKET m_sockListen;

	PlayerInfo *playerinfo[MAX_USER];
	CDynamicObject *enemy_objects[MAX_ENEMY];
	int m_nRet;

	std::vector<PlayerInfo> m_vPlayerInfo;
	std::vector<std::pair<std::thread*, bool>> PlayerProcess;

public:
	CServerFramework();
	~CServerFramework();

	void InitializeServer();
	void CleanUpServer();
	SOCKET GetListenSocK() { return m_sockListen; }
	PlayerInfo *GetPlayerInfo(int index) { return playerinfo[index]; }
	PlayerInfo *GetClientInfo(int index) { return playerinfo[index]; }
	CDynamicObject *GetEnemyObject(int index) { return enemy_objects[index]; }
	
	pair<float, float> StartAstar(PathNode* start_node, float target_x, float target_y) { return astar.SerchPath(start_node, target_x, target_y); }
	void SetMapDataInAstar(CCollisionCheck *space) { astar.SetSpace(space); }

	std::vector<PlayerInfo> GetPlayerList() { return m_vPlayerInfo; }
	std::vector<std::pair<std::thread*, bool>> GetThreadList() { return PlayerProcess; }
};