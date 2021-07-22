//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "Shader.h"
#include "SkinnedMesh.h"

bool g_debug = false;
bool g_debugCursor = false;
bool g_zoom = false;
bool g_chat_active = false;
bool g_party = false;
bool g_bBlood = false;

int g_GameState = GAME_STATE_LOADING;
volatile bool g_bBuildFinish = false;
//박종혁
bool g_bSelectWindow = false;
bool g_bExit = false;
bool g_bStart = false;
bool g_bClassSelect = false;
bool g_bRightArrow = false;
bool g_bLeftArrow = false;


//ksh
#include "resource.h"
extern SOCKET party_socket;
extern cs_packet_db db_packet;
CGameFramework party;
extern WCHAR in_name[200];
extern WCHAR jo_name[200];

extern HINSTANCE g_hInst;
extern TCHAR ipTmp[200];
extern HWND hIp;
extern TCHAR tc_ip[200];

wstring id;

extern member party_members[4];
//ksh


wstring g_chat_massege;

bool g_full = false;

float particleCreateRange;

CGameFramework::CGameFramework()
{
    m_pd3dDevice = NULL;
    m_pDXGISwapChain = NULL;
    m_pd3dRenderTargetView = NULL;
    m_pd3dDeviceContext = NULL;

    m_pd3dDepthStencilBuffer = NULL;
    m_pd3dDepthStencilView = NULL;

    m_pScene = NULL;

    m_pPlayer = NULL;
    m_pCamera = NULL;

    m_pMinimapScene = NULL;
    m_pMinimapCamera = NULL;

    m_nWndClientWidth = FRAME_BUFFER_WIDTH;
    m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

    m_ptOldCursorPos.x = (int)(GetSystemMetrics(SM_CXSCREEN) * 0.5f);
    m_ptOldCursorPos.y = (int)(GetSystemMetrics(SM_CYSCREEN) * 0.5f);
    //ShowCursor(false);

    m_wsa_send_buf.buf = m_csend_buf;
    m_wsa_send_buf.len = BUFF_SIZE;


    _tcscpy_s(m_pszBuffer, _T("LabProject ("));

    srand(timeGetTime());

	// blurring
	m_pHorizontalBlurShader = nullptr;
	m_pVerticalBlurShader = nullptr;
	m_pd3dsrvOffScreen = nullptr;
	m_pd3duavOffScreen = nullptr;
	m_pd3drtvOffScreen = nullptr;
	m_pd3dsrvTexture = nullptr;
	m_pd3duavTexture = nullptr;

	m_bBlur = false;
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	//ksh
	for (int i = 0; i < 3; ++i) {
		//&party_members[i] = NULL;
		party_members[i].pHP = -5;
	}
	//ksh
    m_hInstance = hInstance;
    m_hWnd = hMainWnd;

    if (!CreateDirect3DDisplay()) return(false);

	//BuildObjects();
	Build2DObjects();

    return(true);
}

bool CGameFramework::CreateDirect3DDisplay()
{
    RECT rcClient;
    ::GetClientRect(m_hWnd, &rcClient);
    m_nWndClientWidth = rcClient.right - rcClient.left;
    m_nWndClientHeight = rcClient.bottom - rcClient.top;

    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    ::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
    dxgiSwapChainDesc.BufferCount = 1;
    dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
    dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
    dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.OutputWindow = m_hWnd;
    dxgiSwapChainDesc.SampleDesc.Count = 1;
    dxgiSwapChainDesc.SampleDesc.Quality = 0;
    dxgiSwapChainDesc.Windowed = TRUE;
    dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH/*0*/;

    UINT dwCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    dwCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE d3dDriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };
    UINT nDriverTypes = sizeof(d3dDriverTypes) / sizeof(D3D_DRIVER_TYPE);

    D3D_FEATURE_LEVEL pd3dFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT nFeatureLevels = sizeof(pd3dFeatureLevels) / sizeof(D3D_FEATURE_LEVEL);

    D3D_DRIVER_TYPE nd3dDriverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL nd3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hResult = S_OK;
#ifdef _WITH_DEVICE_AND_SWAPCHAIN
    for (UINT i = 0; i < nDriverTypes; i++)
    {
        nd3dDriverType = d3dDriverTypes[i];
        if (SUCCEEDED(hResult = D3D11CreateDeviceAndSwapChain(NULL, nd3dDriverType, NULL, dwCreateDeviceFlags, pd3dFeatureLevels, nFeatureLevels, D3D11_SDK_VERSION, &dxgiSwapChainDesc, &m_pDXGISwapChain, &m_pd3dDevice, &nd3dFeatureLevel, &m_pd3dDeviceContext))) break;
    }
#else
    for (UINT i = 0; i < nDriverTypes; i++)
    {
        nd3dDriverType = d3dDriverTypes[i];
        if (SUCCEEDED(hResult = D3D11CreateDevice(NULL, nd3dDriverType, NULL, dwCreateDeviceFlags, pd3dFeatureLevels, nFeatureLevels, D3D11_SDK_VERSION, &m_pd3dDevice, &nd3dFeatureLevel, &m_pd3dDeviceContext))) break;
    }
    if (!m_pd3dDevice) return(false);

    if (FAILED(hResult = m_pd3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_n4xMSAAQualities))) return(false);
#ifdef _WITH_MSAA4_MULTISAMPLING
    dxgiSwapChainDesc.SampleDesc.Count = 4;
    dxgiSwapChainDesc.SampleDesc.Quality = m_n4xMSAAQualities - 1;
#else
    dxgiSwapChainDesc.SampleDesc.Count = 1;
    dxgiSwapChainDesc.SampleDesc.Quality = 0;
#endif
	IDXGIFactory1 *pdxgiFactory = NULL;
	UINT udxgiFlag = 0;
	if (FAILED(hResult = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (LPVOID*)&pdxgiFactory))) return(false);
	IDXGIDevice2 *pdxgiDevice = NULL;
	if (FAILED(hResult = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice2), (LPVOID*)&pdxgiDevice))) return(false);
	if (FAILED(hResult = pdxgiFactory->CreateSwapChain(pdxgiDevice, &dxgiSwapChainDesc, &m_pDXGISwapChain))) return(false);

	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	hResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, reinterpret_cast<LPVOID*>(&m_pd2dFactory));
	hResult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_pdwFactory);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	hResult = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pwicFactory));
	hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, &m_pd2dDevice);
	hResult = m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dContext);


	if (pdxgiDevice) pdxgiDevice->Release();
	if (pdxgiFactory) pdxgiFactory->Release();
#endif

    if (!CreateRenderTargetDepthStencilView()) return(false);

    return(true);
}

bool CGameFramework::CreateRenderTargetDepthStencilView()
{
    HRESULT hResult = S_OK;

    ID3D11Texture2D *pd3dBackBuffer;
    if (FAILED(hResult = m_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pd3dBackBuffer))) return(false);
    if (FAILED(hResult = m_pd3dDevice->CreateRenderTargetView(pd3dBackBuffer, NULL, &m_pd3dRenderTargetView))) return(false);
    if (pd3dBackBuffer) pd3dBackBuffer->Release();

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC d3dDepthStencilBufferDesc;
    ZeroMemory(&d3dDepthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
    d3dDepthStencilBufferDesc.Width = m_nWndClientWidth;
    d3dDepthStencilBufferDesc.Height = m_nWndClientHeight;
    d3dDepthStencilBufferDesc.MipLevels = 1;
    d3dDepthStencilBufferDesc.ArraySize = 1;
    d3dDepthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
#ifdef _WITH_MSAA4_MULTISAMPLING
    d3dDepthStencilBufferDesc.SampleDesc.Count = 4;
    d3dDepthStencilBufferDesc.SampleDesc.Quality = m_n4xMSAAQualities - 1;
#else
    d3dDepthStencilBufferDesc.SampleDesc.Count = 1;
    d3dDepthStencilBufferDesc.SampleDesc.Quality = 0;
#endif
    d3dDepthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    d3dDepthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    d3dDepthStencilBufferDesc.CPUAccessFlags = 0;
    d3dDepthStencilBufferDesc.MiscFlags = 0;
    if (FAILED(hResult = m_pd3dDevice->CreateTexture2D(&d3dDepthStencilBufferDesc, NULL, &m_pd3dDepthStencilBuffer))) return(false);

    // SetPrivateData Test
    static char pszName[] = "DepthStencilBuffer";
    //HR()
    HRESULT hr = m_pd3dDepthStencilBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(pszName) - 1, pszName);

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
    ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    d3dDepthStencilViewDesc.Format = d3dDepthStencilBufferDesc.Format;
#ifdef _WITH_MSAA4_MULTISAMPLING
    d3dDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
#else
    d3dDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
#endif
    d3dDepthStencilViewDesc.Texture2D.MipSlice = 0;
    if (FAILED(hResult = m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, &m_pd3dDepthStencilView))) return(false);

    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dRenderTargetView, m_pd3dDepthStencilView);

	CreateBlurringShader(m_pd3dDevice);
	CreateBlurringResources(m_pd3dDevice);

	float fdpiX, fdpiY;
	m_pd2dFactory->GetDesktopDpi(&fdpiX, &fdpiY);

	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
			, fdpiX
			, fdpiY
			);

	IDXGISurface2 *dxgiBackBuffer;
	ID2D1Bitmap1 *pd2dBitmapBackbuffer;
	hResult = m_pDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	hResult = m_pd2dContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties, &pd2dBitmapBackbuffer);

	m_pd2dContext->SetTarget(pd2dBitmapBackbuffer);

	dxgiBackBuffer->Release();
	pd2dBitmapBackbuffer->Release();

    return(true);
}

void CGameFramework::OnDestroy()
{
    ReleaseObjects();
	Release2DObjects();
    CSkinnedMesh::AnimationDestroy();//스키닝메쉬 클래스의 static포인터에 저장된 애니메이션 정보를 삭제

	if (m_pd3dVS_Lobby) m_pd3dVS_Lobby->Release();
	if (m_pd3dPS_Lobby) m_pd3dPS_Lobby->Release();

	// blurring
	if (m_pHorizontalBlurShader) m_pHorizontalBlurShader->Release();
	if (m_pVerticalBlurShader) m_pVerticalBlurShader->Release();
	if (m_pd3dsrvOffScreen) m_pd3dsrvOffScreen->Release();
	if (m_pd3duavOffScreen) m_pd3duavOffScreen->Release();
	if (m_pd3drtvOffScreen) m_pd3drtvOffScreen->Release();
	if (m_pd3dsrvTexture) m_pd3dsrvTexture->Release();
	if (m_pd3duavTexture) m_pd3duavTexture->Release();

    if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState();
    if (m_pd3dRenderTargetView) m_pd3dRenderTargetView->Release();
    if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
    if (m_pd3dDepthStencilView) m_pd3dDepthStencilView->Release();
    if (m_pDXGISwapChain) m_pDXGISwapChain->Release();
    if (m_pd3dDeviceContext) m_pd3dDeviceContext->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
    if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
  
	D2D1_POINT_2F CurrentMouseCursorPoint;
	CurrentMouseCursorPoint.x = LOWORD(lParam);
	CurrentMouseCursorPoint.y = HIWORD(lParam);


	switch (nMessageID)
    {
	case WM_LBUTTONDOWN:
		m_ptCursorPOS.x = LOWORD(lParam);
		m_ptCursorPOS.y = HIWORD(lParam);
		
		if (g_GameState == GAME_STATE_LOBBY)
		{
			if (g_bStart && (g_bSelectWindow == false))
			{
				g_bSelectWindow = true;//메인에서 [시작] 누르면 캐릭터 선택창
				break;
			}
			if (g_bExit & (!g_bSelectWindow))//메인에서 [나가기] 하면 나가짐
			{
				PostQuitMessage(0);
			}
			if (g_bRightArrow)
			{
				break;
			}
			if (g_bLeftArrow)
			{
				break;
			}
			if (g_bClassSelect)
			{
				break;
				//클래스 교체가 이루어져야 됨
			}
			if ((CurrentMouseCursorPoint.x > (m_nWndClientWidth - 300)) && (CurrentMouseCursorPoint.x < (m_nWndClientWidth - 300 + 288)) && (CurrentMouseCursorPoint.y >(m_nWndClientHeight - 300)) && (CurrentMouseCursorPoint.y < (m_nWndClientHeight - 300 + 60)))
			{
				if (g_bSelectWindow == true) {
					g_GameState = GAME_STATE_PLAY;
					g_debugCursor = false;
					ShowCursor(g_debugCursor);
					break;
				}
			}//로비에서 게임 입장 누르면 InGame으로 입장

			if (g_bExit && (g_bSelectWindow == true))
			{
				g_bSelectWindow = false;
				break;
			}//로비에서 나가기 누르면 메인으로 뒤돌아감	
		}
		
		break;


    case WM_RBUTTONDOWN:
        g_zoom ^= true;
        if (g_zoom)
            m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 10.0f);
        else
            m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
        break;
    case WM_LBUTTONUP:
        break;
    case WM_RBUTTONUP:
        break;
	case WM_MOUSEMOVE: {
		m_pt_mouse_move_position.x = LOWORD(lParam);
		m_pt_mouse_move_position.y = HIWORD(lParam);
	}
					   if ((g_GameState == GAME_STATE_LOBBY) && (!g_bSelectWindow))//맨 처음 로비
					   {
						   if ((CurrentMouseCursorPoint.x >(m_nWndClientWidth - 300)) && (CurrentMouseCursorPoint.x < (m_nWndClientWidth - 300 + 288)) && (CurrentMouseCursorPoint.y >(m_nWndClientHeight - 300)) && (CurrentMouseCursorPoint.y < (m_nWndClientHeight - 300 + 60)))
							   g_bStart = true;
						   else
							   g_bStart = false;
						   //Exit, 혹은 뒤로가기
						   if ((CurrentMouseCursorPoint.x >(m_nWndClientWidth - 300)) && (CurrentMouseCursorPoint.x < (m_nWndClientWidth - 300 + 288)) && (CurrentMouseCursorPoint.y >(m_nWndClientHeight - 200)) && (CurrentMouseCursorPoint.y < (m_nWndClientHeight - 200 + 60)))
							   g_bExit = true;
						   else
							   g_bExit = false;
					   }
					   else if ((g_GameState == GAME_STATE_LOBBY) && g_bSelectWindow)//카메라가 선택창에 도착했을때
					   {
						   //마우스 커서는 하나이므로, 최소 하나의 버튼만 켜진다.
						   if ((CurrentMouseCursorPoint.x >((m_nWndClientWidth / 2) - (m_nWndClientWidth / 6.3f) - 64)) && (CurrentMouseCursorPoint.x < ((m_nWndClientWidth / 2) - (m_nWndClientWidth / 6.3f) + 64))
							   && (CurrentMouseCursorPoint.y >((m_nWndClientHeight / 2) - 64)) && (CurrentMouseCursorPoint.y < ((m_nWndClientHeight / 2) + 64)))
							   g_bLeftArrow = true;
						   else
							   g_bLeftArrow = false;

						   //mtx = D2D1::Matrix3x2F::Translation((m_nWndClientWidth / 2) - 64 + (m_nWndClientWidth/8.5f), (m_nWndClientHeight / 2) - 64);
						   if ((CurrentMouseCursorPoint.x >((m_nWndClientWidth / 2) + (m_nWndClientWidth / 8.5f) - 64)) && (CurrentMouseCursorPoint.x < ((m_nWndClientWidth / 2) + (m_nWndClientWidth / 8.5f) + 64))
							   && (CurrentMouseCursorPoint.y >((m_nWndClientHeight / 2) - 64)) && (CurrentMouseCursorPoint.y < ((m_nWndClientHeight / 2) + 64)))
							   g_bRightArrow = true;
						   else
							   g_bRightArrow = false;
						   //맨 위 Class버튼
						   if ((CurrentMouseCursorPoint.x >(m_nWndClientWidth - 300)) && (CurrentMouseCursorPoint.x < (m_nWndClientWidth - 300 + 288)) && (CurrentMouseCursorPoint.y >(m_nWndClientHeight - 400)) && (CurrentMouseCursorPoint.y < (m_nWndClientHeight - 400 + 60)))
							   g_bClassSelect = true;
						   else
							   g_bClassSelect = false;
						   //중간 Connect 혹은 생성, 시작 위치가 됨
						   if ((CurrentMouseCursorPoint.x >(m_nWndClientWidth - 300)) && (CurrentMouseCursorPoint.x < (m_nWndClientWidth - 300 + 288)) && (CurrentMouseCursorPoint.y >(m_nWndClientHeight - 300)) && (CurrentMouseCursorPoint.y < (m_nWndClientHeight - 300 + 60)))
							   g_bStart = true;
						   else
							   g_bStart = false;
						   //Exit, 혹은 뒤로가기
						   if ((CurrentMouseCursorPoint.x >(m_nWndClientWidth - 300)) && (CurrentMouseCursorPoint.x < (m_nWndClientWidth - 300 + 288)) && (CurrentMouseCursorPoint.y >(m_nWndClientHeight - 200)) && (CurrentMouseCursorPoint.y < (m_nWndClientHeight - 200 + 60)))
							   g_bExit = true;
						   else
							   g_bExit = false;
					   }
					   break;
    default:
        break;
    }
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
    CNode* pStart = nullptr;
    CNode* pEnd = nullptr;

    if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN: {
			if (g_chat_active) {
				if (g_chat_massege.empty()) break;
				//ksh7
				wstring check;

				wchar_t t = L' ';
				auto b = g_chat_massege.find(t);
				int size = g_chat_massege.size();

				if (g_chat_massege[0] == L'/')
				{
					check += g_chat_massege[1];
					check += g_chat_massege[2];

					if (check == L"초대") {
						for (auto i = b + 1; i < size; ++i) {
							id += g_chat_massege[i];
						}
						sendInviteParty();

					}
					else if (check == L"탈퇴") {
						//파티 false;
						exitParty();
					}
				}
				//ksh7
				if (g_chat_massege[0] != L'/') {
					// string send
					wstring str = L"ID :";
					g_chat_massege = str + g_chat_massege;
					sc_packet_chat_message *packet = reinterpret_cast<sc_packet_chat_message*>(m_csend_buf);
					packet->size = sizeof(sc_packet_chat_message);
					m_wsa_send_buf.len = sizeof(sc_packet_chat_message);
					packet->type = PLAYER_EVENT_CHAT_MESSAGE;
					wcsncpy_s(packet->message, g_chat_massege.c_str(), MAX_STR_SIZE);
					DWORD iobyte;
					WSASend(m_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);
					g_chat_massege.clear();
				}
					g_chat_active = false;				
			}
			else g_chat_active = true;
		}break;
		}break;	
    case WM_KEYUP:
        switch (wParam)
        {
        case VK_ESCAPE:
            ::PostQuitMessage(0);
            break;
        case VK_RETURN:
            break;
        case 0x52: // 'R' 재장전
            pStart = m_pPlayer->getBulletList()->getHead();
            pEnd = m_pPlayer->getBulletList()->getTail();
            if (pStart->m_pNext == pEnd)
            {
                m_pPlayer->getBulletList()->Init();
            }
            break;
        case VK_F1:
        {
        }
        break;
        case VK_F2:
        case VK_F3:
            if (m_pPlayer)
            {
                m_pPlayer->ChangeCamera(m_pd3dDevice, DWORD(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
                m_pCamera = m_pPlayer->GetCamera();
                m_pScene->SetCamera(m_pCamera);
            }
            break;
        case VK_F4:
            g_debug ^= true;
            break;
        case VK_F5:
            g_debugCursor ^= true;
            ShowCursor(g_debugCursor);
            break;
        case VK_F9:
        {
            g_full ^= true;

            BOOL bFullScreenState = FALSE;
            m_pDXGISwapChain->GetFullscreenState(&bFullScreenState, NULL);
            if (!bFullScreenState)
            {
                DXGI_MODE_DESC dxgiTargetParameters;
                dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                dxgiTargetParameters.Width = m_nWndClientWidth;
                dxgiTargetParameters.Height = m_nWndClientHeight;
                dxgiTargetParameters.RefreshRate.Numerator = 0;
                dxgiTargetParameters.RefreshRate.Denominator = 0;
                dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
                dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
                m_pDXGISwapChain->ResizeTarget(&dxgiTargetParameters);
            }
            m_pDXGISwapChain->SetFullscreenState(!bFullScreenState, NULL);

			RECT rc;
			GetClientRect(m_hWnd, &rc);

			m_pMinimapCamera->SetViewport(m_pd3dDeviceContext, 0.040f * rc.right, 0.058f * rc.bottom,
				0.226f * rc.right, 0.302f * rc.bottom);


            break;
        }
        case VK_F10:
            break;
		case 'O':
		case 'o':
			g_party ^= true;
			if (g_party)
				m_pt_mouse_positoin = m_pt_mouse_move_position;
			break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}


//ksh
void CGameFramework::sendJoinParty()
{
	cs_packet_party *my_packet = reinterpret_cast<cs_packet_party *>(m_csend_buf);	// 키가 눌러졌으면 패킷을 정리하고
	my_packet->size = sizeof(cs_packet_party);
	m_wsa_send_buf.len = sizeof(cs_packet_party);
	DWORD iobyte;

	my_packet->type = 88;	// 타입만 바꿔서 보내면 된다.
	my_packet->bool_invite = false;
	my_packet->bool_join = true;
	wcsncpy_s(my_packet->invite_name, in_name, 1024); // 초대한 사람 이름이다.
	wcsncpy_s(my_packet->join_name, jo_name, 1024); // 수락한 사람 이름이다.

													//party4
	int ret = WSASend(party_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);	// 아까 만든 소켓에다가 보낸다. 1=버퍼는 1개다, 버퍼의크기는 my_packet사이즈,
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CGameFramework::sendNoParty()
{
	cs_packet_party *my_packet = reinterpret_cast<cs_packet_party *>(m_csend_buf);	// 키가 눌러졌으면 패킷을 정리하고
	my_packet->size = sizeof(cs_packet_party);
	m_wsa_send_buf.len = sizeof(cs_packet_party);
	DWORD iobyte;

	my_packet->type = 88;	// 타입만 바꿔서 보내면 된다.
	my_packet->bool_invite = true;
	my_packet->bool_join = true;
	wcsncpy_s(my_packet->invite_name, in_name, 1024); // 초대한 사람 이름이다.
	wcsncpy_s(my_packet->join_name, jo_name, 1024); // 수락한 사람 이름이다.

													//party4
	int ret = WSASend(party_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);	// 아까 만든 소켓에다가 보낸다. 1=버퍼는 1개다, 버퍼의크기는 my_packet사이즈,
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void CGameFramework::sendInviteParty()
{
	cs_packet_party *my_packet = reinterpret_cast<cs_packet_party *>(m_csend_buf);	// 키가 눌러졌으면 패킷을 정리하고
	my_packet->size = sizeof(cs_packet_party);
	m_wsa_send_buf.len = sizeof(cs_packet_party);
	DWORD iobyte;

	//party1
	my_packet->type = 88;	// 타입만 바꿔서 보내면 된다.
	my_packet->bool_invite = true;	// 이건 상관없다.
	my_packet->bool_join = false;

	int b = id.size();
	for (auto i = 0; i < b; ++i) {
		my_packet->join_name[i] = id[i];
		my_packet->invite_name[i] = id[i];
	}

	int ret = WSASend(party_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
	//}
	//else {
	//	cout << "자신의 이름을 입력하였습니다." << endl;
	//}
}

void CGameFramework::exitParty()
{
	cs_packet_party *my_packet = reinterpret_cast<cs_packet_party *>(m_csend_buf);	// 키가 눌러졌으면 패킷을 정리하고
	my_packet->size = sizeof(cs_packet_party);
	m_wsa_send_buf.len = sizeof(cs_packet_party);
	DWORD iobyte;

	//party1
	my_packet->type = 88;	// 타입만 바꿔서 보내면 된다.
	my_packet->bool_invite = false;	// 이건 상관없다.
	my_packet->bool_join = false;
	wcsncpy_s(my_packet->join_name, L"0", 1024);
	wcsncpy_s(my_packet->invite_name, L"0", 1024);

	int ret = WSASend(party_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);	// 소켓에다가 보낸다. 1=버퍼는 1개다, 버퍼의크기는 my_packet사이즈,
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

//LRESULT CALLBACK InputParty(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
//{
//	char text_file_ip1[20];
//
//	strcpy_s(text_file_ip1, "Party");
//	MultiByteToWideChar(CP_ACP, 0, text_file_ip1, strlen(text_file_ip1) + 1, ipTmp, strlen(text_file_ip1) + 1);
//
//	switch (iMessage) {
//	case WM_INITDIALOG:
//		hIp = GetDlgItem(hDlg, IDC_EDIT2);
//		SetWindowText(hIp, ipTmp);
//		return TRUE;
//
//	case WM_COMMAND:
//		switch (wParam) {
//		case IDOK1:
//			g_debugCursor ^= true;
//			ShowCursor(g_debugCursor);
//			GetWindowText(hIp, tc_ip, 20);
//			party.sendInviteParty();
//			EndDialog(hDlg, IDOK1);
//			break;
//
//		case IDCANCEL1:
//			g_debugCursor ^= true;
//			ShowCursor(g_debugCursor);
//			EndDialog(hDlg, IDCANCEL1);
//			break;
//		}
//		break;
//
//	case WM_DESTROY:
//		g_debugCursor ^= true;
//		ShowCursor(g_debugCursor);
//		EndDialog(hDlg, TRUE);
//		return TRUE;
//	}
//
//	return FALSE;
//}
//ksh

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	static HIMC imeID = nullptr;

    switch (nMessageID)
    {
    case WM_SIZE:
    {
        m_nWndClientWidth = LOWORD(lParam);
        m_nWndClientHeight = HIWORD(lParam);

        m_pd3dDeviceContext->OMSetRenderTargets(0, NULL, NULL);

        if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
        if (m_pd3dRenderTargetView) m_pd3dRenderTargetView->Release();
        if (m_pd3dDepthStencilView) m_pd3dDepthStencilView->Release();

        m_pDXGISwapChain->ResizeBuffers(1, m_nWndClientWidth, m_nWndClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

        CreateRenderTargetDepthStencilView();

        if (m_pCamera) m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, m_nWndClientWidth, m_nWndClientHeight, 0.0f, 1.0f);
        break;
    }
	case WM_ACTIVATE: {
		if (imeID == nullptr) {
			imeID = ImmCreateContext();
			ImmAssociateContext(hWnd, imeID);
		}
	}break;
	case WM_IME_STARTCOMPOSITION: break;
	case WM_IME_CHAR:
		if (g_chat_active)
			g_chat_massege.push_back((wchar_t)wParam);
		if ((wchar_t)wParam == '\r') break;
		break;
	case WM_CHAR:
		if (!g_chat_active) break;
		if ((wchar_t)wParam == '\b') {
			if (g_chat_massege.empty()) break;
			g_chat_massege.pop_back();
			break;
		}
		if ((wchar_t)wParam == '\r') break;
		g_chat_massege.push_back((wchar_t)wParam); break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
        m_fRefireTime = 0.0f;
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
        OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
        break;
    case WM_KEYDOWN:{
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
    case WM_KEYUP:
        OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
        break;
    }
    return(0);
}

BYTE * CGameFramework::ReadCompiledShaderCode(ID3D11Device *pd3dDevice, WCHAR *pszFileName, int& nSize)
{
	std::ifstream ifsCompiledShader;
	ifsCompiledShader.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	int nFileSize = ifsCompiledShader.tellg();
	ifsCompiledShader.seekg(0);
	BYTE *pByteCode = new BYTE[nFileSize];
	ifsCompiledShader.read((char *)pByteCode, nFileSize);
	ifsCompiledShader.close();

	nSize = nFileSize;
	return pByteCode;
}

void CGameFramework::CreateBlurringShader(ID3D11Device *pd3dDevice)
{
	int nSize;
	BYTE *pHorizontalBlurShaderByteCode = ReadCompiledShaderCode(pd3dDevice, L"HorzBlurCS.cso", nSize);
	pd3dDevice->CreateComputeShader(pHorizontalBlurShaderByteCode, nSize, nullptr, &m_pHorizontalBlurShader);
	delete[] pHorizontalBlurShaderByteCode;

	BYTE *pVerticalBlurShaderByteCode = ReadCompiledShaderCode(pd3dDevice, L"VertBlurCS.cso", nSize);
	pd3dDevice->CreateComputeShader(pVerticalBlurShaderByteCode, nSize, nullptr, &m_pVerticalBlurShader);
	delete[] pVerticalBlurShaderByteCode;
}

void CGameFramework::CreateBlurringResources(ID3D11Device *pd3dDevice) // ok
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC d3dTex2DDesc;
	d3dTex2DDesc.Width = m_nWndClientWidth;
	d3dTex2DDesc.Height = m_nWndClientHeight;
	d3dTex2DDesc.MipLevels = 1; // for a multisampled texture. if you want to create full set of subtextures, set 0.
	d3dTex2DDesc.ArraySize = 1;
	d3dTex2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dTex2DDesc.SampleDesc.Count = 1;
	d3dTex2DDesc.SampleDesc.Quality = 0;
	d3dTex2DDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dTex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	d3dTex2DDesc.CPUAccessFlags = 0;
	d3dTex2DDesc.MiscFlags = 0;

	ID3D11Texture2D *pd3dTex2D = nullptr;
	hr = pd3dDevice->CreateTexture2D(&d3dTex2DDesc, nullptr, &pd3dTex2D);

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dsrvDesc;
	d3dsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dsrvDesc.Format = d3dTex2DDesc.Format;
	d3dsrvDesc.Texture2D.MipLevels = d3dTex2DDesc.MipLevels;
	d3dsrvDesc.Texture2D.MostDetailedMip = 0;
	hr = pd3dDevice->CreateShaderResourceView(pd3dTex2D, &d3dsrvDesc, &m_pd3dsrvOffScreen);

	D3D11_UNORDERED_ACCESS_VIEW_DESC d3duavDesc;
	d3duavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	d3duavDesc.Format = d3dTex2DDesc.Format;
	d3duavDesc.Texture2D.MipSlice = 0;
	hr = pd3dDevice->CreateUnorderedAccessView(pd3dTex2D, &d3duavDesc, &m_pd3duavOffScreen);

	D3D11_RENDER_TARGET_VIEW_DESC d3drtvDesc;
	d3drtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	d3drtvDesc.Format = d3dTex2DDesc.Format;
	d3drtvDesc.Texture2D.MipSlice = 0;
	hr = pd3dDevice->CreateRenderTargetView(pd3dTex2D, &d3drtvDesc, &m_pd3drtvOffScreen);
	pd3dTex2D->Release();

	d3dTex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hr = pd3dDevice->CreateTexture2D(&d3dTex2DDesc, nullptr, &pd3dTex2D);
	hr = pd3dDevice->CreateShaderResourceView(pd3dTex2D, &d3dsrvDesc, &m_pd3dsrvTexture);
	hr = pd3dDevice->CreateUnorderedAccessView(pd3dTex2D, &d3duavDesc, &m_pd3duavTexture);
	pd3dTex2D->Release();
}

void CGameFramework::SceneBlurring(int nBlurCount)
{
	UINT cxGroups = (UINT)ceilf(m_nWndClientWidth / 256.0f);
	UINT cyGroups = (UINT)ceilf(m_nWndClientHeight / 256.0f);
	ID3D11ShaderResourceView *pd3dNullResourceViews[1] = { nullptr };
	ID3D11UnorderedAccessView *pd3dNullUnorderedViews[1] = { nullptr };

	for (int i = 0; i < nBlurCount; ++i)
	{
		m_pd3dDeviceContext->CSSetShaderResources(0, 1, &m_pd3dsrvOffScreen);
		m_pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3duavTexture, nullptr);
		m_pd3dDeviceContext->CSSetShader(m_pHorizontalBlurShader, nullptr, NULL);
		m_pd3dDeviceContext->Dispatch(cxGroups, m_nWndClientHeight, 1);

		m_pd3dDeviceContext->CSSetShaderResources(0, 1, pd3dNullResourceViews);
		m_pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, pd3dNullUnorderedViews, nullptr);

		m_pd3dDeviceContext->CSSetShaderResources(0, 1, &m_pd3dsrvTexture);
		m_pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &m_pd3duavOffScreen, nullptr);
		m_pd3dDeviceContext->CSSetShader(m_pVerticalBlurShader, nullptr, NULL);
		m_pd3dDeviceContext->Dispatch(m_nWndClientWidth, cyGroups, 1);

		m_pd3dDeviceContext->CSSetShaderResources(0, 1, pd3dNullResourceViews);
		m_pd3dDeviceContext->CSSetUnorderedAccessViews(0, 1, pd3dNullUnorderedViews, nullptr);
		m_pd3dDeviceContext->CSSetShader(nullptr, nullptr, NULL);
	}
}

void CGameFramework::DrawBlurredSceneToScreen()
{
	ID3D11Texture2D *pd3dBackBuffer = nullptr;
	m_pd3dRenderTargetView->GetResource((ID3D11Resource **)&pd3dBackBuffer);
	ID3D11Texture2D *pd3dBlurredTex = nullptr;
	m_pd3duavOffScreen->GetResource((ID3D11Resource **)&pd3dBlurredTex);

	m_pd3dDeviceContext->CopyResource(pd3dBackBuffer, pd3dBlurredTex);

	pd3dBackBuffer->Release();
	pd3dBlurredTex->Release();
}

void CGameFramework::BuildObjects()
{
    CreateShaderVariables();

    CSkinnedMesh::LoadAnimationSet();
    ID3D11SamplerState *pd3dSamplerState = NULL;
    D3D11_SAMPLER_DESC d3dSamplerDesc;
    ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    d3dSamplerDesc.MinLOD = 0;
    d3dSamplerDesc.MaxLOD = 0;
    m_pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

    ID3D11ShaderResourceView *pd3dsrvTexture = NULL;
    CTexture *pPlayerTexture = new CTexture(1, 1, 0, 0);
    D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, _T("Character/SimpleMilitary_SpecialForces_03_White.png"), NULL, NULL, &pd3dsrvTexture, NULL);
    //D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, _T("Textures/SimpleMilitary_GasMask_White.png"), NULL, NULL, &pd3dsrvTexture, NULL);
    pPlayerTexture->SetTexture(0, pd3dsrvTexture);
    pPlayerTexture->SetSampler(0, pd3dSamplerState);

    CTexture *pWeaponTexture = new CTexture(1, 1, 0, 0);
    D3DX11CreateShaderResourceViewFromFile(m_pd3dDevice, _T("Character/Weapon/Weapons_AssultRifle01.png"), NULL, NULL, &pd3dsrvTexture, NULL);
    pWeaponTexture->SetTexture(0, pd3dsrvTexture);
    pWeaponTexture->SetSampler(0, pd3dSamplerState);

    pd3dsrvTexture->Release();
    pd3dSamplerState->Release();

    CSkinnedMesh *pPlayerMesh = new CSkinnedMesh(m_pd3dDevice, "Character/speicalforceStatic.data", 0.008f);
    CMaterialColors *pColor = new CMaterialColors();
    CMaterial *pCharMat = new CMaterial(pColor);
    pCharMat->SetTexture(pPlayerTexture);

    m_pPlayer = new CTerrainPlayer(1);
    m_pPlayer->AddRef();
    m_pPlayer->SetMesh(pPlayerMesh);
    m_pPlayer->SetTexture(pPlayerTexture);
    m_pPlayer->ChangeCamera(m_pd3dDevice, THIRD_PERSON_CAMERA, 0.0f);

    CMaterial *pWeaponMat = new CMaterial(pColor);
    pWeaponMat->SetTexture(pWeaponTexture);

    CAssetMesh *pWeaponMesh = new CAssetMesh(m_pd3dDevice, "Character/Weapon/SimpleMilitary_Weapons.data", CAssetMesh::CharacterWeapon);

    CGameObject *pWeapon = new CGameObject();
    pWeapon->SetMesh(pWeaponMesh);
    pWeapon->SetMaterial(pWeaponMat);

    pWeapon->SetObjectType(ObjectType::player_weapon);
    m_pPlayer->SetChild(pWeapon);

    CCharacterShader *pPlayerShader = new CCharacterShader(1);
    pPlayerShader->CreateShader(m_pd3dDevice);
    m_pPlayer->SetShader(pPlayerShader);

    CObjectsShader* pObjectShader = new CObjectsShader();
    UINT meshtype = VERTEX_POSITION_ELEMENT | VERTEX_NORMAL_ELEMENT | VERTEX_TEXTURE_ELEMENT_0;
    pObjectShader->CreateShader(m_pd3dDevice, meshtype);

    pWeapon->SetShader(pObjectShader);


    m_pScene = new CScene();
    m_pScene->SetPlayer(m_pPlayer);
    m_pScene->BuildObjects(m_pd3dDevice);
    m_pScene->SetSocket(m_socket);

    CHeightMapTerrain *pTerrain = m_pScene->GetTerrain();
    float fHeight = pTerrain->GetHeight(pTerrain->GetWidth()*0.5f, pTerrain->GetLength()*0.5f, false) + 100.0f;

    m_pPlayer->SetPosition(D3DXVECTOR3(555.0f, 0, 0));
    m_pPlayer->SetPlayerUpdatedContext(pTerrain);
    m_pPlayer->SetCameraUpdatedContext(pTerrain);

    m_pCamera = m_pPlayer->GetCamera();
    m_pCamera->SetViewport(m_pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    m_pCamera->GenerateViewMatrix();

    m_pScene->SetCamera(m_pCamera);

    m_pMinimapScene = new CMinimapScene();
    m_pMinimapScene->BuildObjects(m_pd3dDevice);

    // minimapCamera
    m_pMinimapCamera = new CCamera(nullptr);

	RECT rc;
	GetClientRect(m_hWnd, &rc);

    m_pMinimapCamera->SetViewport(m_pd3dDeviceContext, 0.040f * FRAME_BUFFER_WIDTH, 0.058f * FRAME_BUFFER_HEIGHT,
		0.226f * FRAME_BUFFER_WIDTH, 0.302f * FRAME_BUFFER_HEIGHT);
    m_pMinimapCamera->SetPlayer(m_pPlayer);
    m_pMinimapCamera->GenerateProjectionMatrix(1.0f, 2000.0f, 1.0f, 45.0f);

	// Shadow
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.BorderColor[0] = 0.0f;
	d3dSamplerDesc.BorderColor[1] = 0.0f;
	d3dSamplerDesc.BorderColor[2] = 0.0f;
	d3dSamplerDesc.BorderColor[3] = 0.0f;
	d3dSamplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
	d3dSamplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX
	m_pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &m_pd3dShadowSS);

	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &m_pd3dShadowPCFSS);

	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 100000;
	d3dRasterizerDesc.DepthBiasClamp = 0.f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 1.f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.ScissorEnable = FALSE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	m_pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dShadowRS);

	D3D11_BUFFER_DESC cbShadowMapDesc;
	cbShadowMapDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbShadowMapDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbShadowMapDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbShadowMapDesc.MiscFlags = 0;
	cbShadowMapDesc.ByteWidth = sizeof(D3DXMATRIX);
	cbShadowMapDesc.StructureByteStride = 0;
	m_pd3dDevice->CreateBuffer(&cbShadowMapDesc, nullptr, &m_pd3dcbShadow);

	m_pShadowMap = new ShadowMap(m_pd3dDevice, 8192, 8192);

	// Lobby
	CreateLobbyShader(m_pd3dDevice);

	atomic_thread_fence(std::memory_order_seq_cst);
	g_bBuildFinish = true;
}

void CGameFramework::ReleaseObjects()
{
    ReleaseShaderVariables();

    if (m_pScene) m_pScene->ReleaseObjects();
    if (m_pScene) delete m_pScene;
    if (m_pMinimapScene) delete m_pMinimapScene;

    if (m_pPlayer) delete m_pPlayer;
}

void CGameFramework::CreateShaderVariables()
{
    D3D11_BUFFER_DESC d3dBufferDesc;
    ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
    d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    d3dBufferDesc.ByteWidth = sizeof(VS_CB_WORLD_MATRIX);
    m_pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &CGameObject::m_pd3dcbWorldMatrix);

    d3dBufferDesc.ByteWidth = sizeof(D3DXCOLOR) * 4;
    m_pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &CGameObject::m_pd3dcbMaterialColors);

    CCamera::CreateShaderVariables(m_pd3dDevice);
    CTexture::CreateShaderVariables(m_pd3dDevice);
}

void CGameFramework::ReleaseShaderVariables()
{
    CGameObject::ReleaseShaderVariables();
    CCamera::ReleaseShaderVariables();
    CTexture::ReleaseShaderVariables();
}

void CGameFramework::ProcessInput()
{
	if (g_chat_active) {
		m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
		return;
	}

    static UCHAR pKeysBuffer[256];
    bool bProcessedByScene = false;
    if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
    if (!bProcessedByScene)
    {

        unsigned int CurrentState = m_pPlayer->GetPlayerState(); //2015.5.15 박종혁

        DWORD dwDirection = 0;
        if (pKeysBuffer[0x57] & 0xF0) { dwDirection |= DIR_FORWARD, m_pPlayer->SetPlayerState(CPlayer::MOVE); }

        if (pKeysBuffer[0x53] & 0xF0) { dwDirection |= DIR_BACKWARD, m_pPlayer->SetPlayerState(CPlayer::MOVE); }
        if (pKeysBuffer[0x41] & 0xF0) { dwDirection |= DIR_LEFT, m_pPlayer->SetPlayerState(CPlayer::MOVE); }
        if (pKeysBuffer[0x44] & 0xF0) { dwDirection |= DIR_RIGHT, m_pPlayer->SetPlayerState(CPlayer::MOVE); }
        if (pKeysBuffer[VK_PRIOR] & 0xF0) { dwDirection |= DIR_UP, m_pPlayer->SetPlayerState(CPlayer::MOVE); }
        if (pKeysBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
        if (pKeysBuffer[VK_LBUTTON] & 0xF0)
        {
            m_fRefireTime -= m_GameTimer.GetTimeElapsed();
            if (m_fRefireTime <= 0.0f)
            {
                m_pPlayer->getBulletList()->Add(m_pPlayer->GetPosition() + D3DXVECTOR3(0.0f, 1.0f, 0.0f), m_pPlayer->GetCamera()->GetLookVector());
                m_fRefireTime = 0.15f;
            }

            if (CurrentState != CPlayer::SHOOT)
            {
                m_pPlayer->SetPlayerState(CPlayer::SHOOT);
                cs_packet_charater_animate *animate_packet = reinterpret_cast<cs_packet_charater_animate*>(m_csend_buf);
                m_wsa_send_buf.len = sizeof(cs_packet_charater_animate);
                animate_packet->size = sizeof(cs_packet_charater_animate);
                animate_packet->type = PLAYER_ANIMATE_STATE;
                animate_packet->anim_state = ANIMATE_SHOT;
                DWORD d_iobyte;
                WSASend(m_socket, &m_wsa_send_buf, 1, &d_iobyte, 0, NULL, NULL);

            }
        }
        //cout << m_pPlayer->getBulletList()->m_nBulletCount << endl;

        float cxDelta = 0.0f, cyDelta = 0.0f;
        POINT ptCursorPos;

        if (!g_debugCursor)
        {
            GetCursorPos(&ptCursorPos);
            cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 30.0f;
            cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 30.0f;
            SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
        }

        if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
        {
            if (cxDelta || cyDelta)
            {
                m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
                cs_packet_player_state *packet = reinterpret_cast<cs_packet_player_state*>(m_csend_buf);
                packet->size = sizeof(cs_packet_player_state);
                m_wsa_send_buf.len = sizeof(cs_packet_player_state);
                DWORD iobyte;
                packet->type = PLAYER_EVENT_POS;
                packet->direction = dwDirection;
                packet->look_x = m_pPlayer->GetLook().x;
                packet->look_y = m_pPlayer->GetLook().y;
                packet->look_z = m_pPlayer->GetLook().z;
                packet->right_x = m_pPlayer->GetRight().x;
                packet->right_y = m_pPlayer->GetRight().y;
                packet->right_z = m_pPlayer->GetRight().z;
                int retval = WSASend(m_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);

            }
            if (dwDirection)
            {
				cs_packet_player_state *packet = reinterpret_cast<cs_packet_player_state*>(m_csend_buf);
                packet->size = sizeof(cs_packet_player_state);
                m_wsa_send_buf.len = sizeof(cs_packet_player_state);
                DWORD iobyte;
                packet->type = PLAYER_EVENT_POS;
                packet->direction = dwDirection;
                packet->look_x = m_pPlayer->GetLook().x;
                packet->look_y = m_pPlayer->GetLook().y;
                packet->look_z = m_pPlayer->GetLook().z;
                packet->right_x = m_pPlayer->GetRight().x;
                packet->right_y = m_pPlayer->GetRight().y;
                packet->right_z = m_pPlayer->GetRight().z;
                int retval = WSASend(m_socket, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);

                cs_packet_charater_animate *animate_packet = reinterpret_cast<cs_packet_charater_animate*>(m_csend_buf);
                m_wsa_send_buf.len = sizeof(cs_packet_charater_animate);
                animate_packet->size = sizeof(cs_packet_charater_animate);
                animate_packet->type = PLAYER_ANIMATE_STATE;
                animate_packet->anim_state = ANIMATE_MOVE;
                DWORD d_iobyte;
                WSASend(m_socket, &m_wsa_send_buf, 1, &d_iobyte, 0, NULL, NULL);
            }
        }
        m_pPlayer->Update(m_GameTimer.GetTimeElapsed());

    }
}

void CGameFramework::AnimateObjects(CConnectServer *server_info)
{
    float fTimeElapsed = m_GameTimer.GetTimeElapsed();
	static int time_count = 0;
	time_count++;
	if (server_info->GetPlayerAttack()) {
		m_bBlur = true;
		server_info->SetPlayerAttack();
	}


	if (time_count > 100) {
		if (m_bBlur) {
			m_bBlur = false;
		}
		time_count = 0;
	}


    if (m_pPlayer)
    {
        unsigned int Current_state = m_pPlayer->GetPlayerState();
        unsigned int AnimateState = m_pPlayer->GetMesh()->GetAnimationState();

        D3DXVECTOR3 d3dvshift = D3DXVECTOR3(server_info->GetMyPlayerPos().first.first, server_info->GetMyPlayerPos().first.second, server_info->GetMyPlayerPos().second);
        if (m_pPlayer->GetFirst()) {
            m_pPlayer->SetPosition(d3dvshift);
            m_pPlayer->SetFirst(false);
        }
        else {
            D3DXVECTOR3 d3dvpredpos = m_pPlayer->GetPosition();
            d3dvshift -= d3dvpredpos;
            m_pPlayer->Move(d3dvshift, true);
            m_pPlayer->Animate(fTimeElapsed, NULL);
            m_pPlayer->getBulletList()->Update(fTimeElapsed);
        }
        //2016.5.15 박종혁 시작
        ///////////////////////////////////////////////////////////////////////////////////////
        if ((Current_state != AnimateState) && (AnimateState != CPlayer::RELOAD))//현재 입력받은 상태와 애니메이션 상태가 다른가? 여기는 애니메이션을 Set하는 곳이다.
        {
            m_pPlayer->GetMesh()->SetAnimationState(Current_state);//그러면 애니메이션 상태를 바꾸고,
            if (Current_state == CPlayer::SHOOT)//
                                                //m_pPlayer->GetMesh()->ShootPistol();//사격 애니메이션으로 바꾼다.
                m_pPlayer->GetMesh()->ShootRifle();
            if (Current_state == CPlayer::MOVE)
                m_pPlayer->GetMesh()->RunAnimationSet();

        }
        if (Current_state == CPlayer::RELOAD)
        {
            m_pPlayer->GetMesh()->SetAnimationState(Current_state);
            m_pPlayer->GetMesh()->ReloadRifle();//바꾸기는 쉽지..
        }
        //2016.5.9 박종혁
        if (m_pPlayer->GetMesh()->FBXFrameAdvance(fTimeElapsed))//2016.5.8 박종혁
        {
            if (Current_state == CPlayer::SHOOT)//애니메이션이 끝나고 나서 이제 상태를 어떻게 변경할까? Shoot가 왜 액션이 없을까?
                                                //m_pPlayer->GetMesh()->ShootPistol();
                m_pPlayer->GetMesh()->ShootRifle();
            if ((Current_state == CPlayer::MOVE) && (Current_state != CPlayer::SHOOT))
            {
                m_pPlayer->GetMesh()->RunAnimationSet();
                m_pPlayer->SetPlayerState(CPlayer::SHOOT);

            }

            if (AnimateState == CPlayer::RELOAD)
            {
                m_pPlayer->GetMesh()->SetAnimationState(CPlayer::IDLE);
                //m_pPlayer->GetMesh()->IdleSet();
            }
        }
        if ((Current_state == CPlayer::IDLE) && (AnimateState != CPlayer::IDLE))
        {
            m_pPlayer->GetMesh()->IdleSet();
			cs_packet_charater_animate *animate_packet = reinterpret_cast<cs_packet_charater_animate*>(m_csend_buf);
            m_wsa_send_buf.len = sizeof(cs_packet_charater_animate);
            animate_packet->size = sizeof(cs_packet_charater_animate);
            animate_packet->type = PLAYER_ANIMATE_STATE;
            animate_packet->anim_state = ANIMATE_IDLE;
            DWORD d_iobyte;
            WSASend(m_socket, &m_wsa_send_buf, 1, &d_iobyte, 0, NULL, NULL);
        }

        m_pPlayer->SetPlayerState(CPlayer::IDLE);//계속 IDLE로 바꾸도록 노력한다.

        //2016.5.15 박종혁 끝/////////////////////////////////////////////////////////////////////////////////
    }
    if (m_pScene)
    {
        // Animate Other Player
        for (auto i = 0; i < MAX_USER; ++i) {
            if (false == server_info->GetOhterPlayerViewState(i))
            {
                m_pScene->GetOtherPlayer(i)->SetFirst(true);
                m_pScene->GetOtherPlayer(i)->SetActive(false);
                continue;
            }
            D3DXVECTOR3 d3dxvposition = D3DXVECTOR3(server_info->GetOtherPlayerPos(i).first.first, 1.3f, server_info->GetOtherPlayerPos(i).second);
            D3DXVECTOR3 d3dxv_look_vector = D3DXVECTOR3(server_info->GetOtherPlayerLookVector(i).first.first, server_info->GetOtherPlayerLookVector(i).first.second, server_info->GetOtherPlayerLookVector(i).second);

            if (m_pScene->GetOtherPlayer(i)->GetFirst()) {
                m_pScene->GetOtherPlayer(i)->SetActive(true);
                m_pScene->GetOtherPlayer(i)->SetFirst(false);
                m_pScene->GetOtherPlayer(i)->SetLookAndGenerate(d3dxv_look_vector);
                m_pScene->GetOtherPlayer(i)->SetPosition(d3dxvposition);
            }
            else {
                D3DXVECTOR3 d3dxvpredposition = m_pScene->GetOtherPlayer(i)->GetPosition();
                d3dxvposition -= d3dxvpredposition;
                d3dxvposition *= fTimeElapsed;
                m_pScene->GetOtherPlayer(i)->SetLookAndGenerate(d3dxv_look_vector);
                m_pScene->GetOtherPlayer(i)->Move(d3dxvposition);
            }
            CMesh* TempMesh = m_pScene->GetOtherPlayer(i)->GetMesh();
            unsigned int animate_state = server_info->GetOtherPlayerAnimateState(i);
            unsigned int tempState = TempMesh->GetAnimationState();
            if (tempState != animate_state)//애니메이션 상태가 다를 경우
            {
                TempMesh->SetAnimationState(animate_state);
                if (animate_state == 0)
                    TempMesh->IdleSet();
                else if (animate_state == 1)
                    TempMesh->RunAnimationSet();
                else if (animate_state == 2)
                    TempMesh->ShootRifle();
            }
            //true, 아니면 false를 리턴하는데, true면 애니메이션 한 세트가 끝났다는 뜻
            if (TempMesh->FBXFrameAdvance(fTimeElapsed))
            {
                if (tempState == 0)
                    TempMesh->IdleSet();
                else if (tempState == 1)
                    TempMesh->RunAnimationSet();
                else if (tempState == 2)
                    TempMesh->ShootRifle();
            }
        }
        // Animate NPC

        m_pScene->AnimateBullets(fTimeElapsed, m_pPlayer->getBulletList(), m_pd3dDevice);
        for (auto i = 0; i < MAX_ENEMY; ++i) {
            if (false == server_info->GetEnemyNPCHeartBeatState(i))
            {
                m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetFirst(true);
                m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetActive(false);
                continue;
            }

            if (server_info->GetEnemyNPCView(i))
            {
                m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetActive(true);
                D3DXVECTOR3 d3dposition = D3DXVECTOR3(server_info->GetEnemyNPCPos(i).first.first, 1.3f, server_info->GetEnemyNPCPos(i).second);
                D3DXVECTOR3 d3dxv_look_vector = D3DXVECTOR3(server_info->GetEnemyNPCLookVector(i).first.first, server_info->GetEnemyNPCLookVector(i).first.second, server_info->GetEnemyNPCLookVector(i).second);

				if (m_pScene->GetEnemyObject(0)->GetGameObject(i)->GetFirst()) {
                    m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetLookAndGenerate(d3dxv_look_vector);
                    m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetPosition(d3dposition);
                    m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetFirst(false);
                }
                else {
									
					D3DXVECTOR3 d3dDistPosition = m_pPlayer->GetPosition();
					d3dDistPosition -= d3dposition;

					int dist = d3dDistPosition.x *d3dDistPosition.x + d3dDistPosition.z * d3dDistPosition.z;
					

					if (dist <= 20 * 20) {
						CCamera *pCamera = m_pPlayer->GetCamera();
						D3DXMATRIX mtx_camera_view = pCamera->GetViewMatrix();
						D3DXMATRIX mtx_camera_pro = pCamera->GetProjectionMatrix();

						D3DXVECTOR4 mtx_screen = D3DXVECTOR4(0, 0, 0, 1);
						D3DXVec4Transform(&mtx_screen, &D3DXVECTOR4(d3dposition, 1.0f), &mtx_camera_view);
						D3DXVec4Transform(&mtx_screen, &mtx_screen, &mtx_camera_pro);

						mtx_screen.x = mtx_screen.x / mtx_screen.w;
						mtx_screen.y = mtx_screen.y / mtx_screen.w;
						mtx_screen.z = mtx_screen.z / mtx_screen.w;


						mtx_screen.x = mtx_screen.x * 0.5 + 0.5;
						mtx_screen.y = mtx_screen.y * 0.5 + 0.5;

						mtx_screen.x = mtx_screen.x * m_nWndClientWidth;
						mtx_screen.y = mtx_screen.y * m_nWndClientHeight;

						m_vd3dvec2.push_back(make_pair(i, D3DXVECTOR2(mtx_screen.x, mtx_screen.y)));
					}
					D3DXVECTOR3 d3dvpredpos = m_pScene->GetEnemyObject(0)->GetGameObject(i)->GetPosition();
					d3dposition -= d3dvpredpos;
                    d3dposition *= fTimeElapsed;
                    m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetLookAndGenerate(d3dxv_look_vector);
                    m_pScene->GetEnemyObject(0)->GetGameObject(i)->Move(d3dposition);
                }
            }
            else {
                m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetActive(false);
                m_pScene->GetEnemyObject(0)->GetGameObject(i)->SetFirst(true);
            }

        }
        m_pScene->AnimateObjects(fTimeElapsed);

		// Animate Steer
		int count = 0;
		for (auto i = 0; i < 3; ++i) {
			for (auto j = 0; j < 100; ++j) {
				CSteerGSInstancedObjectShader* pSteerShader = m_pScene->GetSteerObject(i);
				CGameObject* steer = pSteerShader->GetGameObject(j);
				D3DXVECTOR3 d3dxvPosition = D3DXVECTOR3(server_info->GetSteerPosition(count).first, 200.0f, server_info->GetSteerPosition(count).second);
				D3DXVECTOR3 d3dxvLookVector = D3DXVECTOR3(server_info->GetSteerLookVector(count).first.first, server_info->GetSteerLookVector(count).first.second, server_info->GetSteerLookVector(count).second);
				if (steer->GetFirst()) {
					steer->SetPosition(d3dxvPosition);
					steer->SetLookAndGenerate(d3dxvLookVector);
					steer->SetFirst(false);
				}
				else {
					D3DXVECTOR3 d3dxvPredPosition = steer->GetPosition();
					D3DXVECTOR3 d3dxvDeltaPosition = d3dxvPosition - d3dxvPredPosition;
					D3DXVec3Normalize(&d3dxvDeltaPosition, &d3dxvDeltaPosition);
					steer->SetLookAndGenerate(-d3dxvDeltaPosition);
					steer->Move(d3dxvDeltaPosition * fTimeElapsed);
				}
				count++;
			}
		}
    }

    if (m_pMinimapScene && m_pScene) m_pMinimapScene->AnimateObjects(fTimeElapsed, m_pPlayer, m_pScene->GetNPCOwner(), server_info);
}

//
// Shadow
//
void CGameFramework::OnCreateShadowMap(ID3D11DeviceContext* pd3dDeviceContext, CConnectServer *server_info)
{
	//
	// create V, P matrixs
	//
	static float AccumulatedAngle;
	AccumulatedAngle += m_GameTimer.GetTimeElapsed();
	pair<pair<float, float>, float> d3dposition = server_info->GetSunSetPosition();

	D3DXVECTOR3 d3dpos;
	d3dpos.x = d3dposition.first.first / 10.0f;
	d3dpos.y = d3dposition.first.second / 10.0f;
	d3dpos.z = d3dposition.second / 10.0f;


	D3DXVec3Normalize(&d3dpos, &d3dpos);



	m_pScene->SetLightPosition(d3dpos);

	D3DXVECTOR3 d3dxvDir = d3dpos;//D3DXVECTOR3(0.3f, -1.0f, 0.3f);
	D3DXVec3Normalize(&d3dxvDir, &d3dxvDir);
	D3DXVECTOR3 d3dxvEye = m_pPlayer->GetPosition() + -d3dxvDir * 128.0f;
	D3DXVECTOR3 d3dxvAt = d3dxvEye + d3dxvDir * 384.0f;
	D3DXVECTOR3 d3dxvUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&m_d3dxmtxShadowView, &d3dxvEye, &d3dxvAt, &d3dxvUp);
	D3DXMatrixOrthoOffCenterLH(&m_d3dxmtxShadowProjection, -128.0f, 128.0f, -128.0f, 128.0f, -128.0f, 256.0f);

	CCamera::UpdateShaderVariable(m_pd3dDeviceContext, &m_d3dxmtxShadowView, &m_d3dxmtxShadowProjection);
	m_d3dxmtxShadowMap = m_d3dxmtxShadowView * m_d3dxmtxShadowProjection;
	m_pShadowMap->BindDSV_And_SetNullRenderTarget(pd3dDeviceContext);

	m_pScene->Render(m_pd3dDeviceContext, nullptr, m_pPlayer->GetPosition(), true);

	m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance(CConnectServer *server_info)
{
    m_GameTimer.Tick();
	if (GAME_STATE_PLAY == g_GameState)
	{
	if (particleCreateRange < 100.0f)
		particleCreateRange += m_GameTimer.GetTimeElapsed() * 30.0f;

    ProcessInput();

    AnimateObjects(server_info);

	// Shadow


	OnCreateShadowMap(m_pd3dDeviceContext, server_info);
	m_pd3dDeviceContext->PSSetSamplers(PS_SLOT_SAMPLER_SHADOW, 1, &m_pd3dShadowSS);
	m_pd3dDeviceContext->PSSetSamplers(PS_SLOT_SAMPLER_SHADOW_PCF, 1, &m_pd3dShadowPCFSS);
	m_pd3dDeviceContext->PSSetShaderResources(PS_SLOT_TEXTURE_SHADOW, 1, &m_pShadowMap->mDepthMapSRV);
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	m_pd3dDeviceContext->Map(m_pd3dcbShadow, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	D3DXMATRIX *pd3dxmtxShadowTransform = (D3DXMATRIX *)d3dMappedResource.pData;
	D3DXMatrixTranspose(pd3dxmtxShadowTransform, &m_d3dxmtxShadowMap);
	m_pd3dDeviceContext->Unmap(m_pd3dcbShadow, 0);
	m_pd3dDeviceContext->VSSetConstantBuffers(VS_CB_SLOT_SHADOW, 1, &m_pd3dcbShadow);

    if (m_pScene) m_pScene->OnPreRender(m_pd3dDeviceContext);

	// blurring
	ID3D11RenderTargetView *pd3dRenderTargetViews[1] = { m_pd3drtvOffScreen };
	m_pd3dDeviceContext->OMSetRenderTargets(1, pd3dRenderTargetViews, m_pd3dDepthStencilView);

    float fClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    if (m_pd3dRenderTargetView) m_pd3dDeviceContext->ClearRenderTargetView(m_pd3drtvOffScreen, fClearColor);
    if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (m_pPlayer) m_pPlayer->UpdateShaderVariables(m_pd3dDeviceContext);
    m_pCamera->SetViewport(m_pd3dDeviceContext);

    if (m_pScene) m_pScene->Render(m_pd3dDeviceContext, m_pCamera, m_pPlayer->GetPosition(), false);

#ifdef _WITH_PLAYER_TOP
    m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
#endif

    CMesh* PlayerMesh = m_pPlayer->GetMesh();
    m_pPlayer->GetMesh()->UpdateBoneTransform(m_pd3dDeviceContext, PlayerMesh->GetFBXAnimationNum(), PlayerMesh->GetFBXNowFrameNum());

    if (m_pPlayer) m_pPlayer->Render(m_pd3dDeviceContext, m_pCamera);

    // 미니맵
    m_pMinimapCamera->SetViewport(m_pd3dDeviceContext);
    m_pMinimapCamera->SetPosition(D3DXVECTOR3(0.0f + m_pPlayer->GetPosition().x, 100.0f, 1.0f + m_pPlayer->GetPosition().z));
    m_pMinimapCamera->GenerateViewMatrix();
    m_pMinimapCamera->UpdateShaderVariables(m_pd3dDeviceContext);
    m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    if (m_pMinimapScene) m_pMinimapScene->Render(m_pd3dDeviceContext, m_pMinimapCamera);

	// change render target view
	pd3dRenderTargetViews[0] = m_pd3dRenderTargetView;
	m_pd3dDeviceContext->OMSetRenderTargets(1, pd3dRenderTargetViews, m_pd3dDepthStencilView);

	// try blurring
	if(m_bBlur)
		SceneBlurring(20);
	else
		SceneBlurring(0);

	m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dRenderTargetView, fClearColor);
	m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// copy resource to swapchain backbuffer from OffScreen texture 2D
	DrawBlurredSceneToScreen();
		Render2D(server_info);
	}
	else if (GAME_STATE_LOADING == g_GameState)
	{
		m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dRenderTargetView, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

		Render2D();
		if (g_bBuildFinish) {
			//ShowCursor(true);//박종혁
			g_GameState = GAME_STATE_LOBBY;
			m_pCamera->SetViewport(m_pd3dDeviceContext);
			m_pCamera->SetPosition(D3DXVECTOR3(-28.0f, 86.0f, 9.0f));
			m_pCamera->SetLookAt(D3DXVECTOR3(80.0f, 11.0f, 51.0f));
			m_pCamera->RegenerateViewMatrix();
			m_pCamera->UpdateShaderVariables(m_pd3dDeviceContext);
		}
	}
	else if (GAME_STATE_LOBBY == g_GameState)
	{
		if ((m_pCamera->GetPosition().x <= 80.0f) && g_bSelectWindow) {
			D3DXVECTOR3 pos = m_pCamera->GetPosition();
			pos += m_pCamera->GetLookVector() * m_GameTimer.GetTimeElapsed() * 60.0f;
			m_pCamera->SetPosition(pos);
			m_pCamera->RegenerateViewMatrix();
			m_pCamera->UpdateShaderVariables(m_pd3dDeviceContext);
		}

		if ((m_pCamera->GetPosition().x >= -25.0f) && (!g_bSelectWindow)) {
			D3DXVECTOR3 pos = m_pCamera->GetPosition();
			pos -= m_pCamera->GetLookVector() * m_GameTimer.GetTimeElapsed() * 60.0f;
			m_pCamera->SetPosition(pos);
			m_pCamera->RegenerateViewMatrix();
			m_pCamera->UpdateShaderVariables(m_pd3dDeviceContext);
		}

		if (m_pd3dRenderTargetView)m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dRenderTargetView, D3DXCOLOR(0.3f, 0.7f, 0.9f, 1.0f));
		if (m_pd3dDepthStencilView) m_pd3dDeviceContext->ClearDepthStencilView(m_pd3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dRenderTargetView, m_pd3dDepthStencilView);

		m_pd3dDeviceContext->VSSetShader(m_pd3dVS_Lobby, nullptr, NULL);
		m_pd3dDeviceContext->PSSetShader(m_pd3dPS_Lobby, nullptr, NULL);

		m_pScene->RenderLobby(m_pd3dDeviceContext, m_pCamera);

		//m_pPlayer->Render(m_pd3dDeviceContext, m_pCamera);

		Render2D();
	}
    m_pDXGISwapChain->Present(0, 0);

	if (GAME_STATE_LOADING == g_GameState)
	{
		BuildObjects();
	}

    m_GameTimer.GetFrameRate(m_pszBuffer + 12, 37);
    ::SetWindowText(m_hWnd, m_pszBuffer);
}


// 박종혁
void CGameFramework::RenderLobbyButton()
{
	D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Identity();

	if ((GAME_STATE_LOBBY == g_GameState) && (!g_bSelectWindow))//(로비메인), (나가기)
	{
		m_pd2dContext->SetTransform(matrix);
		m_pd2dContext->DrawBitmap(m_ppd2dLoadingImage[1], D2D1::RectF(0.f, 0.f, m_nWndClientWidth / 3.0f, m_nWndClientHeight / 3.0f), 0.8f);

		matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 300, m_nWndClientHeight - 300);
		m_pd2dContext->SetTransform(matrix);

		if (g_bStart)
			m_pd2dContext->DrawBitmap(m_pp2dMainImage[1], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dMainImage[0], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);

		matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 300, m_nWndClientHeight - 200);
		m_pd2dContext->SetTransform(matrix);
		if (g_bExit)
			m_pd2dContext->DrawBitmap(m_pp2dMainImage[3], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dMainImage[2], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);
	}
	else if ((GAME_STATE_LOBBY == g_GameState) && g_bSelectWindow)//선택창 (클래스, 시작, 나가기)
	{
		matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 300, m_nWndClientHeight - 400);
		m_pd2dContext->SetTransform(matrix);

		if (g_bClassSelect)
			m_pd2dContext->DrawBitmap(m_pp2dSelectImage[1], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dSelectImage[0], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);

		matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 300, m_nWndClientHeight - 300);
		m_pd2dContext->SetTransform(matrix);

		if (g_bStart)
			m_pd2dContext->DrawBitmap(m_pp2dSelectImage[3], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dSelectImage[2], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);

		matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 300, m_nWndClientHeight - 200);
		m_pd2dContext->SetTransform(matrix);
		if (g_bExit)
			m_pd2dContext->DrawBitmap(m_pp2dSelectImage[5], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dSelectImage[4], D2D1::RectF(0.f, 0.f, 288, 60), 0.7f);

		matrix = D2D1::Matrix3x2F::Translation((m_nWndClientWidth / 2) - 64 - (m_nWndClientWidth / 6.3f), (m_nWndClientHeight / 2) - 64);
		m_pd2dContext->SetTransform(matrix);
		if (g_bLeftArrow)
			m_pd2dContext->DrawBitmap(m_pp2dArrowImage[1], D2D1::RectF(0.f, 0.f, 128.f, 128.f), 1.f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dArrowImage[0], D2D1::RectF(0.f, 0.f, 128.f, 128.f), 1.f);

		matrix = D2D1::Matrix3x2F::Translation((m_nWndClientWidth / 2) - 64 + (m_nWndClientWidth / 8.5f), (m_nWndClientHeight / 2) - 64);
		m_pd2dContext->SetTransform(matrix);
		if (g_bRightArrow)
			m_pd2dContext->DrawBitmap(m_pp2dArrowImage[3], D2D1::RectF(0.f, 0.f, 128.f, 128.f), 1.f);
		else
			m_pd2dContext->DrawBitmap(m_pp2dArrowImage[2], D2D1::RectF(0.f, 0.f, 128.f, 128.f), 1.f);

	}
	else if (GAME_STATE_LOADING == g_GameState)
	{
		m_pd2dContext->DrawBitmap(m_ppd2dLoadingImage[0], D2D1::RectF(0.f, 0.f, m_nWndClientWidth, m_nWndClientHeight), 1.0f);
		m_pd2dContext->DrawBitmap(m_ppd2dLoadingImage[1], D2D1::RectF(0.f, 0.f, m_nWndClientWidth / 3.0f, m_nWndClientHeight / 3.0f), 0.8f);
	}
}

void CGameFramework::CreateLobbyShader(ID3D11Device *pd3dDevice)
{
	int nSize;
	BYTE *pVS_LobbyShaderByteCode = ReadCompiledShaderCode(pd3dDevice, L"VS_Lobby.cso", nSize);
	pd3dDevice->CreateVertexShader(pVS_LobbyShaderByteCode, nSize, nullptr, &m_pd3dVS_Lobby);
	delete[] pVS_LobbyShaderByteCode;

	BYTE *pPS_LobbyShaderByteCode = ReadCompiledShaderCode(pd3dDevice, L"PS_Lobby.cso", nSize);
	pd3dDevice->CreatePixelShader(pPS_LobbyShaderByteCode, nSize, nullptr, &m_pd3dPS_Lobby);
	delete[] pPS_LobbyShaderByteCode;
}

void CGameFramework::Render2D() {
	m_pd2dContext->BeginDraw();

	RenderLobbyButton();

	m_pd2dContext->EndDraw();
}


////// Direct 2D /////

void CGameFramework::Build2DObjects() {
#pragma region
	LoadImageFromFile(L"Bitmap/Screenshot.png", &m_ppd2dLoadingImage[0], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/logo.png", &m_ppd2dLoadingImage[1], nullptr, 0, 0, WICBitmapTransformRotate0);

	LoadImageFromFile(L"Bitmap/button_off.jpg", &m_pp2dMainImage[0], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_on.jpg", &m_pp2dMainImage[1], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_off.jpg", &m_pp2dMainImage[2], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_on.jpg", &m_pp2dMainImage[3], nullptr, 0, 0, WICBitmapTransformRotate0);

	LoadImageFromFile(L"Bitmap/button_off.jpg", &m_pp2dSelectImage[0], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_on.jpg", &m_pp2dSelectImage[1], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_off.jpg", &m_pp2dSelectImage[2], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_on.jpg", &m_pp2dSelectImage[3], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_off.jpg", &m_pp2dSelectImage[4], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/button_on.jpg", &m_pp2dSelectImage[5], nullptr, 0, 0, WICBitmapTransformRotate0);

	LoadImageFromFile(L"Bitmap/RightArrow.png", &m_pp2dArrowImage[0], nullptr, 0, 0, WICBitmapTransformRotate180);
	LoadImageFromFile(L"Bitmap/RightArrowOn.png", &m_pp2dArrowImage[1], nullptr, 0, 0, WICBitmapTransformRotate180);
	LoadImageFromFile(L"Bitmap/RightArrow.png", &m_pp2dArrowImage[2], nullptr, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFile(L"Bitmap/RightArrowOn.png", &m_pp2dArrowImage[3], nullptr, 0, 0, WICBitmapTransformRotate0);
#pragma endregion
	//Text
	m_pdwFactory->CreateTextFormat(L"휴먼매직체", nullptr, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"ko-ko", &m_dwTextFormat);

	//SolidColor
	m_pd2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_pd2dsbrColor);
	m_pd2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.6f), &m_pd2dsbrGray);
	
	//party
	// 파티
	m_pd2dFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, 100, 100), &m_prcParty);
	m_pdwFactory->CreateTextFormat(L"휴먼매직체", nullptr, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"ko-ko", &m_pdwPartyText);



	//SolidColor
	m_pd2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::WhiteSmoke, 1.0f), &m_pd2dsbrBlackColor);

	BuildChttingBoxObjects();
	BuildPartyWindowObjects();
	BuildMonsterHPObjects();
	BuildUIObjects();
}

void CGameFramework::BuildPartyWindowObjects() {
	m_nBitmaps = 5;
	m_ppd2dButtonBitmap = new ID2D1Bitmap1*[m_nBitmaps];
	bool b = false;
	b = LoadImageFromFile(L"Image/Etc/button_on.jpg", &m_ppd2dButtonBitmap[BITMAP::ON_BUTTON], nullptr, 0, 0, WICBitmapTransformRotate0);
	b = LoadImageFromFile(L"Image/Etc/button_off.jpg", &m_ppd2dButtonBitmap[BITMAP::OFF_BUTTON], nullptr, 0, 0, WICBitmapTransformRotate0);
	b = LoadImageFromFile(L"Image/Etc/Media-Controls-Play-icon.png", &m_ppd2dButtonBitmap[BITMAP::ARROW_RIGHT_BUTTON], nullptr, 0, 0, WICBitmapTransformRotate0);
	b = LoadImageFromFile(L"Image/Etc/Media-Controls-Play-icon.png", &m_ppd2dButtonBitmap[BITMAP::ARROW_LEFT_BUTTON], nullptr, 0, 0, WICBitmapTransformRotate180);
	b = LoadImageFromFile(L"Image/Etc/party_window.png", &m_ppd2dButtonBitmap[BITMAP::PARTY_WINDOW], nullptr, 0, 0, WICBitmapTransformRotate0);
}

void CGameFramework::BuildChttingBoxObjects() {

	m_pdwFactory->CreateTextFormat(L"휴먼매직체", nullptr, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"ko-ko", &m_dwMyChattingFormat);
	m_pdwFactory->CreateTextFormat(L"휴먼매직체", nullptr, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"ko-ko", &m_dwOtherChattingFormat);

	D2D1_RECT_F rc = D2D1::RectF(0.0f, 0.0f, 600.0f, 200.0f);
	m_pd2dFactory->CreateRectangleGeometry(rc, &m_pd2drcOutputChatbox);
	rc = D2D1::RectF(0.0f, 0.0f, 600.0f, 40.0f);
	m_pd2dFactory->CreateRectangleGeometry(rc, &m_pd2drcInputChatbox);
	rc = D2D1::RectF(0.0f, 0.0f, 100.0f, 100.0f);

	m_pd2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.5f), &m_pd2dsbrChatboxColor);
}

void CGameFramework::BuildMonsterHPObjects() {
	m_vpd2drcMonsterHp.reserve(10);

	for (auto i = 0; i < 10; ++i) {
		ID2D1RectangleGeometry *prc;
		m_pd2dFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, 100, 100), &prc);
		m_vpd2drcMonsterHp.push_back(prc);
	}

	m_pd2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &m_pd2dsbrRedColor);
	m_pd2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 0.6f), &m_pd2dsbrYellowColor);

	m_pd2dFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, 100, 7), &m_pd2drcHpbar);
}

void CGameFramework::BuildUIObjects() {
	m_pdwFactory->CreateTextFormat(L"고딕체", nullptr, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"ko-ko", &m_dwStatusTextFormat);
}

void CGameFramework::Release2DObjects() {
	// 박종혁


	// 2D bitmap
	for (int i = 0; i < 2; i++)
		m_ppd2dLoadingImage[i]->Release();
	for (int i = 0; i < 4; i++)
		m_pp2dMainImage[i]->Release();
	for (int i = 0; i < 6; i++)
		m_pp2dSelectImage[i]->Release();
	for (int i = 0; i < 4; i++)
		m_pp2dArrowImage[i]->Release();

	// delete Direct 2D resource
	if (m_pd2dsbrColor) m_pd2dsbrColor->Release();
	if (m_dwTextFormat) m_dwTextFormat->Release();

	// delete Direct 2D resource


	if (m_pd2drcOutputChatbox)m_pd2drcOutputChatbox->Release();
	if (m_pd2drcInputChatbox)m_pd2drcInputChatbox->Release();
	if (m_pd2dsbrChatboxColor)m_pd2dsbrChatboxColor->Release();
	if (m_dwMyChattingFormat)m_dwMyChattingFormat->Release();
	if (m_dwOtherChattingFormat) m_dwOtherChattingFormat->Release();
	if (m_dwStatusTextFormat) m_dwStatusTextFormat->Release();
	if (m_pd2dsbrBlackColor)m_pd2dsbrBlackColor->Release();
	if (m_pd2dsbrYellowColor) m_pd2dsbrYellowColor->Release();
	if (m_pd2dsbrRedColor) m_pd2dsbrRedColor->Release();
	if (m_pd2drcHpbar)m_pd2drcHpbar->Release();
	for (auto b : m_vpd2drcMonsterHp) {
		(*b).Release();
	}
	if (m_ppd2dButtonBitmap) {
		for (auto i = 0; i < m_nBitmaps; ++i)
			m_ppd2dButtonBitmap[i]->Release();
		delete[] m_ppd2dButtonBitmap;
	}
	
	if (m_prcParty) m_prcParty->Release();
	if (m_pdwPartyText) m_pdwPartyText->Release();
	if (m_pd2dsbrGray) m_pd2dsbrGray->Release();

	// delete Direct 2D Device
	if (m_pd2dFactory)m_pd2dFactory->Release();
	if (m_pd2dDevice)m_pd2dDevice->Release();
	if (m_pd2dContext)m_pd2dContext->Release();
	if (m_pdwFactory)m_pdwFactory->Release();
	if (m_pwicFactory)m_pwicFactory->Release();
}

void CGameFramework::RenderChattingBox(CConnectServer *server_info) {
	D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation((m_nWndClientWidth - 550.0f) / 2, m_nWndClientHeight - 50.0f);
	m_pd2dContext->SetTransform(matrix);

	if (g_chat_active) {
		float str_size = g_chat_massege.size();
		m_pd2dContext->DrawGeometry(m_pd2drcInputChatbox, m_pd2dsbrBlackColor);
		m_pd2dContext->FillGeometry(m_pd2drcInputChatbox, m_pd2dsbrChatboxColor);
		matrix = D2D1::Matrix3x2F::Translation((m_nWndClientWidth - 550.0f) / 2, m_nWndClientHeight - 50.0f);
		m_pd2dContext->SetTransform(matrix);
		m_pd2dContext->DrawTextW(g_chat_massege.c_str(), str_size, m_dwMyChattingFormat, D2D1::RectF(0.0f, 0.0f, 600.0f, 40.0f), m_pd2dsbrBlackColor);
	
		matrix = D2D1::Matrix3x2F::Translation((m_nWndClientWidth - 550.0f) / 2, m_nWndClientHeight - 300.0f);
		m_pd2dContext->SetTransform(matrix);
		m_pd2dContext->FillGeometry(m_pd2drcOutputChatbox, m_pd2dsbrChatboxColor);
		m_pd2dContext->DrawGeometry(m_pd2drcOutputChatbox, m_pd2dsbrBlackColor);

		for (auto i = 0; i < 6; ++i) {
			int line = (i)* 30;
			matrix = D2D1::Matrix3x2F::Translation((m_nWndClientWidth - 550.0f) / 2, m_nWndClientHeight - 150.0f - line);
			m_pd2dContext->SetTransform(matrix);
			wstring str = server_info->GetChat(i);
			m_pd2dContext->DrawTextW(str.c_str(), str.size(), m_dwOtherChattingFormat, D2D1::RectF(0.0f, 0.0f, 600.0f, 300.0f), m_pd2dsbrBlackColor);
		}
	}
}

void CGameFramework::RenderPartyWindow(CConnectServer *server_info) {//ok

	static bool party = false;
	// g_party -> 파티 수락창 bool 값
	if (server_info->GetPartyWindow()) {
		if (!party) {
			m_pt_mouse_positoin = m_pt_mouse_move_position;
			party = true;
		}
		D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation(m_pt_mouse_positoin.x, m_pt_mouse_positoin.y);
		m_pd2dContext->SetTransform(matrix);

		D2D1_POINT_2F d2dptMouse = D2D1::Point2F(m_pt_mouse_positoin.x, m_pt_mouse_positoin.y);
		D2D1_POINT_2F d2dptMoveMouse = D2D1::Point2F(m_pt_mouse_move_position.x, m_pt_mouse_move_position.y);
		// 파티 수락 창이 뜨면
		m_pd2dContext->DrawBitmap(m_ppd2dButtonBitmap[BITMAP::PARTY_WINDOW], D2D1::RectF(0, 0, 200, 100), 0.9f);

		D2D1_RECT_F d2drc = D2D1::RectF(0.0f, 0.0f, 40.0f, 40.0f);
		// 수락
		matrix = D2D1::Matrix3x2F::Translation(d2dptMouse.x + 40.0f, d2dptMouse.y + 60.0f);
		m_pd2dContext->SetTransform(matrix);
		// button on
		if (d2dptMoveMouse.x > d2dptMouse.x + 40.0f &&
			d2dptMoveMouse.y > d2dptMouse.y + 60.0f &&
			d2dptMoveMouse.x < d2dptMouse.x + 40.0f + d2drc.right &&
			d2dptMoveMouse.y < d2dptMouse.y + 60.0f + d2drc.bottom)
			m_pd2dContext->DrawBitmap(m_ppd2dButtonBitmap[BITMAP::OFF_BUTTON], d2drc);
		// button off
		else
			m_pd2dContext->DrawBitmap(m_ppd2dButtonBitmap[BITMAP::ON_BUTTON], d2drc);
		// click
		if (m_ptCursorPOS.x > d2dptMouse.x + 40.0f &&
			m_ptCursorPOS.y > d2dptMouse.y + 60.0f &&
			m_ptCursorPOS.x < d2dptMouse.x + 40.0f + d2drc.right &&
			m_ptCursorPOS.y < d2dptMouse.y + 60.0f + d2drc.bottom) {
			// 수락 process
			sendJoinParty();
			server_info->SetPartyWindow();
			party = false;
		}
		// 거절
		matrix = D2D1::Matrix3x2F::Translation(d2dptMouse.x + 120.0f, d2dptMouse.y + 60.0f);
		m_pd2dContext->SetTransform(matrix);
		// button on
		if (d2dptMoveMouse.x > d2dptMouse.x + 120.0f &&
			d2dptMoveMouse.y > d2dptMouse.y + 60.0f &&
			d2dptMoveMouse.x < d2dptMouse.x + 120.0f + d2drc.right &&
			d2dptMoveMouse.y < d2dptMouse.y + 60.0f + d2drc.bottom)
			m_pd2dContext->DrawBitmap(m_ppd2dButtonBitmap[BITMAP::OFF_BUTTON], d2drc);
		// button off
		else
			m_pd2dContext->DrawBitmap(m_ppd2dButtonBitmap[BITMAP::ON_BUTTON], d2drc);
		// click

		if (m_ptCursorPOS.x > d2dptMouse.x + 120.0f &&
			m_ptCursorPOS.y > d2dptMouse.y + 60.0f &&
			m_ptCursorPOS.x < d2dptMouse.x + 120.0f + d2drc.right &&
			m_ptCursorPOS.y < d2dptMouse.y + 60.0f + d2drc.bottom) {
			// 거절 process
			sendNoParty();
			server_info->SetPartyWindow();
			party = false;
		}
	}
}

void CGameFramework::RenderMonsterHp(CConnectServer *server_info) {
	for (auto i = 0; i < m_vd3dvec2.size(); ++i) {
		if (i == m_vpd2drcMonsterHp.size()) break;
		D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Identity();
		m_pd2dContext->SetTransform(matrix);
		matrix = D2D1::Matrix3x2F::Translation(m_vd3dvec2[i].second.x - 30.0f, m_nWndClientHeight - m_vd3dvec2[i].second.y - 60.0f);
		m_pd2dContext->SetTransform(matrix);
		
		if(server_info->GetEnemyHP(m_vd3dvec2[i].first) > 0) m_pd2dContext->FillGeometry(m_pd2drcHpbar, m_pd2dsbrYellowColor);

		matrix = D2D1::Matrix3x2F::Translation(m_vd3dvec2[i].second.x - 30.0f, m_nWndClientHeight - m_vd3dvec2[i].second.y - 59.0f);
		m_pd2dContext->SetTransform(matrix);
		m_pd2dFactory->CreateRectangleGeometry(D2D1::RectF(0.0f, 0.0f, server_info->GetEnemyHP(m_vd3dvec2[i].first), 5.0f), &m_vpd2drcMonsterHp[i]);
		
		m_pd2dContext->FillGeometry(m_vpd2drcMonsterHp[i], m_pd2dsbrRedColor);
	}
	m_vd3dvec2.clear();
}

void CGameFramework::RenderPartUI(CConnectServer *server_info) {

	
	for (auto i = 0; i < 3; ++i) {
		if (party_members[i].pHP != -5) {
			D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 150, 50 + (i * 130));
			m_pd2dContext->SetTransform(matrix);
			wchar_t text[20];
			wmemset(text, 0, 20);
			wsprintf(text, L" %d / %d ", party_members[i].pLevel, party_members[i].pHP);
			m_pd2dContext->FillGeometry(m_prcParty, m_pd2dsbrGray);
			m_pd2dContext->DrawTextW(text, 20, m_pdwPartyText, D2D1::RectF(0, 0, 100, 100), m_pd2dsbrBlackColor);
			matrix = D2D1::Matrix3x2F::Translation(m_nWndClientWidth - 150, 70 + (i * 130));
			m_pd2dContext->SetTransform(matrix);
			text[20];
			wmemset(text, 0, 20);
			wcsncpy_s(text, party_members[i].party_member, 20);
			m_pd2dContext->DrawTextW(text, 20, m_pdwPartyText, D2D1::RectF(0, 0, 100, 100), m_pd2dsbrBlackColor);
		}
	}

	//hp
	//level
	//name
}

void CGameFramework::RenderUI(CConnectServer *server_info) {

	// Status Render
	D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Translation((50.0f), m_nWndClientHeight - 300.0f);
	m_pd2dContext->SetTransform(matrix);
	int str_size = 30;
	wchar_t text[30];
	wmemset(text, 0, str_size);

	pair<pair<float, float>, float> st = server_info->GetMyPlayerStatus();

	int level = st.first.first;
	wsprintf(text, L"Level : %d", level);
	m_pd2dContext->DrawTextW(text, str_size, m_dwStatusTextFormat, D2D1::RectF(0, 0, 200, 100), m_pd2dsbrBlackColor);

	matrix = D2D1::Matrix3x2F::Translation((50.0f), m_nWndClientHeight - 270.0f);
	m_pd2dContext->SetTransform(matrix);
	wmemset(text, 0, str_size);
	int status = st.first.second;
	wsprintf(text, L"HP : %d / %d", status, level * 100);
	m_pd2dContext->DrawTextW(text, str_size, m_dwStatusTextFormat, D2D1::RectF(0, 0, 300, 100), m_pd2dsbrBlackColor);

	matrix = D2D1::Matrix3x2F::Translation((50.0f), m_nWndClientHeight - 240.0f);
	m_pd2dContext->SetTransform(matrix);
	wmemset(text, 0, str_size);
	status = st.second;
	wsprintf(text, L"EXP : %d / %d", status, level * 500);
	m_pd2dContext->DrawTextW(text, str_size, m_dwStatusTextFormat, D2D1::RectF(0, 0, 300, 100), m_pd2dsbrBlackColor);


	matrix = D2D1::Matrix3x2F::Translation(m_nWndClientHeight - 50.0f, m_nWndClientHeight - 240.0f);
	m_pd2dContext->SetTransform(matrix);
	CNode* begin = m_pPlayer->getBulletList()->getHead()->m_pNext;
	CNode* end = m_pPlayer->getBulletList()->getTail();
	static int count = 0;
	while (begin != end) {
		count++;
		begin = begin->m_pNext;
	}
	wmemset(text, 0, str_size);
	wsprintf(text, L"BULLET : %d / %d", 30 - m_pPlayer->getBulletList()->GetBulletCount(), 30);
	m_pd2dContext->DrawTextW(text, str_size, m_dwStatusTextFormat, D2D1::RectF(0, 0, 300, 100), m_pd2dsbrBlackColor);
}

void CGameFramework::Render2D(CConnectServer *server_info) {
	m_pd2dContext->BeginDraw();
	D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Identity();

	RenderChattingBox(server_info);
	RenderPartyWindow(server_info);
	RenderMonsterHp(server_info);
	RenderUI(server_info);
	RenderPartUI(server_info);

	m_pd2dContext->EndDraw();
}

bool CGameFramework::LoadImageFromFile(_TCHAR *pszstrFileName, ID2D1Bitmap1 **ppd2dBitmap, D2D_RECT_U *pd2drcImage, UINT nWidth, UINT nHeight, WICBitmapTransformOptions nFlipRotation) {
	HRESULT hResult;
	IWICBitmapDecoder *pwicBitmapDecoder = NULL;
	if (FAILED(hResult = m_pwicFactory->CreateDecoderFromFilename(pszstrFileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder))) return(false);
	IWICBitmapFrameDecode *pwicBitmapFrameDecode = NULL;
	if (FAILED(hResult = pwicBitmapDecoder->GetFrame(0, &pwicBitmapFrameDecode))) return(false);
	IWICBitmapSource *pwicSource = pwicBitmapFrameDecode;
	UINT nImageWidth, nImageHeight;
	if (FAILED(hResult = pwicSource->GetSize(&nImageWidth, &nImageHeight))) return(false);
	IWICFormatConverter	*pwicFormatConverter = NULL;
	IWICBitmapScaler *pwicScaler = NULL;
	IWICBitmapClipper *pwicClipper = NULL;
	IWICBitmapFlipRotator *pwicFlipRotator = NULL;
	if (pd2drcImage)
	{
		if (pd2drcImage->left < 0) pd2drcImage->left = 0;
		if (pd2drcImage->top < 0) pd2drcImage->top = 0;
		if (pd2drcImage->right > nImageWidth) pd2drcImage->right = nImageWidth;
		if (pd2drcImage->bottom > nImageHeight) pd2drcImage->bottom = nImageHeight;
		WICRect wicRect = { pd2drcImage->left, pd2drcImage->top, (pd2drcImage->right - pd2drcImage->left), (pd2drcImage->bottom - pd2drcImage->top) };
		if (FAILED(hResult = m_pwicFactory->CreateBitmapClipper(&pwicClipper))) return(false);
		if (FAILED(hResult = pwicClipper->Initialize(pwicSource, &wicRect))) return(false);
		pwicSource = pwicClipper;
	}
	if ((nWidth != 0) || (nHeight != 0))
	{
		if (nWidth == 0) nWidth = UINT(float(nHeight) / float(nImageHeight) * float(nImageWidth));
		if (nHeight == 0) nHeight = UINT(float(nWidth) / float(nImageWidth) * float(nImageHeight));
		if (FAILED(hResult = m_pwicFactory->CreateBitmapScaler(&pwicScaler))) return(false);
		if (FAILED(hResult = pwicScaler->Initialize(pwicSource, nWidth, nHeight, WICBitmapInterpolationModeCubic))) return(false);
		pwicSource = pwicScaler;
	}
	if (nFlipRotation != WICBitmapTransformRotate0)
	{
		if (FAILED(hResult = m_pwicFactory->CreateBitmapFlipRotator(&pwicFlipRotator))) return(false);
		if (FAILED(hResult = pwicFlipRotator->Initialize(pwicSource, nFlipRotation))) return(false);
		pwicSource = pwicFlipRotator;
	}
	if (FAILED(hResult = m_pwicFactory->CreateFormatConverter(&pwicFormatConverter))) return(false);
	if (FAILED(hResult = pwicFormatConverter->Initialize(pwicSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut))) return(false);
	if (FAILED(hResult = m_pd2dContext->CreateBitmapFromWicBitmap(pwicFormatConverter, NULL, ppd2dBitmap))) return(false);

	if (pwicBitmapFrameDecode) pwicBitmapFrameDecode->Release();
	if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
	if (pwicFormatConverter) pwicFormatConverter->Release();
	if (pwicClipper) pwicClipper->Release();
	if (pwicScaler) pwicScaler->Release();
	if (pwicFlipRotator) pwicFlipRotator->Release();

	return(true);
}
