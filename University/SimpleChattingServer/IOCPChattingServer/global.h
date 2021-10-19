#pragma once

#pragma comment(lib,"ws2_32")

#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <windows.h>
#include <thread>
#include <map>
#include <atomic>

#include "protocol.h"

using std::cout;
using std::endl;

const auto THREADS_NUM = 5;

