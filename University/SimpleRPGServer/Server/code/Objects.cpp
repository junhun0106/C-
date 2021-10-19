#include "Objects.h"

CObjects::CObjects()
{
	m_fXpos = 0.0;
	m_fYpos = 0.0;


}
CObjects::~CObjects()
{
}

// 정적 객체
CStaticObject::CStaticObject() : CObjects()
{

}
CStaticObject::~CStaticObject()
{

}

// 동적 객체
CDynamicObject::CDynamicObject() : CObjects()
{
	m_dwMoveTime = true;
	m_bActive = false;
	m_dwMoveTime = 0;
	m_bdeath = true;
	hp = 0;
	npc_type = 0;
	exp = 0;

	target_id = -1;
	target_mode = false;

	wmemset(name, 0, MAX_NAME_SIZE);
}
CDynamicObject::~CDynamicObject()
{

}
void CDynamicObject::CreateNPC(int type) {
	npc_id = -1;

	npc_type = type;
	switch (npc_type) {
	case MONSTER_STATIC_NO_ATTACK: {
		hp = 100; original_hp = 100; exp = 10; 
		damage = 10;
		m_fXpos = 20 + (40 * (rand() % 100));
		m_fYpos = (200 * 40) + 20 + (40 * (rand() % 200));	
		break;
	}
	case MONSTER_STATIC_ATTACK: {
		hp = 150; original_hp = 150; exp = 20;
		damage = 10;
		m_fXpos = (100 * 40) + 20 + (40 * (rand() % 100));
		m_fYpos = (200 * 40) + 20 + (40 * (rand() % 200));
		break;
	}
	case MONSTER_DYNAMIC_NO_ATTACK: {
		hp = 200; original_hp = 200; exp = 30;
		damage = 20;
		m_fXpos = (100 * 40) + 20 + (40 * (rand() % 100));
		m_fYpos = 20 + (40 * (rand() % 200));
		break;
	}
	case MONSTER_DYNAMIC_ATTACK: {
		hp = 200; original_hp = 200; exp = 30;
		damage = 20;
		m_fXpos = (200 *40) + 20 + (40 * (rand() % 100));
		m_fYpos = 20 +(40 * (rand() % 200));
		break;
	}
	case MONSTER_DYNAMIC_LONG: {
		hp = 200; original_hp = 200; exp = 30;
		damage = 30;
		m_fXpos = (200 * 40) + 20 + (40 * (rand() % 100));
		m_fYpos = (200 * 40) + (40 * (rand() % 200));
		break;
	}
	default: break;
	}

	regen.first = m_fXpos;
	regen.second = m_fYpos;
}
void CDynamicObject::Enemy_Move() {
	int key = rand() % 4;
	switch (key) {
	case 0:	if (m_fXpos < WORLD_WIDTH - 40.0f) m_fXpos += 40.0f;
		break;
	case 1: if (m_fXpos > 40.0f) m_fXpos -= 40.0f;
		break;
	case 2: if (m_fYpos < WORLD_HEIGHT - 40.0f) m_fYpos += 40.0f;
		break;
	case 3: if (m_fYpos > 40.0f) m_fYpos -= 40.0f;
		break;
	default:
		break;
	}
}
bool CDynamicObject::Distance(CDynamicObject* other) {
	int other_x = other->GetPosX() / 40;
	int other_y = other->GetPosY() / 40;

	int fDeltaX = (m_fXpos / 40) - other_x;
	int fDeltaY = (m_fYpos / 40) - other_y;


	int dist = (fDeltaX * fDeltaX) + (fDeltaY * fDeltaY);

	return (dist <= 6 * 6);
}