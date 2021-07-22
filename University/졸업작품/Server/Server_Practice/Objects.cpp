#include "Objects.h"



CObjects::CObjects()
{
	m_fXpos = 0.0f;
	m_fYpos = 0.0f;
	m_fZpos = 0.0f;

	m_fMax_Xpos = 0.0f;
	m_fMax_Zpos = 0.0f;
	m_fMin_Xpos = 0.0f;
	m_fMin_Zpos = 0.0f;

	m_fLook_x = 1.0f;
	m_fLook_y = 0.0f;
	m_fLook_z = 0.0f;

	m_nID = -1;

}
CObjects::~CObjects()
{
}
// 沥利 按眉
CStaticObject::CStaticObject() : CObjects()
{

}
CStaticObject::~CStaticObject()
{

}

// 悼利 按眉
CDynamicObject::CDynamicObject() : CObjects()
{
	heart_beat = true;
	speed = 2.0f;
	hp = 2;
	animate_state = ANIMATE_IDLE;

	target_id = -1;
	target_Mode = false;

	move_mode = false;

	m_fLook_x = 1.0f;
	m_fLook_y = 0.0f;
	m_fLook_z = 0.0f;
}
CDynamicObject::~CDynamicObject()
{

}
void CDynamicObject::CreateNPC() {

}
void CDynamicObject::Enemy_Move()
{
	int button = rand() % 9;
	switch (button) {
	case 0: // right
		m_fXpos += (1.0f * speed); 
		m_fLook_x = 1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = 0.0f;
		break;
	case 1: // left
		m_fXpos -= (1.0f * speed); 
		m_fLook_x = -1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = 0.0f;
		break;
	case 2: // foward
		m_fZpos += (1.0f * speed); 
		m_fLook_x = 0.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = 1.0f;
		break;
	case 3:	// back
		m_fZpos -= (1.0f * speed); 
		m_fLook_x = 0.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = -1.0f;
		break;
	case 4: // right foward
		m_fXpos += (1.0f* speed); 
		m_fZpos += (1.0f* speed); 
		m_fLook_x = 1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = 1.0f;
		break;
	case 5: // right back
		m_fXpos += (1.0f* speed); 
		m_fZpos -= (1.0f* speed); 
		m_fLook_x = 1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = -1.0f;
		break;
	case 6: // left foward
		m_fXpos -= (1.0f* speed); 
		m_fZpos += (1.0f* speed); 
		m_fLook_x = -1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = 1.0f;
		break;
	case 7: // left back
		m_fXpos -= (1.0f* speed); 
		m_fZpos -= (1.0f* speed); 
		m_fLook_x = -1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = -1.0f;
		break;
	case 9: // idle
		m_fXpos = 0.0f;
		m_fZpos = 0.0f;
		m_fLook_x = 1.0f;
		m_fLook_y = 0.0f;
		m_fLook_z = 0.0f;
	default: break;
	}
}