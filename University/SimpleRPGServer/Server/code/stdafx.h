#pragma once

#pragma comment(lib,"ws2_32")


#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <math.h>
#include <WinSock2.h>
#include <WinSock.h>

#include <sqlext.h>

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
 
#include <set>
#include <chrono>
#include <queue>
#include <map>
#include <list>
#include <algorithm>

#include "protocol.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

using namespace std;

const auto NUM_THREADS = 6;