#include "SERVER.h"

/////// Thread /////
void CSERVER::WorkerThread() {
	DWORD iosize, key;
	OVERLAP_EX *overlap;
	int result;

	while (true) {
		result = GetQueuedCompletionStatus(hIocp, &iosize, &key, reinterpret_cast<LPOVERLAPPED*>(&overlap), 10000);
		if (overlap == NULL) {
			if (shut_down) break;
			continue;
		}
		if (0 == iosize) {
			auto client = client_list.find(key)->second;
			closesocket(client->GetSocket());
			client->SetConnected(false);
			delete client;
			caslock.lock();
			client_list.erase(key);
			caslock.unlock();
			cout << key << "client Exit" << endl;
			client_count--;
			continue;
		}

		auto client = client_list.find(key)->second;

		if (OPERATION_RECV == overlap->operation) {
			unsigned char *buf_ptr = overlap->socket_buf;
			int remained = iosize;
			int required = 0;
			while (0 < remained) {
				if (0 == client->overlap_ex.previous_packet_size) {
					client->SetPredPacketSize(buf_ptr[0]);
				}
				required = client->overlap_ex.previous_packet_size - client->GetPredDataSize();
				if (remained >= required) {
					client->SetStorePacket(client->GetPredDataSize(), buf_ptr, required);
					CSERVER::ProcessPacket(key, buf_ptr);
					remained -= required;
					buf_ptr += required;
					client->SetPredPacketSize(0);
					client->SetPredDataSize(0);
				}
				else {
					client->SetStorePacket(client->GetPredDataSize(), buf_ptr, remained);
					buf_ptr += remained;
					remained = 0;
				}
			}
			CSERVER::RecvPacket(key);
		}
		else if (OPERATION_SEND == overlap->operation) {
			delete overlap;
		}
		else {
			cout << "Unknown Operation" << endl;
			continue;
		}
	}
	cout << "Worker Thread Exit" << endl;
}
void CSERVER::AcceptThread() {
	SOCKADDR_IN listen_addr;
	SOCKET accept_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	char buf[50];
	setsockopt(accept_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&buf, sizeof(buf));

	ZeroMemory(&listen_addr, sizeof(SOCKADDR_IN));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(SERVERPORT);
	listen_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ZeroMemory(&listen_addr.sin_zero, 8);
	::bind(accept_socket, reinterpret_cast<SOCKADDR*>(&listen_addr), sizeof(listen_addr));
	::listen(accept_socket, MAX_USER);

	// accept time out process
	fd_set fd;
	timeval time_val;
	unsigned long on = 1;
	::ioctlsocket(accept_socket, FIONBIO, &on); // non blocking
	while (1) {
		FD_ZERO(&fd);
		FD_SET(accept_socket, &fd);
		time_val.tv_sec = 5;
		time_val.tv_usec = 0;
		if (0 >= select(0, &fd, NULL, NULL, &time_val)) {
			if (false == shut_down) continue;
			break;
		}
		else {
			// accept
			SOCKADDR_IN client_addr;
			int client_size = sizeof(client_addr);
			SOCKET client_sock = WSAAccept(accept_socket, reinterpret_cast<SOCKADDR*>(&client_addr), &client_size, NULL, NULL);
			if (INVALID_SOCKET == client_sock) {
				closesocket(client_sock);
				continue;
			}
			if (client_list.size() > MAX_USER) {
				closesocket(client_sock);
				continue;
			}
			int new_id = 0;

			auto begin = client_list.begin();
			auto end = client_list.end();
			while (begin != end) {
				if (begin->first == new_id) { ++begin; ++new_id; }
				else
					break;

			}
			CCLIENT* client = new CCLIENT();
			client->InitializeOverlapped();
			client->SetClient(client_sock, new_id);
			client->SetConnected(true);
			caslock.lock();
			client_list.insert(CLIENT_INFO::value_type(new_id, client));
			caslock.unlock();
			client_count++;

			// test
			//begin = client_list.begin();
			//end = client_list.end();

			//cout << client_list.size() << "players in server" << endl;
			//while (begin != end) {
			//	cout << begin->first << "player in server" << endl;
			//	begin++;
			//}

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), hIocp, new_id, 0);

			CSERVER::RecvPacket(new_id);
		}
	}
	cout << "close accept thread" << endl;
	closesocket(accept_socket);
}
void CSERVER::InitializeThreads() {
	threadspool.clear();
	for (auto i = 0; i < THREADS_NUM; ++i)
		threadspool.push_back(new std::thread(&CSERVER::WorkerThread, this));
	threadspool.push_back(new std::thread(&CSERVER::AcceptThread, this));
}
void CSERVER::DestroyThreads() {
	for (auto th : threadspool) th->join();
	for (auto th : threadspool) delete th;
}