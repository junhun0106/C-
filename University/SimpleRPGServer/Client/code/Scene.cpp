#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{
	m_pd2dGeometry = NULL;
	m_pd2dRcGeometry = NULL;
	m_pd2dsbrDraw = NULL;
	m_pd2dsbrFill = NULL;
	m_pd2dsbrPoint = nullptr;

	m_ppRect = nullptr;

	m_nObject = 0;

}
CScene::~CScene()
{
	if (m_pd2dGeometry) m_pd2dGeometry->Release();
	if (m_pd2dRcGeometry) m_pd2dRcGeometry->Release();
	if (m_pd2dsbrDraw)m_pd2dsbrDraw->Release();
	if (m_pd2dsbrFill) m_pd2dsbrFill->Release();
	if (m_pd2dsbrPoint) m_pd2dsbrPoint->Release();

	if (m_ppRect) {
		for (auto i = 0; i < m_nObject; ++i) {
			delete m_ppRect[i];
		}
		delete[] m_ppRect;
	}

}
void CScene::Draw(ID2D1RenderTarget *pd2dRenderTarget)
{
	for (int i = 0; i < m_nObject; i++)
	{
		float Object_Xpos = m_ppRect[i]->GetXpos() / 40;
		float Object_Ypos = m_ppRect[i]->GetYpos() / 40;
		float player_x = (m_fPlayerPos.first / 40);
		float player_y = (m_fPlayerPos.second / 40);
		float deltaX = player_x - (Object_Xpos);
		float deltaY = player_y - (Object_Ypos);
		float dist = deltaX * deltaX + deltaY * deltaY;
		if (dist < 10 * 10) {
			float vector_x = (420.0f - m_fPlayerPos.first); // 현재 위치에서 중앙으로 가야 하는 위치.
			float vector_y = (420.0f - m_fPlayerPos.second); 
			D2D1_POINT_2F pos, last;
			pos.x = m_ppRect[i]->GetXpos();
			pos.y = m_ppRect[i]->GetYpos();
			last.x = pos.x + vector_x; // 플레이어가 가야하는 위치 만큼 평행이동 시킨다.
			last.y = pos.y + vector_y;
			Matrix3x2F mtx = Matrix3x2F::Translation(last.x, last.y);
			pd2dRenderTarget->SetTransform(mtx);
			m_ppRect[i]->DrawWorld(pd2dRenderTarget);
		}
	}

}
void CScene::BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory, IWICImagingFactory *pwicFactory)
{
	m_pd2dFactory = pd2dFactory;
	if (pd2dFactory) pd2dFactory->AddRef();
	m_pdwFactory = pdwFactory;
	if (pdwFactory) pdwFactory->AddRef();
	m_pwicFactory = pwicFactory;
	if (pwicFactory) pwicFactory->AddRef();
	m_pd2dRenderTarget = pd2dRenderTarget;
	if (pd2dRenderTarget) pd2dRenderTarget->AddRef();

	m_nObject = 80000 + 900 + 5000;
	m_ppRect = new CGameObject*[m_nObject];

	float fRect = 20.0f;
	m_pd2dFactory->CreateRectangleGeometry(RectF(-fRect, -fRect, fRect, fRect), &m_pd2dRcGeometry);
	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::Black, 1.0f), &m_pd2dsbrDraw);
	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::White, 1.0f), &m_pd2dsbrFill);
	m_pd2dRenderTarget->CreateSolidColorBrush(ColorF(ColorF::Blue, 1.0f), &m_pd2dsbrPoint);
	int j = 0, t = 0;
	int nObject = 0;

	// rock
	int x = 99 * 40 + 20.0f;
	for (auto i = 0; i < 300; ++i) {
		m_ppRect[nObject] = new CGameObject();
		m_ppRect[nObject]->SetPosition(x, i * 40 + 20.0f);
		m_ppRect[nObject]->SetBitmapRect(RectF(0.0f, 0.0f, 150.0f, 150.0f), RectF(-fRect, -fRect, fRect, fRect));
		nObject++;
	}

	x = 299 * 40 + 20.0f;
	for (auto i = 0; i < 300; ++i) {
		m_ppRect[nObject] = new CGameObject();
		m_ppRect[nObject]->SetPosition(x, i * 40 + 20.0f);
		m_ppRect[nObject]->SetBitmapRect(RectF(0.0f, 0.0f, 150.0f, 150.0f), RectF(-fRect, -fRect, fRect, fRect));
		m_ppRect[nObject]->SetSolidColorBrush(m_pd2dsbrPoint, m_pd2dsbrPoint);
		nObject++;
	}

	x = 199 * 40 + 20.0f;
	for (auto i = 400; i > 100; --i) {
		m_ppRect[nObject] = new CGameObject();
		m_ppRect[nObject]->SetPosition(x, i * 40 + 20.0f);
		m_ppRect[nObject]->SetBitmapRect(RectF(0.0f, 0.0f, 150.0f, 150.0f), RectF(-fRect, -fRect, fRect, fRect));
		m_ppRect[nObject]->SetSolidColorBrush(m_pd2dsbrPoint, m_pd2dsbrPoint);
		nObject++;
	}

	// 바닥
	for (int k = 0; k < 200; ++k)
	{
		for (int i = 0; i < 200; ++i)
		{
			m_ppRect[nObject] = new CGameObject();
			m_ppRect[nObject]->SetPosition((j * (fRect * 2)) + fRect, (t * (fRect * 2)) + fRect);
			m_ppRect[nObject]->SetGeometry(m_pd2dRcGeometry);
			if (k % 10 != 0) {
				m_ppRect[nObject]->SetSolidColorBrush(m_pd2dsbrDraw, NULL);
			}
			else if (k % 10 == 0) m_ppRect[nObject]->SetSolidColorBrush(m_pd2dsbrPoint, m_pd2dsbrPoint);
			j += 2;
			nObject++;
		}
		j = 0;
		t += 2;
	}
	j = 1;
	t = 1;
	for (int k = 0; k < 200; ++k)
	{
		for (int i = 0; i < 200; ++i)
		{
			m_ppRect[nObject] = new CGameObject();
			m_ppRect[nObject]->SetPosition((j * fRect * 2.0f) + fRect, (t * fRect*2.0f) + fRect);
			m_ppRect[nObject]->SetGeometry(m_pd2dRcGeometry);
			if (i % 10 != 0) {
				m_ppRect[nObject]->SetSolidColorBrush(m_pd2dsbrDraw, NULL);
			}
			else {
				m_ppRect[nObject]->SetSolidColorBrush(m_pd2dsbrPoint, m_pd2dsbrPoint);
			}
			j += 2;
			nObject++;
		}
		j = 1;
		t += 2;
	}

	// tree
	std::ifstream in_file("obtacle.txt");
	int x_pos, y_pos;
	x_pos = y_pos = 0;
	for (auto i = 0; i < 5000; ++i) {
		m_ppRect[nObject] = new CGameObject();
		in_file >> x_pos >> y_pos;
		m_ppRect[nObject]->SetPosition(x_pos * 40.0f + 20.0f, y_pos * 40.0f + 20.0f);
		m_ppRect[nObject]->SetBitmapRect(RectF(0.0f, 0.0f, 150.0f, 150.0f), RectF(-fRect, -fRect, fRect, fRect));
		nObject++;
	}
	in_file.close();
}
void CScene::ReleaseObjects()
{

}
