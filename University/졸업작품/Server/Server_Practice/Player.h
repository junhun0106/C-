#pragma once
#include "Objects.h"
#include "FLOCK.h"

class CPlayer : public CDynamicObject
{

	////////////// ����. 8�� 15��
	int level;
	int take_exp;
	int max_exp; // level x 500


public:
	CPlayer();
	virtual ~CPlayer();
	virtual void Move(float, float);

	bool Distance(CPlayer *other);
	bool Astar_Distance(CDynamicObject* other);
	bool Distance(CDynamicObject *other);
	bool attack_area(CDynamicObject *ohter);
	bool Steering_Distance(BOID *other);

	////////////// ����. 8�� 15��
	void SetStatus(int l, int e, int h) {
		level = l;
		take_exp = e;
		hp = h;
		max_hp = level * 100;
		max_exp = level * 500;
	}
	void SetPlayerDamage(int type) {
		switch (type) {
		case 0: // ������
			damage = 20.0f;
			break;
		case 1: // �����
			damage = 40.0f;
			break;
		case 2: // �޵�
			damage = 10.0f;
			break;
		case 3: // ������
			damage = 50.0f;
			break;
		default:
			break;
		}
	}

	int GetLevel() { return level; }
	int GetTakeExp() { return take_exp; }
	void LevelUp() { level++; take_exp = 0; max_exp = level * 500; }
	bool UpExp(int n) {
		take_exp += n;
		if (take_exp > max_exp) {
			LevelUp();
			return true;
		}
		return false;
	}
};

