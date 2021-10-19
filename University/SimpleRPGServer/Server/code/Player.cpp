#include "stdafx.h"
#include "Player.h"


CPlayer::CPlayer() : CDynamicObject()
{
	m_fXpos = -50000.0f;
	m_fYpos = -50000.0f;

	level = 0;
	need_exp = 0;
	player_hp = 0;

	feedback = false;
}
CPlayer::~CPlayer()
{
}
void CPlayer::Move(float fXpos, float fYpos)
{
	m_fXpos += fXpos;
	m_fYpos += fYpos;
}
bool CPlayer::Collision(CDynamicObject* other) {
	int other_x = other->GetPosX() / 40;
	int other_y = other->GetPosY() / 40;

	int fDeltaX = (m_fXpos / 40) - other_x;
	int fDeltaY = (m_fYpos / 40) - other_y;


	int dist = (fDeltaX * fDeltaX) + (fDeltaY * fDeltaY);

	return (dist <= 1 * 1);
}
bool CPlayer::Distance(CPlayer* other) {
	int other_x = other->GetPosX() / 40;
	int other_y = other->GetPosY() / 40;

	int fDeltaX = (m_fXpos / 40) - other_x;
	int fDeltaY = (m_fYpos / 40) - other_y;


	int dist = (fDeltaX * fDeltaX) + (fDeltaY * fDeltaY);

	return (dist <= 6 * 6);
}
bool CPlayer::Distance(CDynamicObject *other) {
	int other_x = other->GetPosX() / 40;
	int other_y = other->GetPosY() / 40;

	int fDeltaX = (m_fXpos / 40) - other_x;
	int fDeltaY = (m_fYpos / 40) - other_y;


	int dist = (fDeltaX * fDeltaX) + (fDeltaY * fDeltaY);

	return (dist <= 10 * 10 );
}