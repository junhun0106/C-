#pragma once

#include "Player.h"

class CScene
{
protected:
	ID2D1Factory					*m_pd2dFactory;
	IWICImagingFactory				*m_pwicFactory;
	IDWriteFactory					*m_pdwFactory;
	ID2D1HwndRenderTarget			*m_pd2dRenderTarget;

	ID2D1Geometry					*m_pd2dGeometry;
	ID2D1RectangleGeometry			*m_pd2dRcGeometry;
	ID2D1SolidColorBrush			*m_pd2dsbrDraw;
	ID2D1SolidColorBrush			*m_pd2dsbrFill;

	ID2D1SolidColorBrush			*m_pd2dsbrPoint;

	CGameObject						**m_ppRect;

	int								m_nObject;

	std::pair<float, float>			m_fPlayerPos;


public:
	CScene();
	~CScene();

	virtual void Draw(ID2D1RenderTarget *pd2dRenderTartget);
	virtual void BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory = NULL, IWICImagingFactory *pwicFactory = NULL);
	virtual void ReleaseObjects();

	virtual void SetPlayer(CPlayer* player) {}
	void SetPlayerPos(float x, float y) { m_fPlayerPos.first = x; m_fPlayerPos.second = y; }

	CGameObject **GetRect() { return m_ppRect; }
	int GetObejctsEA() { return m_nObject; }
};



