#include "stdafx.h"
#include "PlayerObject.h"

CPlayerObject::CPlayerObject() : CGameObject(1)
{
	m_uiAnimationState = ANIMATE_IDLE;
}


CPlayerObject::~CPlayerObject()
{
}


