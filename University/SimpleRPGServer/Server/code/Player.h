#pragma once
#include "Objects.h"

class CPlayer : public CDynamicObject
{
	int need_exp;
	int level;
	int player_hp;
	bool level_up;
	bool feedback;
public:
	CPlayer();
	virtual ~CPlayer();
	virtual void Move(float, float);

	void IncreaseEXP(int exp) {
		need_exp += exp;
		if (need_exp >= (100 * level)) {
			level++;
			need_exp = 0;
			player_hp = level * 100;
			level_up = true;
		}
	}
	void DecreaseHp(int d) { player_hp -= d; }
	void IncreaseHp() { player_hp += 5; }
	void SetStatus(int lev, int exp) {
		need_exp = exp;
		level = lev;
		player_hp = level * 100;
	}

	int GetLevel() { return level; }
	int GetPlayerEXP() { return need_exp; }
	int GetPlayerHp() { return player_hp; }
	void SetPlayerHp(int hp) { player_hp = hp; }
	void SetLevelUp() { level_up = false; }
	bool GetLevelUP() { return level_up; }
	void SetFeedBack(bool b) { feedback = b; }
	bool GetFeedBack() { return feedback; }

	void SetName(wchar_t* w) { wcscpy_s(name, w); }
	wchar_t* GetName() { return name; }

	bool Collision(CDynamicObject *other);
	bool Distance(CPlayer *other);
	bool Distance(CDynamicObject *other);
};

