#include "ServerFramework.h"

//ksh
Party party[1000];
//ksh

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
double Rounding(double x, int digit)
{
	return (floor((x)* pow(float(10), digit) + 0.5f) / pow(float(10), digit));
}
void CServerFramework::InitializeServer(CSpacePartition* space)
{
	WSAStartup(MAKEWORD(2, 2), &m_Wsadata);

	for (auto i = 0; i < MAX_USER; ++i) {
		playerinfo[i]->InitializeOverlapped();
	}

	std::ifstream file_in("Scripts/AssetsText(몬스터용2km2km).script");
	string ignore;
	string object_name;
	int object_num = 0;


	file_in >> ignore >> ignore;
	file_in >> ignore >> object_num;

	int file_object_count, mesh_object_count, object_count;
	file_object_count = mesh_object_count = object_count = 0;
	file_in >> ignore >> object_name;
	file_in >> ignore >> mesh_object_count;

	while (file_object_count < MAX_ENEMY) {
		float x_pos, y_pos, z_pos;
		x_pos = y_pos = z_pos = 0;
		file_in >> ignore >> x_pos >> y_pos >> z_pos;

		int x = (int)x_pos;
		int y = (int)y_pos;
		int z = (int)z_pos;

		y_pos += 0.0f;
		enemy_objects[file_object_count] = new CDynamicObject();
		enemy_objects[file_object_count]->SetPosition(x, y, z);
		enemy_objects[file_object_count]->SetName(object_name);
		enemy_objects[file_object_count]->EnemyStatus(0);
		enemy_objects[file_object_count]->ResetPosition(x,y,z);
		file_object_count++;
	}
	file_in.close();

	//ksh
	for (int i = 0; i < 1000; i++) {
		party[i].is = false;
		party[i].mm_count = 0;
		for (int j = 0; j < 4; j++) {
			party[i].client[j] == NULL;
		}
	}
	//ksh

	astar.CreatePath(space);
	
	for (auto i = 0; i < 100; ++i) {
		boids = new BOID(rand() % 100, rand() % 100);
		flock.Add(boids);
	}
	for (auto i = 0; i < MAX_STEERING - 100; ++i) {
		boids = new BOID(rand() % 1000 + 100 , rand() % 1000 + 100);
		flock.Add(boids);
	}
}
void CServerFramework::CleanUpServer()
{
	closesocket(m_sockListen);
	WSACleanup();
}
