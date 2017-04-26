#include "MyTcpServer.h"
#include "SimpleServer.h"


int g_screenWith = 0;
int g_screenHeight = 0;
extern MyTcpServer *g_myTcpServer;
HANDLE  m_hMutex;
int main(void)
{
	m_hMutex = CreateMutex(NULL, TRUE, TEXT("AVCapture"));
	if (m_hMutex != NULL && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		ReleaseMutex(m_hMutex);
		MessageBox(NULL, TEXT("该程序已经启动"), TEXT("错误"), MB_OK);
		return FALSE;
	}
	g_myTcpServer = NULL;
	g_screenWith = GetSystemMetrics(SM_CXSCREEN);
	g_screenHeight = GetSystemMetrics(SM_CYSCREEN);
	MyTcpServer Tcpserver;
	g_myTcpServer = &Tcpserver;
	Tcpserver.WinSockInit();
	Tcpserver.StartTcpServer();
	SimpleServer  simpleServer;
	simpleServer._init();
	int ntmp = getchar();
	printf("main end : %d", ntmp);
	if (m_hMutex != NULL)
	{
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
	}
	return 0;
}