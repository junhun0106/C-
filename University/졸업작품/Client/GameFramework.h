#pragma once

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "ConnectServer.h"
#include "ShadowMap.h"

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	bool CreateRenderTargetDepthStencilView();
	bool CreateDirect3DDisplay();

	void CreateShaderVariables();
	void ReleaseShaderVariables();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects(CConnectServer *server_info);
	void FrameAdvance(CConnectServer *server_info);

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void SetSocket(SOCKET sock) { m_socket = sock; }

	// blurring
	BYTE * ReadCompiledShaderCode(ID3D11Device *pd3dDevice, WCHAR *pszFileName, int& nSize);
	void CreateBlurringShader(ID3D11Device *pd3dDevice);
	void CreateBlurringResources(ID3D11Device *pd3dDevice);
	void SceneBlurring(int nBlurCount);
	void DrawBlurredSceneToScreen();

	// Shadow
	void OnCreateShadowMap(ID3D11DeviceContext* pd3dDeviceContext, CConnectServer *server_info);

	// Lobby
	void CreateLobbyShader(ID3D11Device *pd3dDevice);

	//ksh
	void sendJoinParty();
	void exitParty();
	void sendInviteParty();
	void sendNoParty();
	//ksh
	// Direct 2D
	void Build2DObjects();
	void BuildChttingBoxObjects();
	void BuildPartyWindowObjects();
	void BuildMonsterHPObjects();
	void BuildUIObjects();
	void Release2DObjects();
	void RenderChattingBox(CConnectServer *server_info);
	void RenderMonsterHp(CConnectServer *server_info);
	void RenderPartyWindow(CConnectServer *server_info);
	void RenderUI(CConnectServer *server_info);
	void RenderPartUI(CConnectServer *server_info);
	void Render2D(CConnectServer *server_info);
	bool LoadImageFromFile(_TCHAR *pszstrFileName, ID2D1Bitmap1 **ppd2dBitmap, D2D_RECT_U *pd2drcImage, UINT nWidth, UINT nHeight, WICBitmapTransformOptions nFlipRotation);
private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd;
	HMENU							m_hMenu;

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	ID3D11Device					*m_pd3dDevice;
	IDXGISwapChain					*m_pDXGISwapChain;
	ID3D11RenderTargetView			*m_pd3dRenderTargetView;
	ID3D11DeviceContext				*m_pd3dDeviceContext;

	

	UINT							m_n4xMSAAQualities;

	ID3D11Texture2D					*m_pd3dDepthStencilBuffer;
	ID3D11DepthStencilView			*m_pd3dDepthStencilView;

	CGameTimer						m_GameTimer;

	CScene							*m_pScene;

	CPlayer							*m_pPlayer;
	CCamera							*m_pCamera;

	POINT							m_ptOldCursorPos;
	_TCHAR							m_pszBuffer[50];

    float                           m_fRefireTime = 0.0f;

    // minimap
    CMinimapScene                   *m_pMinimapScene;
    CCamera                         *m_pMinimapCamera;

	WSABUF							m_wsa_send_buf;
	char							m_csend_buf[BUFF_SIZE];
	SOCKET							m_socket;

	int anim_state;

	// Post Processing: blurring
	ID3D11ComputeShader				*m_pHorizontalBlurShader;
	ID3D11ComputeShader				*m_pVerticalBlurShader;

	ID3D11ShaderResourceView		*m_pd3dsrvOffScreen;
	ID3D11UnorderedAccessView		*m_pd3duavOffScreen;
	ID3D11RenderTargetView			*m_pd3drtvOffScreen;
	ID3D11ShaderResourceView		*m_pd3dsrvTexture;
	ID3D11UnorderedAccessView		*m_pd3duavTexture;

	bool							m_bBlur;

	// Shadow
	ShadowMap						*m_pShadowMap;
	D3DXMATRIX						m_d3dxmtxShadowMap;
	D3DXMATRIX						m_d3dxmtxShadowView;
	D3DXMATRIX						m_d3dxmtxShadowProjection;
	ID3D11SamplerState				*m_pd3dShadowSS;
	ID3D11SamplerState				*m_pd3dShadowPCFSS;
	ID3D11RasterizerState			*m_pd3dShadowRS;
	ID3D11Buffer					*m_pd3dcbShadow;


	// Direct 2D

	ID2D1Factory1 *m_pd2dFactory{ nullptr };
	ID2D1Device *m_pd2dDevice{ nullptr };
	ID2D1DeviceContext *m_pd2dContext{ nullptr };
	IDWriteFactory *m_pdwFactory{ nullptr };
	IWICImagingFactory *m_pwicFactory{ nullptr };


	// -- 채팅
	ID2D1RectangleGeometry *m_pd2drcOutputChatbox{ nullptr };
	ID2D1RectangleGeometry *m_pd2drcInputChatbox{ nullptr };
	ID2D1SolidColorBrush *m_pd2dsbrChatboxColor{ nullptr };
	IDWriteTextFormat *m_dwMyChattingFormat{ nullptr };
	IDWriteTextFormat *m_dwOtherChattingFormat{ nullptr };


	// - HP, Level, exp

	IDWriteTextFormat *m_dwStatusTextFormat{ nullptr };
	ID2D1SolidColorBrush *m_pd2dsbrBlackColor{ nullptr };
	ID2D1SolidColorBrush *m_pd2dsbrYellowColor{ nullptr };

	// -- Party Window
	POINT m_ptCursorPOS;
	POINT m_pt_mouse_move_position;
	POINT m_pt_mouse_positoin;

	enum BITMAP {
		OFF_BUTTON = 0,
		ON_BUTTON = 1,
		ARROW_RIGHT_BUTTON = 2,
		ARROW_LEFT_BUTTON = 3,
		PARTY_WINDOW = 4
	};
	ID2D1Bitmap1 **m_ppd2dButtonBitmap{ nullptr };
	int m_nBitmaps;

	// MONTER HP
	ID2D1SolidColorBrush *m_pd2dsbrRedColor{ nullptr };
	vector<ID2D1RectangleGeometry *> m_vpd2drcMonsterHp;
	ID2D1RectangleGeometry* m_pd2drcHpbar;
	vector<pair<int, D3DXVECTOR2>> m_vd3dvec2;


	// 파티
	ID2D1RectangleGeometry *m_prcParty{ nullptr };
	ID2D1SolidColorBrush *m_pd2dsbrGray{ nullptr };
	IDWriteTextFormat *m_pdwPartyText{ nullptr };

	// 박종혁

	IDWriteTextFormat *m_dwTextFormat{ nullptr };
	ID2D1SolidColorBrush *m_pd2dsbrColor{ nullptr };

	ID2D1Bitmap1 *m_ppd2dLoadingImage[2]{ nullptr, nullptr };

	//bitmap
	ID2D1Bitmap1 *m_pp2dMainImage[4]{ nullptr, nullptr, nullptr, nullptr };
	ID2D1Bitmap1 *m_pp2dSelectImage[6]{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ID2D1Bitmap1 *m_pp2dArrowImage[4]{ nullptr, nullptr, nullptr ,nullptr };

	// Lobby
	ID3D11InputLayout *m_pd3dInputLayout{ nullptr };
	ID3D11VertexShader *m_pd3dVS_Lobby{ nullptr };
	ID3D11PixelShader *m_pd3dPS_Lobby{ nullptr };

	void Render2D();

	void RenderLobbyButton();
};
