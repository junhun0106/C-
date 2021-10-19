#pragma once

#include "CLIENT.h"

class CASLock {
	volatile unsigned int run = 0;
public:
	bool is_runnig() {
		return 1 == ::InterlockedExchange(&run, 1);
	}
	void lock() {
		while (is_runnig()) {
			::Sleep(2);
		}
	}
	void unlock() {
		::InterlockedExchange(&run, false);
	}
};


class CSERVER
{
private:
	WSADATA wsadata;
	SOCKET listen_socket;
	HANDLE hIocp;
	bool shut_down;
	//클라이언트 정보
	std::atomic<int> client_count;
	using CLIENT_INFO = std::map<int, CCLIENT*>;
	CLIENT_INFO client_list;
	//방 정보
	std::atomic<int> room_count;	

	CASLock caslock;
public:
	CSERVER() = default;
	~CSERVER() = default;

	void StartUpServer();
	void CleanUpServer();
	void SetShutDown(bool b) {
		shut_down = b;
	}

	///////////////// ProcessPacket.cpp
private:
	typedef void(CSERVER::*PACKET_FP)(int id, unsigned char* packet);
	using DATAS = std::map<int, PACKET_FP>;
	DATAS handler;

	bool register_handle(int type, PACKET_FP fp);
	void Initialize_handler();

	void RequestCreateRoom(int id, unsigned char* packet);
	void RequestChattingMessage(int id, unsigned char* packe);
	void CheckEmptyRoom(int id, unsigned char* packe);

public:
	void ProcessPacket(int id, unsigned char* packet);
	void RecvPacket(int id);
	void SendPakcet(int id, unsigned char* packet);

	///////////////// Threads.cpp
private:
	std::vector<std::thread*> threadspool;
	void AcceptThread(); // 후에 AcceptEX()로 바꿔보기.
	void WorkerThread();
	void InitializeThreads();
	void DestroyThreads();
};

