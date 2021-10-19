// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once
#pragma comment(lib,"ws2_32")

//#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
#include <time.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>


#include <string>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <Mmsystem.h>
#include <WinSock2.h>

#include <string>
#include <vector>

#include "..\..\Server\Server_Practice\protocol.h"

#define FRAME_BUFFER_WIDTH			800
#define FRAME_BUFFER_HEIGHT			800

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

using namespace D2D1;
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#define SERVERIP "127.0.0.1"

const auto BUFF_SIZE = 1024;
const auto WM_SOCKET = WM_USER + 1;

