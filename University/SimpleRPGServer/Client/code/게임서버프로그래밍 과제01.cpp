// TestClient.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Framework.h"
#include "ConnectServer.h"
#include "게임서버프로그래밍 과제01.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];					// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

HWND hIp;
HWND hId;
TCHAR tc_ip[20];
char c_ip[20];

TCHAR tc_id[20];
char c_id[20];

CFramework gGameFrameWork;
CConnectServer g_Connect_server;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MY01, szWindowClass, MAX_LOADSTRING);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01));
	MSG msg = { 0 };

	// 기본 메시지 루프입니다.
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			gGameFrameWork.AnimateObjects(&g_Connect_server);
			gGameFrameWork.FrameAdvance(&g_Connect_server);
		}
	}
	g_Connect_server.DisConnectSerever();
	gGameFrameWork.OnDestroy();
	return (int)msg.wParam;

}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY01));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL/*MAKEINTRESOURCE(IDC_LABPROJECT03)*/;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	if (!RegisterClassEx(&wcex)) return(FALSE);

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);


	if (!gGameFrameWork.CreateDeviceIndependentResources()) return(false);



	ID2D1Factory *pd2dFactory = gGameFrameWork.GetD2DFactory();
	float fdpiX, fdpiY;
	pd2dFactory->GetDesktopDpi(&fdpiX, &fdpiY);
	UINT nWidth = UINT(rc.right - rc.left);
	UINT nHeight = UINT(rc.bottom - rc.top);
	nWidth = (UINT)ceil(nWidth * fdpiX / 96.f);
	nHeight = (UINT)ceil(nHeight * fdpiY / 96.f);

	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

	if (!gGameFrameWork.OnCreate(hInstance, hMainWnd)) return(FALSE);
	if (!gGameFrameWork.CreateDeviceDependentResources()) return(false);


	gGameFrameWork.BuildObjects();
	g_Connect_server.ConnectServerInitialize(hMainWnd,c_id, c_ip);
	gGameFrameWork.SetSokcet(g_Connect_server.GetSocket());

	ShowWindow(hMainWnd, nCmdShow);

	return(TRUE);
}
void UpdataIPandID() {
	WideCharToMultiByte(CP_ACP, 0, tc_ip, -1, c_ip, sizeof(c_ip), NULL, FALSE);
	WideCharToMultiByte(CP_ACP, 0, tc_id, -1, c_id, sizeof(c_id), NULL, FALSE);

}
BOOL CALLBACK InputServerIpDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	char ip[20];
	TCHAR ipTmp[20];

	char id[20];
	TCHAR idTmp[20];

	strcpy(ip, "127.0.0.1");
	MultiByteToWideChar(CP_ACP, 0, ip, strlen(ip) + 1, ipTmp, strlen(ip) + 1);

	strcpy(id, "aa");
	MultiByteToWideChar(CP_ACP, 0, id, strlen(id) + 1, idTmp, strlen(id) + 1);

	switch (message) {
	case WM_INITDIALOG:
		hIp = GetDlgItem(hDlg, IDC_IP);
		SetWindowText(hIp, ipTmp);
		hId = GetDlgItem(hDlg, IDC_ID);
		SetWindowText(hId, idTmp);
		break;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK: 
			GetWindowText(hIp, tc_ip, 20);
			GetWindowText(hId, tc_id, 20);
			UpdataIPandID();
			EndDialog(hDlg, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		}
		break;
	}

	return(DefWindowProc(hDlg, message, wParam, lParam));
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG), HWND_DESKTOP, InputServerIpDlgProc);
		return 0;

	case WM_ACTIVATE:
	case WM_IME_STARTCOMPOSITION: 
	case WM_IME_CHAR:
	case WM_CHAR:
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFrameWork.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SOCKET:
		if (WSAGETSELECTERROR(lParam)) {
			closesocket((SOCKET)wParam);
			exit(-1); // 클라이언트 강제 종료
		}

		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			g_Connect_server.ReadRecvPacket((SOCKET)wParam);
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			exit(-1);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return(0);
}