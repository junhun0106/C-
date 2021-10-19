#pragma once

#include "global.h"

struct OVERLAP_EX {
	WSAOVERLAPPED origin_wsaoverlapped;
	int operation;
	WSABUF wsabuf;
	unsigned char socket_buf[MAX_BUFF_SIZE];
	int previous_packet_size;
};
class CCLIENT
{
private:
	int previous_data;
	SOCKET client_socket;
	bool is_connected;
	int client_id;
public:
	OVERLAP_EX overlap_ex;
	unsigned char store_packet[MAX_BUFF_SIZE];
public:
	CCLIENT() = default;
	~CCLIENT() = default;

	void InitializeOverlapped();
	void SetClient(SOCKET sock, int id) { client_socket = sock; client_id = id; }

	bool GetConnected() { return is_connected; }
	void SetConnected(bool b) { is_connected = b; }

	SOCKET GetSocket() { return client_socket; }
	int GetID() { return client_id; }

	int GetPredDataSize() { return previous_data; }
	void SetPredDataSize(int size) { previous_data = size; }
	void SetPredPacketSize(int size) { overlap_ex.previous_packet_size = size; }
	void SetStorePacket(int offset, unsigned char* src, int size) {
		memcpy(store_packet + offset, src, size);
	}
};

