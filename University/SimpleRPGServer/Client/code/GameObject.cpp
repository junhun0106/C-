#include "stdafx.h"
#include "GameObject.h"


CGameObject::CGameObject()
{
	m_fXpos = -50000.0f;
	m_fYpos = -50000.0f;

	m_bView_state = true;

	m_pd2dGeometry = NULL;
	m_pd2dRcGeometry = NULL;
	m_pd2dsbrDraw = NULL;
	m_pd2dsbrText = NULL;
	m_pd2dsbrFill = NULL;
	m_pd2dBitmap = NULL;
	m_pdwName = NULL;
	m_pdwChat = NULL;

	m_d2dmtxWorld = D2D1::Matrix3x2F::Identity();
	m_d2dmtxWorld._31 = m_fXpos;
	m_d2dmtxWorld._32 = m_fYpos;
}


CGameObject::~CGameObject()
{
	////if (m_pd2dGeometry) m_pd2dGeometry->Release();
	//if (m_pd2dRcGeometry) m_pd2dRcGeometry->Release();
	////if (m_pd2dsbrDraw) m_pd2dsbrDraw->Release();
	//if (m_pd2dsbrText) m_pd2dsbrText->Release();
	//if (m_pd2dsbrFill) m_pd2dsbrFill->Release();
	////if (m_pd2dBitmap) m_pd2dBitmap->Release();
	//if (m_pdwChat) m_pdwChat->Release();
	//if (m_pdwName) m_pdwName->Release();
}

void CGameObject::SetPosition(float x, float y)
{
	m_fXpos = x;
	m_fYpos = y;
	m_d2dmtxWorld._31 = m_fXpos;
	m_d2dmtxWorld._32 = m_fYpos;
}

void CGameObject::Draw(ID2D1RenderTarget *pd2dRenderTarget)
{
	if (false == m_bView_state) return;
	pd2dRenderTarget->SetTransform(m_d2dmtxWorld);
	if (m_pd2dGeometry && m_pd2dsbrFill) pd2dRenderTarget->FillGeometry(m_pd2dGeometry, m_pd2dsbrFill);
	if (m_pd2dGeometry && m_pd2dsbrDraw) pd2dRenderTarget->DrawGeometry(m_pd2dGeometry, m_pd2dsbrDraw);
	if (m_pd2dBitmap) {
		if (m_d2drcResourceRect.bottom != 0.0f && m_d2drcResourceRect.right != 0.0f) pd2dRenderTarget->DrawBitmap(m_pd2dBitmap, m_d2drcBitmapRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, m_d2drcResourceRect);
		else pd2dRenderTarget->DrawBitmap(m_pd2dBitmap);
	}
}
void CGameObject::DrawWorld(ID2D1RenderTarget *pd2dRenderTarget) {
	if (m_pd2dGeometry && m_pd2dsbrFill) pd2dRenderTarget->FillGeometry(m_pd2dGeometry, m_pd2dsbrFill);
	if (m_pd2dGeometry && m_pd2dsbrDraw) pd2dRenderTarget->DrawGeometry(m_pd2dGeometry, m_pd2dsbrDraw);
	if (m_pd2dBitmap) {
		if (m_d2drcResourceRect.bottom != 0.0f && m_d2drcResourceRect.right != 0.0f) pd2dRenderTarget->DrawBitmap(m_pd2dBitmap, m_d2drcBitmapRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, m_d2drcResourceRect);
		else pd2dRenderTarget->DrawBitmap(m_pd2dBitmap);
	}
}