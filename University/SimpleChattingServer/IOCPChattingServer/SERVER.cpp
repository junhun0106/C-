#include "SERVER.h"

void CSERVER::StartUpServer() {
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	CSERVER::Initialize_handler();
	CSERVER::InitializeThreads();
}

void CSERVER::CleanUpServer() {
	cout << "join thread and delete " << endl;
	CSERVER::DestroyThreads();
	cout << "network cleaup" << endl;
	closesocket(listen_socket);
	WSACleanup();
}
