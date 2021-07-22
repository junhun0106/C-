#pragma once
#include "Objects.h"
#include "FLOCK.h"

class CPlayer : public CDynamicObject
{

	////////////// 수정. 8월 15일
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

	////////////// 수정. 8월 15일
	void SetStatus(int l, int e, int h) {
		level = l;
		take_exp = e;
		hp = h;
		max_hp = level * 100;
		max_exp = level * 500;
	}
	void SetPlayerDamage(int type) {
		switch (type) {
		case 0: // 라이플
			damage = 20.0f;
			break;
		case 1: // 기관총
			damage = 40.0f;
			break;
		case 2: // 메딕
			damage = 10.0f;
			break;
		case 3: // 저격총
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

