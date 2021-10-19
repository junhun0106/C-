#include "stdafx.h"
#include "Player.h"


CPlayer::CPlayer() : CGameObject()
{
	m_n_hp = 100;
	m_n_exp = 0;
	m_n_level = 1;

	wmemset(m_wc_id, 0, MAX_STR_SIZE);

	speach_time = 0;
	wmemset(speach_text, 0, 256);

}
CPlayer::~CPlayer()
{
}
void CPlayer::Draw(ID2D1RenderTarget *pd2dRenderTarget)
{
	if (false == m_bView_state) return;
	pd2dRenderTarget->SetTransform(m_d2dmtxWorld);
	if (m_pd2dGeometry && m_pd2dsbrFill) pd2dRenderTarget->FillGeometry(m_pd2dGeometry, m_pd2dsbrFill);
	if (m_pd2dGeometry && m_pd2dsbrDraw) pd2dRenderTarget->DrawGeometry(m_pd2dGeometry, m_pd2dsbrDraw);
	if (m_pd2dBitmap) {
		if (m_d2drcResourceRect.bottom != 0.0f && m_d2drcResourceRect.right != 0.0f) pd2dRenderTarget->DrawBitmap(m_pd2dBitmap, m_d2drcBitmapRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, m_d2drcResourceRect);
		else pd2dRenderTarget->DrawBitmap(m_pd2dBitmap);
	}
	if (m_pdwName) {
		Matrix3x2F mtx = Matrix3x2F::Translation(m_d2dmtxWorld._31 - 24.0f, m_d2dmtxWorld._32 - 40.0f);
		pd2dRenderTarget->SetTransform(mtx);
		pd2dRenderTarget->DrawTextW(m_wc_id, MAX_STR_SIZE, m_pdwName, RectF(0.0f, 0.0f, 55.0f, 20.0f), m_pd2dsbrText);
	}
	if (speach_time > GetTickCount() - 3000) {
		if (m_pdwChat) {
			Matrix3x2F d2dtextmtx = Matrix3x2F::Translation(m_d2dmtxWorld._31 - 34.0f, m_d2dmtxWorld._32 - 76.0f);
			pd2dRenderTarget->SetTransform(d2dtextmtx);
			pd2dRenderTarget->DrawTextW(speach_text, 256, m_pdwChat, RectF(0.0f, 0.0f, 256.0f, 256.0f), m_pd2dsbrText);
			wmemset(speach_text, 0, 256);
		}
	}
}
void CPlayer::BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory, IWICImagingFactory *pwicFactory)
{
	m_pd2dFactory = pd2dFactory;
	if (pd2dFactory) pd2dFactory->AddRef();
	m_pdwFactory = pdwFactory;
	if (pdwFactory) pdwFactory->AddRef();
	m_pwicFactory = pwicFactory;
	if (pwicFactory) pwicFactory->AddRef();
	m_pd2dRenderTarget = pd2dRenderTarget;
	if (pd2dRenderTarget) pd2dRenderTarget->AddRef();

	float fRect = 20.0f;
	D2D1_RECT_F d2drcBitMapRect = RectF(-fRect, -fRect, fRect, fRect);
	D2D1_RECT_F d2drcResourceRect = RectF(0.0f, 0.0f, 110.0f, 110.0f);
	m_d2drcBitmapRect = d2drcBitMapRect;
	m_d2drcResourceRect = d2drcResourceRect;

	m_pdwFactory->CreateTextFormat(L"굴림체", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15.0F, L"en-us", &m_pdwChat);
	m_pdwFactory->CreateTextFormat(L"휴먼매직체", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15.0f, L"en-us", &m_pdwName);
	m_pd2dFactory->CreateRectangleGeometry(RectF(0.0f, 0.0f, 80.0f, 20.0f), &m_pd2dRcGeometry);
	m_pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(ColorF::Black, 1.0f), &m_pd2dsbrFill);
	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::BlueViolet, 1.0f), &m_pd2dsbrText);
}
void CPlayer::Animate(float fTime)
{

}
void CPlayer::ReleaseObjects()
{

}