#pragma once

#include "Scene.h"
#include "Player.h"

class CUIScene : public CScene
{
	CPlayer *m_pPlayer;

	CGameObject *m_pgo_player_hp;
	CGameObject *m_pgo_player_exp;

	ID2D1RectangleGeometry *m_pd2drc_hp_rect;
	ID2D1RectangleGeometry *m_pd2drc_exp_rect;

	IDWriteTextFormat *m_pdw_text_format;

	ID2D1SolidColorBrush *m_pd2dsbr_text_color;
	ID2D1SolidColorBrush *m_pd2dsbr_rect_color;

	wchar_t mess[256];

public:
	CUIScene();
	virtual ~CUIScene();

	virtual void SetPlayer(CPlayer* player) { m_pPlayer = player; }

	virtual void Draw(ID2D1RenderTarget *pd2dRenderTartget);
	virtual void BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory = NULL, IWICImagingFactory *pwicFactory = NULL);
	virtual void ReleaseObjects();
};

