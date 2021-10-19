#pragma once


#include "Timer.h"
#include "Scene.h"
#include "ConnectServer.h"
#include "UIScene.h"
#include "Enemy.h"

struct PlayerPos {
	float xPos;
	float yPos;
};

class CFramework
{

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HMENU m_hMenu;

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;
	float							m_fLimittedWidth;
	float							m_fLimittedHeight;
	int m_nMy_id;
	CTimer							*m_pTimer;

	ID2D1Factory					*m_pd2dFactory;
	IWICImagingFactory				*m_pwicFactory;
	IDWriteFactory					*m_pdwFactory;
	ID2D1HwndRenderTarget			*m_pd2dRenderTarget;

	IDWriteTextFormat				*m_pdwPositionText;
	ID2D1SolidColorBrush			*m_pd2dsbrBlack;
	POINT							m_ptCursorPos;
	POINT							m_ptOldCursorPos;
	_TCHAR							m_pszBuffer[50];

	ID2D1Bitmap *m_pd2dBitmap;

	CPlayer							*m_pPlayer;
	CPlayer							**m_pOther_player;

	CEnemy							**m_pEnemy_npc;

	CScene							*m_pScene;
	CScene							*m_p_ui_scene;

	PlayerPos						playerpos;
	PlayerPos						*m_pOtherPos;
	std::vector<CPlayer*>			m_vOtherPlayer;
	std::vector<CEnemy*>			m_vEnemyNpc;

	ID2D1Bitmap **m_ppd2dBitmap;
	int m_nBitmap;


	float m_fXpos;
	float m_fYpos;

	int state;
	int m_nPlayerCount;

	WSABUF		m_wsa_send_buf;
	char		m_csend_buf[BUFF_SIZE];

	SOCKET m_client_sock;

	std::wstring str;
	bool chat_active;
	bool collsion;

public:
	CFramework();
	~CFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	bool CreateDeviceIndependentResources();
	bool CreateDeviceDependentResources();
	void ReleaseDeviceIndependentResources();
	void ReleaseDeviceDependentResources();

	ID2D1Factory *GetD2DFactory() { return(m_pd2dFactory); }
	bool LoadImageFromFileLoadImageFromFile(_TCHAR *pszstrFileName, ID2D1Bitmap **ppd2dBitmap, D2D_RECT_U *pd2drcImage, UINT nWidth, UINT nHeight, WICBitmapTransformOptions nFlipRotation);

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects(CConnectServer*);
	void FrameAdvance(CConnectServer* server_info);

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void SetSokcet(SOCKET sock) { m_client_sock = sock; }
};

