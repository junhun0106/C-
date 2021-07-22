#pragma once
#include "Object.h"

class CPlayerObject : public CGameObject
{
public:
	unsigned int m_uiAnimationState;
	void SetAnimationState(unsigned int state)
	{
		if (m_uiAnimationState!=state)
			m_uiAnimationState = state;
	}
	unsigned int GetAnimationState() { return m_uiAnimationState; }
	void Animate(unsigned int inputState)
	{
		m_ppMeshes[0]->GetAnimationState();
	}

	CPlayerObject();
	~CPlayerObject();
};

