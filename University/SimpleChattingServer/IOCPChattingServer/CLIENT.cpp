#include "CLIENT.h"


void CCLIENT::InitializeOverlapped() {
	memset(&overlap_ex.origin_wsaoverlapped, 0, sizeof(WSAOVERLAPPED));
	memset(&overlap_ex.wsabuf.buf, 0, MAX_BUFF_SIZE);
	memset(&overlap_ex.socket_buf, 0, MAX_BUFF_SIZE);
	overlap_ex.wsabuf.buf = reinterpret_cast<char*>(overlap_ex.socket_buf);
	overlap_ex.wsabuf.len = MAX_BUFF_SIZE;
	overlap_ex.operation = OPERATION_RECV;
	is_connected = false;
	previous_data = 0;
}
