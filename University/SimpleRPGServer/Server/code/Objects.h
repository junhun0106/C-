#pragma once

#include "stdafx.h"

class CObjects
{
protected:
	float m_fXpos;
	float m_fYpos;

	int m_nState; // ป๓ลย
public:
	CObjects();
	virtual ~CObjects();

	float GetPosX() { return m_fXpos; }
	float GetPosY() { return m_fYpos; }

	void SetPosition(float f1, float f2) { m_fXpos = f1, m_fYpos = f2; }


	bool Collision_Check(float other_max_X, float other_max_Z, float other_min_X, float other_min_Z) {
		if (other_min_X <= m_fXpos + 0.5f &&
			other_min_Z <= m_fYpos + 0.5f &&
			other_max_X >= m_fXpos - 0.5f &&
			other_max_Z >= m_fYpos - 0.5f) return true;
		return false;
	}

	virtual void move(){}
};
class CStaticObject : public CObjects
{
public:
	CStaticObject();
	virtual ~CStaticObject();
};
class CDynamicObject : public CObjects
{	
protected:
	DWORD m_dwMoveTime;

	bool m_bdeath;
	bool m_bMoveTime;
	bool m_bActive;
	int npc_id;

	bool war_peace;
	
	pair<float, float> target;
	pair<float, float> regen;

	int npc_type;
	int original_hp;
	int hp;

	int exp;
	int damage;

	int target_id;
	bool target_mode;

	wchar_t name[100];
public:
	lua_State *L;

	CDynamicObject();
	virtual ~CDynamicObject();
	virtual void Move() {}

	void SetTargetMode(bool b) { target_mode = b; }
	bool GetTargetMode() { return target_mode; }
	void SetTargetID(int id) { target_id = id; }
	int GetTargetID() { return target_id; }


	void DecreaseHp() { hp -= 50; }
	void ReSetHp() { 
		target_id = -1;
		target_mode = false;
		hp = original_hp; 
	}

	int GetHP() { return hp; }
	int GetExp() { return exp; }

	void CreateNPC(int type);

	void SetNPCID(int n) { npc_id = n; }
	void Enemy_Move();
	
	void StartTime() { m_dwMoveTime = GetTickCount(); }
	void StartTimeBool() { m_bMoveTime = true; }
	void EndTimeBool() { m_bMoveTime = false; }
	bool GetTimeBool() { return m_bMoveTime; }
	DWORD GetTime() { return m_dwMoveTime; }
	int GetType() { return npc_type; }
	int GetDamege() { return damage; }

	bool GetActive() { return m_bActive; }
	void WakeUp_NPC() { m_bActive = true; }
	void Sleep_NPC() { m_bActive = false; }

	bool GetDeath() { return m_bdeath; }
	void SetDeath(bool b) { m_bdeath = b; if (b) hp = 100; }

	bool Distance(CDynamicObject *other);

	pair<float, float> GetRegenPos() { return regen; }
};