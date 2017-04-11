#include "MyTcpServer.h"


int g_screenWith = 0;
int g_screenHeight = 0;
extern MyTcpServer *g_myTcpServer;
int main(void)
{
	g_myTcpServer = NULL;
	g_screenWith = GetSystemMetrics(SM_CXSCREEN);
	g_screenHeight = GetSystemMetrics(SM_CYSCREEN);
	MyTcpServer Tcpserver;
	g_myTcpServer = &Tcpserver;
	Tcpserver.WinSockInit();
	Tcpserver.StartTcpServer();
	int ntmp = getchar();
	printf("main end : %d", ntmp);
	return 0;
}