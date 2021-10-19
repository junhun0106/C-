#include "stdafx.h"
#include "UIScene.h"

CUIScene::CUIScene() : CScene()
{
	m_pPlayer = nullptr;

	m_pgo_player_exp = nullptr;
	m_pgo_player_hp = nullptr;

	m_pd2drc_hp_rect = nullptr;
	m_pd2drc_exp_rect = nullptr;

	m_pd2dsbr_text_color = nullptr;
	m_pd2dsbr_rect_color = nullptr;

	m_pdw_text_format = nullptr;

	for (auto i = 0; i < 256; ++i) {
		mess[i] = (wchar_t)0;
	}
}
CUIScene::~CUIScene()
{
	if (m_pgo_player_exp) delete m_pgo_player_exp;
	if (m_pgo_player_hp) delete m_pgo_player_hp;
	if (m_pd2drc_hp_rect) m_pd2drc_hp_rect->Release();
	if (m_pd2drc_exp_rect) m_pd2drc_exp_rect->Release();
	if (m_pd2dsbr_rect_color) m_pd2dsbr_rect_color->Release();
	if (m_pd2dsbr_text_color) m_pd2dsbr_text_color->Release();

	if (m_pdw_text_format) m_pdw_text_format->Release();

}
void CUIScene::ReleaseObjects() {

}
void CUIScene::BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory, IWICImagingFactory *pwicFactory)
{
	m_pd2dFactory = pd2dFactory;
	if (pd2dFactory) pd2dFactory->AddRef();
	m_pdwFactory = pdwFactory;
	if (pdwFactory) pdwFactory->AddRef();
	m_pwicFactory = pwicFactory;
	if (pwicFactory) pwicFactory->AddRef();
	m_pd2dRenderTarget = pd2dRenderTarget;
	if (pd2dRenderTarget) pd2dRenderTarget->AddRef();

	m_pd2dFactory->CreateRectangleGeometry(RectF(10.0f, 10.0f, 120.0f, 60.0f), &m_pd2drc_hp_rect);
	m_pd2dFactory->CreateRectangleGeometry(RectF(0.0f, 0.0f, FRAME_BUFFER_HEIGHT, 50.0f), &m_pd2drc_exp_rect);
	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::Red, 1.0f), &m_pd2dsbr_rect_color);
	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::Black, 1.0f), &m_pd2dsbr_text_color);
	m_pgo_player_hp = new CGameObject();
	m_pgo_player_hp->SetPosition(0.0f, 0.0f);
	m_pgo_player_hp->SetGeometry(m_pd2drc_hp_rect);
	m_pgo_player_hp->SetSolidColorBrush(m_pd2dsbr_text_color, m_pd2dsbr_rect_color);

	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::Gray, 0.8f), &m_pd2dsbr_rect_color);
	m_pgo_player_exp = new CGameObject();
	m_pgo_player_exp->SetPosition(0.0f, FRAME_BUFFER_HEIGHT - 50.0f);
	m_pgo_player_exp->SetGeometry(m_pd2drc_exp_rect);
	m_pgo_player_exp->SetSolidColorBrush(m_pd2dsbr_text_color, m_pd2dsbr_rect_color);

	pdwFactory->CreateTextFormat(L"휴먼매직체", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"en-us", &m_pdw_text_format);
	m_pdw_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_pdw_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}
void CUIScene::Draw(ID2D1RenderTarget *pd2dRenderTartget) {
	m_pgo_player_hp->Draw(pd2dRenderTartget);
	wmemset(mess, 0, 256);
	wsprintf(mess, L"hp : %d", m_pPlayer->GetHp());
	pd2dRenderTartget->DrawTextW(mess, 256, m_pdw_text_format, RectF(10.0f, 10.0f, 120.0f, 60.0f), m_pd2dsbr_text_color);
	m_pgo_player_exp->Draw(pd2dRenderTartget);
	wmemset(mess, 0, 256);
	wsprintf(mess, L"Level : %d  exp : %d", m_pPlayer->GetLevel(), m_pPlayer->GetExp());
	Matrix3x2F mtx = Matrix3x2F::Translation(0.0f, FRAME_BUFFER_HEIGHT - 10.0f);
	pd2dRenderTartget->SetTransform(mtx);
	pd2dRenderTartget->DrawTextW(mess, 256, m_pdw_text_format, RectF(0.0f, 0.0f, FRAME_BUFFER_HEIGHT, -30.0f), m_pd2dsbr_text_color);
}