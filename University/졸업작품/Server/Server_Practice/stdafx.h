#pragma once

#pragma comment(lib,"ws2_32")


#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <WinSock2.h>
#include <WinSock.h>

#include <vector>
#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>

#include <set>
#include <chrono>
#include <concurrent_queue.h>
#include <queue>

#include "protocol.h"

using namespace std;
using namespace Concurrency;

const auto NUM_THREADS = 6;
