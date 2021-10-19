#pragma once

#include "GameObject.h"

class CPlayer : public CGameObject
{
protected:
	int m_n_hp;
	int m_n_exp;
	int m_n_level;

	wchar_t m_wc_id[MAX_STR_SIZE];

	wchar_t speach_text[256];
	DWORD speach_time;

public:
	CPlayer();
	virtual ~CPlayer();
	
	int GetHp() { return m_n_hp; }
	int GetExp() { return m_n_exp; }
	int GetLevel() { return m_n_level; }

	void SetPlayerStat(int level, int exp, int hp) {
		m_n_level = level;
		m_n_hp = hp;
		m_n_exp = exp;
	}
	
	void SetHp(int hp) { m_n_hp = hp; }
	void setspeach_text(wchar_t* text) { wcsncpy_s(speach_text, text, 256); }
	void set_speach_time(DWORD time) { speach_time = time; }
	void Set_my_user_id(wchar_t *t) { wcsncpy_s(m_wc_id, t, MAX_STR_SIZE); }
	virtual void Animate(float fTimeElapsed);
	virtual void Draw(ID2D1RenderTarget *pd2dRenderTartget);
	virtual void BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory = NULL, IWICImagingFactory *pwicFactory = NULL);
	virtual void ReleaseObjects();
	void Move(float x, float y) {
		m_d2dmtxWorld._31 += x;
		m_d2dmtxWorld._32 += y;
	}
};

