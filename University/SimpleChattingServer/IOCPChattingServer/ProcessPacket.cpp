#include "SERVER.h"

//////// Process Packet ///////////

bool CSERVER::register_handle(int type, PACKET_FP fp) {
	auto f = handler.find(type);
	auto e = handler.end();
	if (f == e) {
		handler.insert(DATAS::value_type(type, fp));
		return true;
	}
	else {
		cout << "Already Insert." << endl;
		return false;
	}
	return false;
}
void CSERVER::Initialize_handler() {
	register_handle(CLIENT_EVENT_CHAT, &CSERVER::RequestChattingMessage);

}
void CSERVER::RequestCreateRoom(int id, unsigned char* packe) {

}
void CSERVER::RequestChattingMessage(int id, unsigned char* packe) {
	cout << id << "player chatting message send" << endl;
}
void CSERVER::CheckEmptyRoom(int id, unsigned char* packe) {

}
void CSERVER::RecvPacket(int id) {
	DWORD flags = 0;
	auto client = client_list.find(id);
	SOCKET socket = client->second->GetSocket();
	int retval = WSARecv(socket, &client->second->overlap_ex.wsabuf, 1, NULL, &flags,
		&client->second->overlap_ex.origin_wsaoverlapped, NULL);
	if (0 != retval) {
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error) {
			cout << "Accept::WSARecv Err : " << error << endl;
		}
	}
}
void CSERVER::SendPakcet(int id, unsigned char* packet) {
	OVERLAP_EX *overlap = new OVERLAP_EX;
	memset(overlap, 0, sizeof(OVERLAP_EX));
	overlap->operation = OPERATION_SEND;
	overlap->wsabuf.buf = reinterpret_cast<char*>(overlap->socket_buf);
	overlap->wsabuf.len = packet[0];
	memcpy(overlap->socket_buf, packet, packet[0]);

	auto client = client_list.find(id);

	int retval = WSASend(client->second->GetSocket(), &overlap->wsabuf, 1, NULL, 0,
		&overlap->origin_wsaoverlapped, NULL);

	if (0 != retval) {
		int error = WSAGetLastError();
		if (WSA_IO_PENDING != error) {
			cout << "SendPacket ERR : " << id << "Player" << endl;
		}
	}
}
void CSERVER::ProcessPacket(int id, unsigned char* packet) {
	auto result = handler.find(packet[1]);
	if (result != handler.end()) {
		(this->*((*result).second))(id, packet);
	}
	else {
		cout << "Unknown Packet" << endl;
		// packet erro process
	}
}
