#include "SERVER.h"

CSERVER g_server;
volatile bool g_ShutDown;

int main() {
	g_server.StartUpServer();
	g_ShutDown = false;
	g_server.SetShutDown(g_ShutDown);
	cout << "build up!" << endl;
	while (1) {
		Sleep(1000);
		bool shut_down;
		cout << "shut down?? : ";
		std::cin >> shut_down;
		g_ShutDown = shut_down;
		_asm mfence;
		if (true == g_ShutDown) {
			g_server.SetShutDown(g_ShutDown);
			break;
		}
	}
	cout << "clean up server process" << endl;
	g_server.CleanUpServer();

	// 제대로 돌아가는지 확인 하기 위해 멈춰둔다.
	system("pause");
}