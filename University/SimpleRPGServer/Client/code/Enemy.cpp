#include "stdafx.h"
#include "Enemy.h"


CEnemy::CEnemy() : CPlayer()
{
	m_prc_hp_rect = nullptr;
}
CEnemy::~CEnemy()
{
	if (m_prc_hp_rect) m_prc_hp_rect->Release();
}

void CEnemy::Animate(float fTimeElapsed)
{

}
void CEnemy::Draw(ID2D1RenderTarget *pd2dRenderTarget)
{
	if (false == m_bView_state) return;
	pd2dRenderTarget->SetTransform(m_d2dmtxWorld);
	if (m_pd2dBitmap) {
		 pd2dRenderTarget->DrawBitmap(m_pd2dBitmap, m_d2drcBitmapRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, m_d2drcResourceRect);
	}
	if (m_prc_hp_rect) {
		Matrix3x2F d2dtextmtx = Matrix3x2F::Translation(m_d2dmtxWorld._31 - (m_n_hp / 2), m_d2dmtxWorld._32 - 40.0f);
		pd2dRenderTarget->SetTransform(d2dtextmtx);
		ID2D1Factory *pd2dFactory;
		m_prc_hp_rect->GetFactory(&pd2dFactory);
		pd2dFactory->CreateRectangleGeometry(RectF(0.0f, 0.0f, m_n_hp, 20.0f), &m_prc_hp_rect);
		pd2dRenderTarget->FillGeometry(m_prc_hp_rect, m_pd2dsbr_hp_color);
	}
	if (m_pdwName) {
		Matrix3x2F d2dtextmtx = Matrix3x2F::Translation(m_d2dmtxWorld._31 - 40.0f, m_d2dmtxWorld._32 - 40.0f);
		pd2dRenderTarget->SetTransform(d2dtextmtx);
		WCHAR text[MAX_STR_SIZE];
		wmemset(text, 0, MAX_STR_SIZE);
		wsprintf(text, L"%ws", npc_name);
		int size = wcslen(text);
		pd2dRenderTarget->DrawTextW(text, MAX_STR_SIZE, m_pdwName, RectF(0.0f, 0.0f, 100.0f, 20.0f), m_pd2dsbrDraw);
	}	
	if (speach_time > GetTickCount() - 3000) {
		if (m_pdwChat) {
			Matrix3x2F d2dtextmtx = Matrix3x2F::Translation(m_d2dmtxWorld._31 - 34.0f, m_d2dmtxWorld._32 - 60.0f);
			pd2dRenderTarget->SetTransform(d2dtextmtx);
			pd2dRenderTarget->FillGeometry(m_pd2dRcGeometry, m_pd2dsbrFill);
			pd2dRenderTarget->DrawTextW(speach_text, 256, m_pdwChat, RectF(0.0f, 0.0f, 80.0f, 20.0f), m_pd2dsbrText);
			wmemset(speach_text, 0, 256);
		}
	}
}
void CEnemy::BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory, IWICImagingFactory *pwicFactory)
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
	D2D1_RECT_F d2drcResourceRect = RectF(0.0f, 0.0f, 31.0f, 36.0f);

	pdwFactory->CreateTextFormat(L"굴림체", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15.0F, L"en-us", &m_pdwName);
	pdwFactory->CreateTextFormat(L"굴림체", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15.0F, L"en-us", &m_pdwChat);
	//m_pdwName->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	//m_pdwName->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	m_pdwChat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_pdwChat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	
	pd2dFactory->CreateRectangleGeometry(RectF(0.0f, 0.0f, 55.0f, 20.0f), &m_prc_hp_rect);
	pd2dFactory->CreateRectangleGeometry(RectF(0.0f, 0.0f, 80.0f, 20.0f), &m_pd2dRcGeometry);
	pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_pd2dsbrDraw);
	pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &m_pd2dsbrText);
	pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(ColorF::Black, 1.0f), &m_pd2dsbrFill);
	pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::Red, 0.8f), &m_pd2dsbr_hp_color);
	m_d2drcBitmapRect = d2drcBitMapRect;
	m_d2drcResourceRect = d2drcResourceRect;
}
void CEnemy::ReleaseObjects()
{
}