#include "ServerFramework.h"


CServerFramework::CServerFramework()
{
	m_nRet = 0;
	m_vPlayerInfo.reserve(10);
	for (auto i = 0; i < MAX_USER; ++i)
		playerinfo[i] = new PlayerInfo;
}
CServerFramework::~CServerFramework()
{
}
void CServerFramework::InitializeServer()
{
	_wsetlocale(LC_ALL, L"korean");
	WSAStartup(MAKEWORD(2, 2), &m_Wsadata);

	for (auto i = 0; i < MAX_USER; ++i) {
		playerinfo[i]->InitializeOverlapped();
	}

	// A zone 후공 고정(1000)
	for (auto i = 0; i < 1000; ++i) {
		enemy_objects[i] = new CDynamicObject();
		enemy_objects[i]->CreateNPC(MONSTER_STATIC_NO_ATTACK);
	}

	// B zone 선공 고정(1000)
	for (auto i = 1000; i < 2000; ++i) {
		enemy_objects[i] = new CDynamicObject();
		enemy_objects[i]->CreateNPC(MONSTER_STATIC_ATTACK);
	}

	// C zone 후공 이동(1000)
	for (auto i = 2000; i < 3000; ++i) {
		enemy_objects[i] = new CDynamicObject();
		enemy_objects[i]->CreateNPC(MONSTER_DYNAMIC_NO_ATTACK);
	}

	// D zone 선공 이동(1000)
	for (auto i = 3000; i < 4000; ++i) {
		enemy_objects[i] = new CDynamicObject();
		enemy_objects[i]->CreateNPC(MONSTER_DYNAMIC_ATTACK);
	}

	// E zone 추가 컨텐츠 몬스터(1000)
	for (auto i = 4000; i < MAX_ENEMY; ++i) {
		enemy_objects[i] = new CDynamicObject();
		enemy_objects[i]->CreateNPC(MONSTER_DYNAMIC_ATTACK);
	}


	astar.CreatePathNode();
}
void CServerFramework::CleanUpServer()
{
	closesocket(m_sockListen);
	WSACleanup();
}
