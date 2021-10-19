#include "stdafx.h"
#include "Framework.h"

std::string s;
CFramework::CFramework()
{

	_tcscpy_s(m_pszBuffer, _T("J.H_2DGame ("));
	srand(timeGetTime());
	m_pTimer = new CTimer();

	/******* Device **********/
	m_pd2dFactory = NULL;
	m_pdwFactory = NULL;
	m_pwicFactory = NULL;
	m_pd2dRenderTarget = NULL;

	m_pdwPositionText = NULL;
	m_pd2dsbrBlack = NULL;
	/*************************/

	m_pPlayer = NULL;
	m_pScene = NULL;
	m_p_ui_scene = nullptr;

	m_pEnemy_npc = NULL;

	m_fXpos = 50.0f;
	m_fYpos = 50.0f;

	state = 0;
	m_nPlayerCount = 1;

	m_vOtherPlayer.reserve(MAX_USER);
	m_vEnemyNpc.reserve(MAX_ENEMY);

	m_wsa_send_buf.buf = m_csend_buf;
	m_wsa_send_buf.len = BUFF_SIZE;

	m_fLimittedWidth = 0.0f;
	m_fLimittedHeight = 0.0f;

	str.clear();
	chat_active = false;
	collsion = false;
}
CFramework::~CFramework()
{
	if (m_pTimer) delete m_pTimer;
	if (m_p_ui_scene) delete m_p_ui_scene;
	if (m_pScene)delete m_pScene;
}
bool CFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	return(true);
}
void CFramework::OnDestroy()
{
	ReleaseObjects();
	ReleaseDeviceDependentResources();
	ReleaseDeviceIndependentResources();
}
bool CFramework::CreateDeviceIndependentResources()
{
	HRESULT hResult;
	if (FAILED(hResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pd2dFactory))) return(false);
	if (FAILED(hResult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_pdwFactory))) return(false);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hResult = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void **)&m_pwicFactory))) return(false);

	return(true);
}
bool CFramework::CreateDeviceDependentResources()
{
	HRESULT hResult;
	D2D1_SIZE_U d2dSize = D2D1::SizeU(m_nWndClientWidth, m_nWndClientHeight);
	D2D1_RENDER_TARGET_PROPERTIES d2dRTProps = D2D1::RenderTargetProperties();
#ifdef _WITH_WAIT_DISPLAY_REFRESH
	D2D1_HWND_RENDER_TARGET_PROPERTIES d2dHwndRTProps = D2D1::HwndRenderTargetProperties(m_hWnd, d2dSize, D2D1_PRESENT_OPTIONS_NONE);
#else
	D2D1_HWND_RENDER_TARGET_PROPERTIES d2dHwndRTProps = D2D1::HwndRenderTargetProperties(m_hWnd, d2dSize, D2D1_PRESENT_OPTIONS_IMMEDIATELY);
#endif
	if (FAILED(hResult = m_pd2dFactory->CreateHwndRenderTarget(d2dRTProps, d2dHwndRTProps, &m_pd2dRenderTarget))) return(false);

	m_pdwFactory->CreateTextFormat(L"궁서체", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0F, L"en-us", &m_pdwPositionText);

	return(true);
}
void CFramework::ReleaseDeviceIndependentResources()
{
	if (m_pwicFactory) m_pwicFactory->Release();
	if (m_pd2dFactory) m_pd2dFactory->Release();
	if (m_pdwFactory) m_pdwFactory->Release();

}
void CFramework::ReleaseDeviceDependentResources()
{
	if (m_pd2dRenderTarget) m_pd2dRenderTarget->Release();
	if (m_pdwPositionText) m_pdwPositionText->Release();
	if (m_pd2dsbrBlack) m_pd2dsbrBlack->Release();
	//if (m_ppd2dBitmap) {
	//	for (auto i = 0; i < m_nBitmap; ++i) {
	//		m_ppd2dBitmap[i]->Release();
	//	}
	//	delete[] m_ppd2dBitmap;
	//}

}
void CFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
void CFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	bool move = false;
	switch (nMessageID)
	{
	case WM_KEYDOWN: {
		switch (wParam)
		{
		case VK_RETURN:
			if (chat_active) {
				// send message
				if (str.empty()) break;
				cs_packet_player_speach *packet = reinterpret_cast<cs_packet_player_speach*>(m_csend_buf);
				packet->size = sizeof(cs_packet_player_speach);
				m_wsa_send_buf.len = sizeof(cs_packet_player_speach);
				DWORD iobyte;
				packet->type = PLAYER_SPEACH;
				wcsncpy_s(packet->message, str.c_str(), str.size());
				int retval = WSASend(m_client_sock, &m_wsa_send_buf, 1, &iobyte, 0, NULL, NULL);
				if (retval == SOCKET_ERROR) {
					while (true);
				}
				str.clear();
				chat_active = false;
			}
			else chat_active = true;
			break;
		case VK_UP: {
			move = true;
			state = VK_UPBUTTON;
			break;
		}
		case VK_DOWN: {
			move = true;
			state = VK_DOWNBUTTON;
			break;
		}
		case VK_RIGHT: {
			move = true;
			state = VK_RIGHTBUTTON;
			break;
		}
		case VK_LEFT: {
			move = true;
			state = VK_LEFTBUTTON;
			break;
		}
		case VK_SPACE: {
			move = true;
			state = VK_SPACEBAR;
			str.push_back(' ');
			// draw attack effect
			break;
		}
		case VK_BACK: {
			state = IDLE;
			if (str.empty()) break;
			//str.pop_back();
			break;
		}
		default: state = IDLE;  break;
		}
		if (move) {
			CS_Player_State *packet = reinterpret_cast<CS_Player_State*>(m_csend_buf);
			packet->size = sizeof(CS_Player_State);
			m_wsa_send_buf.len = sizeof(CS_Player_State);
			DWORD iobyte;
			packet->type = state;
			int retval = WSASend(m_client_sock, &m_wsa_send_buf, 1, &iobyte, 0,
				NULL, NULL);
			if (retval) {
				// error code
			}
		}
	}
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			//std::cout << "enter  : " <<chat_active << std::endl;
			//if (chat_active) chat_active = false;
			//else chat_active = true;
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			break;
		case 'A':
			//::PostQuitMessage(0);
			break;
		case 'S':
		case 's':
			break;
		case VK_UP:
		case VK_DOWN:
		case VK_RIGHT:
		case VK_LEFT:
		case VK_SPACE:
			state = IDLE;
			break;
		default:
			break;
		}
	default:
		break;
	}
}
LRESULT CALLBACK CFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	static HIMC imeID = nullptr;
	
	HIMC hImc = ImmGetContext(hWnd);
	bool flag = ImmGetOpenStatus(hImc);
	if (flag) {
		int i = 2;
		int k = i;
		k = k + i;
	}
	switch (nMessageID)
	{
	case WM_ACTIVATE: {
		if (imeID == nullptr) {
			imeID = ImmCreateContext();
			ImmAssociateContext(hWnd, imeID);
			printf("asdsadsad");
		}
	}break;
	case WM_IME_STARTCOMPOSITION: break;
	case WM_IME_CHAR: if(chat_active)str.push_back((wchar_t)wParam); break;
	case WM_CHAR: 
		if (!chat_active) break;
		if ((wchar_t)wParam == '\b') {
			if (str.empty()) break;
			str.pop_back();
			break;
		}
		str.push_back((wchar_t)wParam); break;
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		m_ptCursorPos.x = LOWORD(LPARAM(lParam));
		m_ptCursorPos.y = HIWORD(LPARAM(lParam));
		break;
	}
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}
bool CFramework::LoadImageFromFileLoadImageFromFile(_TCHAR *pszstrFileName, ID2D1Bitmap **ppd2dBitmap, D2D_RECT_U *pd2drcImage, UINT nWidth, UINT nHeight, WICBitmapTransformOptions nFlipRotation) {
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
	if (FAILED(hResult = m_pd2dRenderTarget->CreateBitmapFromWicBitmap(pwicFormatConverter, NULL, ppd2dBitmap))) return(false);

	if (pwicBitmapFrameDecode) pwicBitmapFrameDecode->Release();
	if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
	if (pwicFormatConverter) pwicFormatConverter->Release();
	if (pwicClipper) pwicClipper->Release();
	if (pwicScaler) pwicScaler->Release();
	if (pwicFlipRotator) pwicFlipRotator->Release();

	return(true);
}
void CFramework::BuildObjects()
{
	m_nBitmap = 9;
	m_ppd2dBitmap = new ID2D1Bitmap*[m_nBitmap];

	const auto player_bitmap = 0;
	const auto other_plater_bitmap = 1;
	const auto npc_beginer_bitmap = 2;
	const auto npc_static_attack_bitmap = 3;
	const auto npc_dynamic_no_attack_bitmap = 4;
	const auto npc_dynamic_attack_bitmpac = 5;
	const auto npc_super_bitmap = 6;
	const auto obtacle_rock = 7;
	const auto obtacle_tree = 8;
	//bitmap load
	//player


	LoadImageFromFileLoadImageFromFile(L"image/Player/warrior.jpg", &m_ppd2dBitmap[player_bitmap], NULL, 0, 0, WICBitmapTransformRotate0);
	//other player
	LoadImageFromFileLoadImageFromFile(L"image/Player/mage.jpg", &m_ppd2dBitmap[other_plater_bitmap], NULL, 0, 0, WICBitmapTransformRotate0);
	//npc_4
	LoadImageFromFileLoadImageFromFile(L"image/Zombies/beginer_zombie_static.png", &m_ppd2dBitmap[npc_beginer_bitmap], NULL, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFileLoadImageFromFile(L"image/Zombies/first_attack_zombie_static.png", &m_ppd2dBitmap[npc_static_attack_bitmap], NULL, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFileLoadImageFromFile(L"image/Zombies/beginer_zombie_move.png", &m_ppd2dBitmap[npc_dynamic_no_attack_bitmap], NULL, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFileLoadImageFromFile(L"image/Zombies/first_attack_zombie_move.png", &m_ppd2dBitmap[npc_dynamic_attack_bitmpac], NULL, 0, 0, WICBitmapTransformRotate0);
	LoadImageFromFileLoadImageFromFile(L"image/Zombies/hp_up_zombie.png", &m_ppd2dBitmap[npc_super_bitmap], NULL, 0, 0, WICBitmapTransformRotate0);
	// obtacle_rock
	LoadImageFromFileLoadImageFromFile(L"image/rock.png", &m_ppd2dBitmap[obtacle_rock], NULL, 0, 0, WICBitmapTransformRotate0);
	// obtacle_tree
	LoadImageFromFileLoadImageFromFile(L"image/wood.png", &m_ppd2dBitmap[obtacle_tree], NULL, 0, 0, WICBitmapTransformRotate0);
	// obtacle_water

	//Player
	m_pPlayer = new CPlayer();
	m_pPlayer->SetBitmap(m_ppd2dBitmap[player_bitmap]);
	m_pPlayer->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);


	//MAP
	m_pScene = new CScene();
	m_pScene->BulidObject(m_pd2dFactory, m_pd2dRenderTarget);

	CGameObject** object = m_pScene->GetRect();
	int nobject = m_pScene->GetObejctsEA();
	for (auto i = 0; i < 900; ++i) {
		object[i]->SetBitmap(m_ppd2dBitmap[obtacle_rock]);
	}
	for (auto i = 80900; i < nobject; ++i) {
		object[i]->SetBitmap(m_ppd2dBitmap[obtacle_tree]);
	}


	// UI
	m_p_ui_scene = new CScene();
	CUIScene *pui_scene = new CUIScene();
	pui_scene->SetPlayer(m_pPlayer);
	pui_scene->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);
	m_p_ui_scene = pui_scene;

	m_vOtherPlayer.resize(MAX_USER);
	m_vEnemyNpc.resize(MAX_ENEMY);
	//other player
	for (auto i = 0; i < MAX_USER; ++i) {
		m_vOtherPlayer[i] = new CPlayer();
		m_vOtherPlayer[i]->SetBitmap(m_ppd2dBitmap[other_plater_bitmap]);
		m_vOtherPlayer[i]->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);
		m_vOtherPlayer[i]->Set_my_user_id(L"other_player");
	}
	//NPC
	for (auto i = 0; i < 1000; ++i) {
		m_vEnemyNpc[i] = new CEnemy();
		m_vEnemyNpc[i]->SetName(L"CHILD ZOMBIE");
		m_vEnemyNpc[i]->SetBitmap(m_ppd2dBitmap[npc_beginer_bitmap]);
		m_vEnemyNpc[i]->BulidObject(m_pd2dFactory, m_pd2dRenderTarget,m_pdwFactory);
	}
	for (auto i = 1000; i < 2000; ++i) {
		m_vEnemyNpc[i] = new CEnemy();
		m_vEnemyNpc[i]->SetName(L"ADULT ZOMBIE");
		m_vEnemyNpc[i]->SetBitmap(m_ppd2dBitmap[npc_static_attack_bitmap]);
		m_vEnemyNpc[i]->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);
	}
	for (auto i = 2000; i < 3000; ++i) {
		m_vEnemyNpc[i] = new CEnemy();
		m_vEnemyNpc[i]->SetName(L"STRONG ZOMBIE");
		m_vEnemyNpc[i]->SetBitmap(m_ppd2dBitmap[npc_dynamic_no_attack_bitmap]);
		m_vEnemyNpc[i]->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);
	}
	for (auto i = 3000; i < 4000; ++i) {
		m_vEnemyNpc[i] = new CEnemy();
		m_vEnemyNpc[i]->SetName(L"FAST ZOMBIE");
		m_vEnemyNpc[i]->SetBitmap(m_ppd2dBitmap[npc_dynamic_attack_bitmpac]);
		m_vEnemyNpc[i]->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);
	}
	for (auto i = 4000; i < MAX_ENEMY; ++i) {
		m_vEnemyNpc[i] = new CEnemy();
		m_vEnemyNpc[i]->SetName(L"SUPER ZOMBIE");
		m_vEnemyNpc[i]->SetBitmap(m_ppd2dBitmap[npc_super_bitmap]);
		m_vEnemyNpc[i]->BulidObject(m_pd2dFactory, m_pd2dRenderTarget, m_pdwFactory);
	}

}
void CFramework::ReleaseObjects()
{
	if (m_pPlayer) delete[] m_pPlayer;
	for (auto i = 0; i < m_vEnemyNpc.size(); ++i) {
		delete m_vEnemyNpc[i];
	}
	for (auto i = 0; i < m_vOtherPlayer.size(); ++i) {
		delete m_vOtherPlayer[i];
	}
}
void CFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	if (!bProcessedByScene)
	{
		static UCHAR pKeyBuffer[256];
		DWORD dwDirection = 0;
		if (GetKeyboardState(pKeyBuffer))
		{
			if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
			if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
			if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
		}
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}
		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				//if (pKeyBuffer[VK_RBUTTON] & 0xF0) ...
			}
			//if (dwDirection) ...
		}
	}
}
void CFramework::AnimateObjects(CConnectServer* server_info)
{
	static bool first = true;
	m_fLimittedWidth = server_info->GetMyPlayerPos().first;
	m_fLimittedHeight = server_info->GetMyPlayerPos().second;
	//m_pPlayer->SetPosition(m_fLimittedWidth, m_fLimittedHeight);
	m_nMy_id = server_info->GetMYid();
	if (first) {
		m_pPlayer->SetPosition(420, 420);
		first = false;
	}
	m_pPlayer->SetPlayerStat(server_info->GetMyPlayerLevel(), server_info->GetMyPlayerExp(), server_info->GetMyPlayerHp());
	m_pPlayer->setspeach_text(server_info->GetMyPlayerSpeach().message);
	m_pPlayer->set_speach_time(server_info->GetMyPlayerSpeach().message_time);
	m_pPlayer->Set_my_user_id(server_info->GetUserID());
	
	for (auto i = 0; i < MAX_USER; ++i) {
		if (server_info->GetOtherPlayerView(i) == false) continue;
		if (i != m_nMy_id) {
			float x = server_info->GetOtherPlayerPos(i).first - m_fLimittedWidth + 420.0f;
			float y = server_info->GetOtherPlayerPos(i).second - m_fLimittedHeight + 420.0f;
			m_vOtherPlayer[i]->SetPosition(x, y);
			m_vOtherPlayer[i]->setspeach_text(server_info->GetOtherPlayerSpeach(i).message);
			m_vOtherPlayer[i]->set_speach_time(server_info->GetOtherPlayerSpeach(i).message_time);
			//m_vOtherPlayer[i]->Set_my_user_id(server_info->GetOhterPlayerID(i));
		}
	}
	for (auto i = 0; i < MAX_ENEMY; ++i) {
		if (server_info->GetEnemyNPCView(i) == false) continue;
		float x = server_info->GetEnemyNPCPos(i).first - m_fLimittedWidth + 420.0f;
		float y = server_info->GetEnemyNPCPos(i).second - m_fLimittedHeight + 420.0f;
		m_vEnemyNpc[i]->SetPosition(x, y);
		m_vEnemyNpc[i]->setspeach_text(server_info->GetEnemySpeach(i).message);
		m_vEnemyNpc[i]->set_speach_time(server_info->GetEnemySpeach(i).message_time);
		m_vEnemyNpc[i]->SetHp(server_info->GetEnemyHP(i));
	}
	m_pScene->SetPlayerPos(m_fLimittedWidth, m_fLimittedHeight);
}
void CFramework::FrameAdvance(CConnectServer* server_info)
{
	m_pTimer->Tick();

	m_pd2dRenderTarget->BeginDraw();
	m_pd2dRenderTarget->SetTransform(Matrix3x2F::Identity());
	m_pd2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f));

	if (m_pScene) m_pScene->Draw(m_pd2dRenderTarget);
	for (auto i = 0; i < MAX_ENEMY; ++i) {
		if (false == server_info->GetEnemyNPCView(i)) continue;
		if (m_vEnemyNpc[i]) m_vEnemyNpc[i]->Draw(m_pd2dRenderTarget);
	}
	if (m_pPlayer) m_pPlayer->Draw(m_pd2dRenderTarget);
	for (auto i = 0; i < MAX_USER; ++i) {
		if (server_info->GetOtherPlayerView(i) == false) continue;
		if (i != m_nMy_id) {
			if (m_vOtherPlayer[i]) m_vOtherPlayer[i]->Draw(m_pd2dRenderTarget);
		}
	}

	if(m_p_ui_scene) m_p_ui_scene->Draw(m_pd2dRenderTarget);

	m_pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &m_pd2dsbrBlack);
	Matrix3x2F d3dPosTextBoxmtx = Matrix3x2F::Translation(300.0F, 0.0f);
	m_pd2dRenderTarget->SetTransform(d3dPosTextBoxmtx);
	wchar_t text[22];
	wmemset(text, 0, 22);
	wsprintf(text, L" POS = ( %d , %d ) ", ((int)m_fLimittedWidth) / 40, ((int)m_fLimittedHeight) / 40);
	m_pd2dRenderTarget->DrawTextW(text, 22, m_pdwPositionText, RectF(0.0f, 0.0f, 500.0f, 100.0f), m_pd2dsbrBlack);

	if (server_info->GetStateMessage().message_time > GetTickCount() - 2000) {
		wchar_t message[256];
		wmemset(message, 0, 256);
		wcsncpy_s(message, server_info->GetStateMessage().message, MAX_STR_SIZE);
		d3dPosTextBoxmtx = Matrix3x2F::Translation(0.0f, FRAME_BUFFER_HEIGHT - 100.0f);
		m_pd2dRenderTarget->SetTransform(d3dPosTextBoxmtx);
		m_pd2dRenderTarget->DrawTextW(message, 256, m_pdwPositionText, RectF(0.0f, 0.0f, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT), m_pd2dsbrBlack);
	}

	if (chat_active) {
		IDWriteTextLayout *layout;
		ID2D1RectangleGeometry *pd2drc;
		float f = str.size();
		Matrix3x2F mtx = Matrix3x2F::Translation(m_pPlayer->GetXpos() - 30.0f , m_pPlayer->GetYpos() - 50.0f);
		//m_pd2dRenderTarget->SetTransform(mtx);
		m_pd2dFactory->CreateRectangleGeometry(RectF(0.0f, 0.0f, f, f), &pd2drc);
		m_pdwFactory->CreateTextLayout(str.c_str(), str.size(), m_pdwPositionText, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, &layout);
		DWRITE_TEXT_RANGE range = DWRITE_TEXT_RANGE{ MAX_STR_SIZE };
		m_pd2dRenderTarget->DrawTextW(str.c_str(), str.size(), m_pdwPositionText, RectF(0.0f, 0.0f, FRAME_BUFFER_WIDTH / 2, FRAME_BUFFER_HEIGHT / 2), m_pd2dsbrBlack);
		//m_pd2dRenderTarget->DrawTextLayout(D2D1::Point2F(m_pPlayer->GetXpos(), m_pPlayer->GetYpos()), layout, m_pd2dsbrBlack);
		m_pd2dRenderTarget->FillGeometry(pd2drc, m_pd2dsbrBlack);
	}
	HRESULT hResult = m_pd2dRenderTarget->EndDraw();
	if (hResult == D2DERR_RECREATE_TARGET)
	{
		ReleaseDeviceDependentResources();
		CreateDeviceDependentResources();
		ReleaseObjects();
		BuildObjects();
	}
	m_pTimer->GetFrameRate(m_pszBuffer + 12, 37);
	::SetWindowText(m_hWnd, m_pszBuffer);
}