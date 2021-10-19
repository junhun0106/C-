#pragma once
class CGameObject
{
protected:
	ID2D1Factory					*m_pd2dFactory;
	IWICImagingFactory				*m_pwicFactory;
	IDWriteFactory					*m_pdwFactory;
	ID2D1HwndRenderTarget			*m_pd2dRenderTarget;

	float m_fXpos;
	float m_fYpos;

	ID2D1Geometry *m_pd2dGeometry;
	ID2D1RectangleGeometry *m_pd2dRcGeometry;
	ID2D1SolidColorBrush *m_pd2dsbrDraw;
	ID2D1SolidColorBrush *m_pd2dsbrText;
	ID2D1SolidColorBrush *m_pd2dsbrFill;

	IDWriteTextFormat *m_pdwName;
	IDWriteTextFormat *m_pdwChat;

	ID2D1Bitmap *m_pd2dBitmap;
	D2D1_RECT_F m_d2drcResourceRect;
	D2D1_RECT_F m_d2drcBitmapRect;

	bool m_bView_state;

	D2D1::Matrix3x2F m_d2dmtxWorld;
public:
	CGameObject();
	~CGameObject();

	float GetXpos() { return m_d2dmtxWorld._31; }
	float GetYpos() { return m_d2dmtxWorld._32; }

	void SetPosition(float fXpos, float fYpos);
	void SetGeometry(ID2D1Geometry *pd2dGeometry) { m_pd2dGeometry = pd2dGeometry; }
	void SetSolidColorBrush(ID2D1SolidColorBrush *pd2dsbrDraw, ID2D1SolidColorBrush *pd2dsbrFill) { m_pd2dsbrDraw = pd2dsbrDraw; m_pd2dsbrFill = pd2dsbrFill; }
	void SetBitmap(ID2D1Bitmap *pd2dBitmap) { m_pd2dBitmap = pd2dBitmap; }
	void SetBitmapRect(D2D1_RECT_F d2drcResource, D2D1_RECT_F d2drcBitmap) {
		m_d2drcResourceRect = d2drcResource;  m_d2drcBitmapRect = d2drcBitmap;
	}
	void SetViewState(bool b) { m_bView_state = b; }
	void SetTextFormat(IDWriteTextFormat *pdwFormat) { m_pdwChat = pdwFormat; }
	bool GetViewState() { return m_bView_state; }

	virtual void Animate(float fTimeElapsed) {}
	virtual void Draw(ID2D1RenderTarget *pd2dRenderTartget);
	void DrawWorld(ID2D1RenderTarget *pd2dRenderTarget);
	virtual void BulidObject(ID2D1Factory *pd2dFactory, ID2D1HwndRenderTarget *pd2dRenderTarget, IDWriteFactory *pdwFactory = NULL, IWICImagingFactory *pwicFactory = NULL) {}
	virtual void ReleaseObjects() {}

};

