#pragma once

#include "stdafx.h"

class CObjects
{
protected:
	float m_fXpos;
	float m_fYpos;
	float m_fZpos;

	float m_fLook_x;
	float m_fLook_y;
	float m_fLook_z;

	float m_fMax_Xpos;
	float m_fMax_Zpos;

	float m_fMin_Xpos;
	float m_fMin_Zpos;
	

	int m_nID;
	string name;

	int m_nState; // 상태

public:
	CObjects();
	virtual ~CObjects();


	float GetPosX() { return m_fXpos; }
	float GetPosY() { return m_fYpos; }
	float GetPosZ() { return m_fZpos; }

	float GetLookX() { return m_fLook_x; }
	float GetLookY() { return m_fLook_y; }
	float GetLookZ() { return m_fLook_z; }

	void SetPosition(float x, float y, float z) { m_fXpos = x, m_fYpos = y, m_fZpos = z; }
	void SetLookVector(float x, float y, float z) { m_fLook_x = x, m_fLook_y = y; m_fLook_z = z; };
	virtual void SetRegenPosition(float x, float y, float z) {  }
	void SetMinMax(float max_x, float max_z, float min_x, float min_z) {
		m_fMax_Xpos = max_x; m_fMax_Zpos = max_z, m_fMin_Xpos = min_x, m_fMin_Zpos = min_z;
	}

	bool Collision_Check(float other_max_X, float other_max_Z, float other_min_X, float other_min_Z) {
		if (other_min_X <= m_fMax_Xpos &&
			other_min_Z <= m_fMax_Zpos &&
			other_max_X >= m_fMin_Xpos &&
			other_max_Z >= m_fMin_Zpos) return true;
		return false;
	}

	void SetName(string s) { name = s; }
	string GetName() { return name; }
	int GetNPCID() { return m_nID; }

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
	float regen_x;
	float regen_y;
	float regen_z;
	bool heart_beat;
	int npc_id;
	
	bool target_Mode;
	int target_id;

	bool move_mode;

	float speed;

	////////////// 수정. 8월 15일
	int damage;
	int hp;
	int max_hp; // level x 100
	int give_exp;

	int animate_state;
	enum MONSTER {
		monster_normal = 0
	};
	int type;

	//ksh
	WCHAR user_name[50];
	int m_party;	// 파티번호 가지고 있자.
	//ksh

public:
	CDynamicObject();
	virtual ~CDynamicObject();
	virtual void Move() {}

	void SetHeartBeat(bool b) { heart_beat = b; }
	void SetAnimateState(int state) { animate_state = state;}
	int  GetAnimateState() { return animate_state; }
	bool GetHeartBeat() { return heart_beat; }
	void ResetPosition(float x, float y, float z) { regen_x = x, regen_z = z; }
	void Reset() { m_fXpos = regen_x, m_fZpos = regen_z; }
	void CreateNPC();

	void SetTargetMode(bool b) { target_Mode = b; }
	bool GetTargetMode() { return target_Mode; }
	void SetTargetID(int n) { target_id = n; }
	int GetTargetID() { return target_id; }

	void SetMoveMode(bool b) { move_mode = b; }
	bool GetMoveMode() { return move_mode; }
	
	void SetNPCID(int n) { npc_id = n; }

	////////////// 수정. 8월 15일
	int GetGiveEXP() { return give_exp; }
	int GetHP() { return hp; }
	int GetDamage() { return damage; }
	bool DecreaseHP(int n) { 
		hp -= n;
		if (hp < 0) {
			ReSetHP();
			return true;
		}
		return false;
	}
	void ReSetHP() { hp = max_hp; }

	void EnemyStatus(int type) {
		switch (type) {
		case 0:// normal
			damage = 20.0f;
			max_hp = 100.0f; // level x 100
			hp = max_hp;
			give_exp = 20.0f;
			break;
		default:
			break;
		}
	}

	//void SetLookVector(float x, float y, float z) { m_fLook_x = x; m_fLook_y = y; m_fLook_z = z; }
	void Enemy_Move();

	//ksh
	WCHAR* GetName_W() { return user_name; }

	void SetPartyNumber(int p) { m_party = p; }
	int GetPartyNumber() { return m_party; }
	//ksh 

};

//ksh
struct Party {
	bool is;
	int mm_count;
	CDynamicObject* client[4];
};
//ksh