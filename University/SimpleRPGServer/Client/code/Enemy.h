#pragma once
#include "Player.h"

class CEnemy : public CPlayer
{
	wchar_t npc_name[MAX_STR_SIZE];

	ID2D1RectangleGeometry *m_prc_hp_rect;
	ID2D1SolidColorBrush *m_pd2dsbr_hp_color;

public:
	CEnemy();
	virtual ~CEnemy();

	void SetName(std::wstring s) { wcsncpy_s(npc_name, s.c_str(), s.size()); }

	virtual void Animate(float fTimeElapsed);
	virtual void Draw(ID2D1RenderTarget *pd2dRenderTartget);
	virtual void BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory = NULL, IWICImagingFactory *pwicFactory = NULL);
	virtual void ReleaseObjects();
};

