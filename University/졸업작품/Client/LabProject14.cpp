// LabProject14.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//
//ksh
#define WIN32_LEAN_AND_MEAN  
#define INITGUID
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//ksh
#include "stdafx.h"
#include "LabProject14.h"
#include "ConnectServer.h"
#include "GameFramework.h"

#define MAX_LOADSTRING 100

TCHAR				szTitle[MAX_LOADSTRING];				// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR				szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.

CGameFramework		gGameFramework;
CConnectServer		g_Connect_server;


BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//ksh
HINSTANCE g_hInst;

HWND hIp;
TCHAR tc_ip[200];	// DB���̵� ������ ������ �ޱ�
TCHAR ipTmp[200];	// DB���̵� �ޱ�


HWND hIp1;
TCHAR tc_ip1[20];
char c_ip1[20];

TCHAR tc_party[200];	// ��Ƽ���̵� ������ ������ �ޱ�

WCHAR in_name[200];
WCHAR jo_name[200];

// ����
SOCKET party_socket;
SOCKET login_socket;
WSABUF	send_wsabuf_ksh;
char 	send_buffer_ksh[1024];

SOCKET party;

#define DB_LOGIN 1
#define DB_LOGOUT 2
#define DB_INSERT 3

cs_packet_db db_packet; // �÷��̾� ���� ������ �ֱ�

LRESULT CALLBACK InputServerIpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

void sendLogInDB()
{
	wchar_t user_name[100];
	wcscpy_s(user_name, tc_ip);

	cs_packet_db *my_packet = reinterpret_cast<cs_packet_db *>(send_buffer_ksh);	// Ű�� ���������� ��Ŷ�� �����ϰ�
	my_packet->size = sizeof(cs_packet_db);
	send_wsabuf_ksh.len = sizeof(cs_packet_db);
	DWORD iobyte;

	my_packet->type = DB_LOGIN;	// Ÿ�Ը� �ٲ㼭 ������ �ȴ�.
	my_packet->pexp = 0;
	my_packet->plevel = 0;
	my_packet->xPos = 0;
	my_packet->yPos = 0;
	my_packet->pclass = 0;
	my_packet->pconnect = 0;
	my_packet->pmodel = 0;
	my_packet->pselect = 0;
	wcscpy_s(my_packet->db_name, tc_ip);
	if (!wcscmp(my_packet->db_name, L"")) {
		MessageBox(NULL,
			(LPCWSTR)L"ȸ���̸��� �������� �ʽ��ϴ�. �ٽ� �Է����ּ���.",
			(LPCWSTR)L"�α��� ����",
			MB_DEFBUTTON2
			);
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, InputServerIpDlgProc);
	}
	else {
		int ret = WSASend(login_socket, &send_wsabuf_ksh, 1, &iobyte, 0, NULL, NULL);	// �Ʊ� ���� ���Ͽ��ٰ� ������. 1=���۴� 1����, ������ũ��� my_packet������,	
		if (ret) {
			int error_code = WSAGetLastError();
			printf("Error while sending packet [%d]", error_code);
		}
		recv(login_socket, (char *)&db_packet, sizeof(db_packet), 0);
	}



	if (db_packet.type == 51)//���̵� ������ �ٽ� �ڽ��� ����
	{
		MessageBox(NULL,
			(LPCWSTR)L"ȸ���̸��� �������� �ʽ��ϴ�. �ٽ� �Է����ּ���.",
			(LPCWSTR)L"�α��� ����",
			MB_DEFBUTTON2
			);
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, InputServerIpDlgProc);
	}
	if (db_packet.type == 52)//���̵� ������ �ٽ� �ڽ��� ����
	{
		MessageBox(NULL,
			(LPCWSTR)L"ȸ���̸��� �������Դϴ�. �ٽ� �Է����ּ���.",
			(LPCWSTR)L"�α��� ����",
			MB_DEFBUTTON2
			);
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, InputServerIpDlgProc);
	}
	// db ���� �޾Ҵ�.

	int a = 0;
}

void SignUpDB()
{
	wchar_t user_name[100];
	wcscpy_s(user_name, tc_ip);

	cs_packet_db *my_packet = reinterpret_cast<cs_packet_db *>(send_buffer_ksh);	// Ű�� ���������� ��Ŷ�� �����ϰ�
	my_packet->size = sizeof(cs_packet_db);
	send_wsabuf_ksh.len = sizeof(cs_packet_db);
	DWORD iobyte;

	my_packet->type = DB_INSERT;	// Ÿ�Ը� �ٲ㼭 ������ �ȴ�.
	my_packet->pexp = 0;
	my_packet->plevel = 0;
	my_packet->xPos = 0;
	my_packet->yPos = 0;
	wcscpy_s(my_packet->db_name, tc_ip);

	if (!wcscmp(my_packet->db_name, L"")) {
		MessageBox(NULL,
			(LPCWSTR)L"ȸ���̸��� �������� �ʽ��ϴ�. �ٽ� �Է����ּ���.",
			(LPCWSTR)L"ȸ������ ����",
			MB_DEFBUTTON2
			);
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, InputServerIpDlgProc);
	}
	else {
		int ret = WSASend(login_socket, &send_wsabuf_ksh, 1, &iobyte, 0, NULL, NULL);	// �Ʊ� ���� ���Ͽ��ٰ� ������. 1=���۴� 1����, ������ũ��� my_packet������,	
		if (ret) {
			int error_code = WSAGetLastError();
			printf("Error while sending packet [%d]", error_code);
		}

		// ���⼭ ȸ�� ���� ����� �� ����� �޴µ�.
		recv(login_socket, (char *)&db_packet, sizeof(db_packet), 0);
	}
	if (db_packet.type == 50)//���̵� ������ �ٽ� �ڽ��� ����
	{
		MessageBox(NULL,
			(LPCWSTR)L"ȸ���̸��� �����մϴ�. �ٽ� �������ּ���.",
			(LPCWSTR)L"ȸ������ ����",
			MB_DEFBUTTON2
			);
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, InputServerIpDlgProc);
	}
}

void acceptDB() {
	login_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);	// ���ϸ����

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(8000);
	ServerAddr.sin_addr.s_addr = inet_addr("192.168.43.197");

	int Result = WSAConnect(login_socket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	int b = 0;
}

LRESULT CALLBACK InputServerIpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hBitmap;
	static int iMainWidth;
	static int iMainHeight;
	BITMAP bitmap;
	PAINTSTRUCT ps;
	HDC hdc;
	HDC dc;

	char text_file_ip1[20];
	TCHAR ipTmp1[20];

	strcpy(text_file_ip1, "10.30.2.33");
	MultiByteToWideChar(CP_ACP, 0, text_file_ip1, strlen(text_file_ip1) + 1, ipTmp1, strlen(text_file_ip1) + 1);

	////////////////
	switch (iMessage) {
	case WM_INITDIALOG:
		hIp = GetDlgItem(hDlg, IDC_ID);
		SetWindowText(hIp, ipTmp);
		hIp1 = GetDlgItem(hDlg, IDC_IP1);
		SetWindowText(hIp1, ipTmp1);

		hBitmap = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		GetObject(hBitmap, sizeof(BITMAP), &bitmap);
		iMainWidth = bitmap.bmWidth;
		iMainHeight = bitmap.bmHeight;
		SetWindowPos(hDlg, NULL, 0, 0, iMainWidth, iMainHeight, SWP_NOZORDER | SWP_NOMOVE);
		return TRUE;
		//break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		dc = CreateCompatibleDC(hdc);
		SelectObject(dc, hBitmap);
		BitBlt(hdc, 0, 0, iMainWidth, iMainHeight, dc, 0, 0, SRCCOPY);
		DeleteDC(dc);
		EndPaint(hDlg, &ps);
		return TRUE;

	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			acceptDB();
			GetWindowText(hIp, tc_ip, 20);
			GetWindowText(hIp1, tc_ip1, 20);
			EndDialog(hDlg, IDOK);
			sendLogInDB();
			break;

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			PostQuitMessage(0);
			break;

		case IDC_BUTTON5:
			acceptDB();
			GetWindowText(hIp, tc_ip, 20);
			GetWindowText(hIp1, tc_ip1, 20);
			EndDialog(hDlg, IDC_BUTTON5);
			SignUpDB();
			break;
		}
		break;

	case WM_DESTROY:
		DeleteObject(hBitmap);
		EndDialog(hDlg, TRUE);
		return TRUE;
	}

	return FALSE;
	//return (DefWindowProc(hDlg, iMessage, wParam, lParam));
}
//ksh

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_LABPROJECT14, szWindowClass, MAX_LOADSTRING);
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT14));

	//ksh
	WSADATA m_wsadata;
	WSAStartup(MAKEWORD(2, 2), &m_wsadata);

	//cleanupó��

	g_hInst = hInstance;

	send_wsabuf_ksh.buf = send_buffer_ksh;
	send_wsabuf_ksh.len = 1024;
	//ksh

    if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

    MSG msg = { 0 };
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
            gGameFramework.FrameAdvance(&g_Connect_server);
        }
    }
	g_Connect_server.DisConnectSerever();
    gGameFramework.OnDestroy();

    return((int)msg.wParam);
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABPROJECT14));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL/*MAKEINTRESOURCE(IDC_LABPROJECT14)*/;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    if (!RegisterClassEx(&wcex)) return(FALSE);

    RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
    bool b = AdjustWindowRect(&rc, dwStyle, FALSE);
    HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, 20, 20, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
    if (!hMainWnd) return(FALSE);

	g_Connect_server.ConnectServerInitialize(hMainWnd);
	gGameFramework.SetSocket(g_Connect_server.GetSocket());
	//ksh
	party_socket = g_Connect_server.GetSocket();
	//ksh

    if (!gGameFramework.OnCreate(hInstance, hMainWnd)) return(FALSE);

    ShowWindow(hMainWnd, nCmdShow);

    return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {	//kshdb
	case WM_CREATE:
		//DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, InputServerIpDlgProc);
		break;
		//ksh
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
        gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
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
			exit(-1); // Ŭ���̾�Ʈ ���� ����
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
