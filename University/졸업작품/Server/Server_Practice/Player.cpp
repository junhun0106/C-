#include "stdafx.h"
#include "Player.h"


CPlayer::CPlayer() : CDynamicObject()
{
	m_fXpos = 0.0f;
	m_fYpos = 0.0f;
	m_fZpos = 0.0f;
}
CPlayer::~CPlayer()
{
}
void CPlayer::Move(float fXpos, float fYpos)
{
	m_fXpos += fXpos;
	m_fYpos += fYpos;
}
bool CPlayer::Distance(CPlayer* other) {
	float other_X = other->GetPosX();
	float other_Z = other->GetPosZ();
	float fDeltaX = m_fXpos - other_X;
	float fDeltaZ = m_fZpos - other_Z;

	int dist = (fDeltaX * fDeltaX) + (fDeltaZ * fDeltaZ);

	return (dist <= 50 * 50);
}
bool CPlayer::Distance(CDynamicObject *other) {
	float other_x = other->GetPosX();
	float other_z = other->GetPosZ();

	float fDeltaX = m_fXpos - other_x;
	float fDeltaZ = m_fZpos - other_z;


	int dist = (fDeltaX * fDeltaX) + (fDeltaZ * fDeltaZ);

	return (dist <= 60 * 60);
}
bool CPlayer::attack_area(CDynamicObject* other) {
	float other_x = other->GetPosX();
	float other_z = other->GetPosZ();
	float fDeltaX = m_fXpos - other_x;
	float fDeltaZ = m_fZpos - other_z;
	int dist = (fDeltaX * fDeltaX) + (fDeltaZ * fDeltaZ);
	return (dist <= 5 * 5);
}
bool CPlayer::Astar_Distance(CDynamicObject* other) {
	float other_x = other->GetPosX();
	float other_z = other->GetPosZ();
	float fDeltaX = m_fXpos - other_x;
	float fDeltaZ = m_fZpos - other_z;
	int dist = (fDeltaX * fDeltaX) + (fDeltaZ * fDeltaZ);
	return (dist <= 30 * 30);
}
bool CPlayer::Steering_Distance(BOID* other) {
	VECTOR2D pos = other->GetPositon();
	float delta_x = m_fXpos - pos.x_pos;
	float delta_z = m_fZpos - pos.z_pos;
	float dist = (delta_x * delta_x + delta_z * delta_z);
	return (dist <= 100 * 100);
}