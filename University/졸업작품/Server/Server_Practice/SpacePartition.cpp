#include "SpacePartition.h"



CSpacePartition::CSpacePartition()
{
	m_nMapSize = 0;
	m_nsection_value = 0;
	m_nOne_Section_size = 0;
	m_nFinal_Section_Number = 0;

	m_nSpace_number = 0;

	m_ObjectsManager.clear();
}


CSpacePartition::~CSpacePartition()
{
	m_ObjectsManager.clear();
}
void CSpacePartition::BuildWorldSpace() {
	std::ifstream file_in("Scripts/server.script");
	string ignore;
	string object_name;
	int object_num = 0;

	float x_pos, y_pos, z_pos, yaw, pitch, roll, min_x, min_z, max_x, max_z;
	x_pos = y_pos = z_pos = yaw = pitch = roll = min_x = min_z = max_x = max_z = 0;

	file_in >> ignore >> ignore;
	file_in >> ignore >> object_num;

	int file_object_count, mesh_object_count, object_count;
	file_object_count = mesh_object_count = object_count = 0;

	while (file_object_count < object_num) {
		file_in >> ignore >> object_name;
		file_in >> ignore >> mesh_object_count;


		for (auto i = 0; i < mesh_object_count; ++i) {
			file_in >> ignore >> x_pos >> y_pos >> z_pos;
			file_in >> ignore >> yaw >> ignore >> ignore;
			file_in >> ignore >> min_x >> min_z >> max_x >> max_z;

			y_pos += 0.5f;

			CObjects *Object = new CObjects();
			Object->SetPosition(x_pos, y_pos, z_pos);
			Object->SetMinMax(max_x, max_z, min_x, min_z);
			Object->SetName(object_name);

			int space_index = serch_space(x_pos, y_pos, z_pos);

			m_ObjectsManager[space_index].insert(make_pair(x_pos, Object));
			file_object_count++;

		}

	}

	file_in.close();

}
void CSpacePartition::Initialize_space_division(int map_size, int sect_value, int want_visible) {
	m_nMapSize = map_size;
	m_nsection_value = sect_value;
	m_nOne_Section_size = pow(2, m_nsection_value);
	m_nFinal_Section_Number = map_size / m_nOne_Section_size;

	m_nSpace_number = pow(m_nOne_Section_size, 2);

	m_ObjectsManager.resize(m_nSpace_number);

}
int CSpacePartition::serch_space(float x, float y, float z) {
	int index_X = x / m_nFinal_Section_Number;
	int index_Y = y / m_nFinal_Section_Number;
	int index_Z = z / m_nFinal_Section_Number;

	int index = index_X + (index_Z * m_nOne_Section_size) + (index_Y * m_nOne_Section_size * m_nOne_Section_size);

	if (index < 0 || index >= m_nSpace_number) return 0;
	if (x < 0  || z < 0 ||
		x > m_nMapSize || z > m_nMapSize) return 0;

	return index;
}
void CSpacePartition::static_build_up(int object_num, const vector<CObjects*>& object_list) {
	for (auto i = 0; i < object_num; ++i) {
		float x = object_list[i]->GetPosX();
		float y = object_list[i]->GetPosY();
		float z = object_list[i]->GetPosZ();

		int index = serch_space(x, y, z);

		m_ObjectsManager[index].insert(make_pair(x, object_list[i]));
	}
}
bool CSpacePartition::find_Object(float x, float y, float z) {
	int index = serch_space(x, y, z);

	auto begin = m_ObjectsManager[index].begin();
	auto end = m_ObjectsManager[index].end();
	while (begin != end) {
		auto b = (*begin).second;
		if (b->GetPosX() - x < 0.01f  && b->GetPosZ() - z < 0.01f)
			return true;
		begin++;
	}
	return false;
}
bool CSpacePartition::Check_Collision(float x, float y, float z, float size) {
	int index = serch_space(x, y, z);
	auto begin = m_ObjectsManager[index].lower_bound(x - 100.0f);
	auto end = m_ObjectsManager[index].upper_bound(x + 100.0f);
	
	while (begin != end) {
		if ((*begin).second->Collision_Check(x + size, z + size, x - size, z - size)) {
			{		
				return true;
			}
		}
		begin++;
	}
	return false;
}
