#include "SimpleServer.h"
#include <ws2tcpip.h>
#include "MyTcpServer.h"


#define   SIMPLE_TCP_PORT  27015
#define   MAX_BUFF_SIZE    4096
fd_set    g_fdClientSock;  
int clientNum = 0;
char g_recvBuff[MAX_BUFF_SIZE];
char  m_localIP[BUF_MAX_SIZE];
extern BOOL  g_bAVCatpure;
extern MyTcpServer *g_myTcpServer;

void UnicodeToAnsi(TCHAR *source, char *dest)  
{      
    int nLen = WideCharToMultiByte(CP_ACP, 0, source, -1, NULL, 0, NULL, NULL);
    char *pAnsiStr = new char[nLen + 1];
    memset(pAnsiStr, 0, nLen+1);
    WideCharToMultiByte(CP_ACP, 0, source, -1, pAnsiStr, nLen, NULL, NULL);
	strcpy_s(dest, nLen, pAnsiStr);
	if (NULL != pAnsiStr)
	{
		delete pAnsiStr;
		pAnsiStr = NULL;
	}
}  


void AnsiToUnicode(char *source, TCHAR *dest)  
{   
    int nLen = MultiByteToWideChar(CP_ACP, 0, source, -1, NULL, 0); 
    wchar_t *pUnicodeStr = new wchar_t[nLen + 1];
    memset(pUnicodeStr, 0, (nLen + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, source, -1, pUnicodeStr, nLen);
	wcscpy_s(dest, nLen, pUnicodeStr);
	if (NULL != pUnicodeStr)
	{
		delete pUnicodeStr;
		pUnicodeStr = NULL;
	}
}  

SimpleServer::SimpleServer(void)
{
	WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) 
	{
        wprintf(L"WSAStartup failed with error: %ld\n", iResult);
    }
	memset(&m_msgComm, 0, sizeof(MSG_COMM));
	memset(m_localIP, 0, sizeof(m_localIP));
	TCHAR addr[BUF_MAX_SIZE] = {0};
	_gethostAddress(addr);
	UnicodeToAnsi(addr, m_localIP);
	printf("host addr :%s.\n", m_localIP);
}

SimpleServer::~SimpleServer(void)
{
	if (INVALID_SOCKET != m_ListenSocket)
		closesocket(m_ListenSocket);
    WSACleanup();  
}

int SimpleServer::_init()
{
    m_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_ListenSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(m_localIP);
	service.sin_port = htons(SIMPLE_TCP_PORT);

    if (bind(m_ListenSocket,
             (SOCKADDR *) & service, sizeof (service)) == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        closesocket(m_ListenSocket);
        WSACleanup();
        return -1;
    }
	if (listen(m_ListenSocket, 4) == SOCKET_ERROR) {
		wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
		closesocket(m_ListenSocket);
		WSACleanup();
		return -1;
	}
	DWORD  dwAccThread;
	DWORD  dwWorkThread;
	CreateThread(NULL, 0, AcceptThread, this, 0, &dwAccThread);
	CreateThread(NULL, 0, WorkerThread, this, 0, &dwWorkThread);
	return 0;
}

DWORD WINAPI SimpleServer::AcceptThread(LPVOID lpParam)
{
	SimpleServer *pServer = (SimpleServer *)lpParam;
	if (NULL == pServer)
		return 0;
	sockaddr_in clientAddr;  
    int nameLen = sizeof( clientAddr );  
	while( clientNum < FD_SETSIZE )//FD_SETSIZE==64  
    {  
        // 当有一个客户端进行连接时，主线程的accept会进行返回  
        SOCKET clientSock = accept( pServer->m_ListenSocket, (sockaddr*)&clientAddr, &nameLen );
        FD_SET(clientSock, &g_fdClientSock);
        clientNum++;
		Sleep(10);
    }
	return 0;
}

DWORD WINAPI SimpleServer::WorkerThread(LPVOID lpParam)
{
	SimpleServer *pServer = (SimpleServer *)lpParam;
	if (NULL == pServer)
		return 0;

	int nRet = 0;
	fd_set fdRead;
    FD_ZERO( &fdRead );
	while (1)
	{
		fdRead = g_fdClientSock;
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10;
        //检查fd_arrray数组里边是否有信号到来  
        nRet = select( 0, &fdRead, NULL, NULL, &tv );
		if (nRet != SOCKET_ERROR)
		{
			for ( int i = 0; i < g_fdClientSock.fd_count; i++ )
			{
				// 遍历出来哪些SOCKET有信号  
                if ( FD_ISSET(g_fdClientSock.fd_array[i],&fdRead)  )  
				{
					// 下面是数据的收发  
                    memset( g_recvBuff, 0, sizeof(char) * MAX_BUFF_SIZE );
                    nRet = recv( g_fdClientSock.fd_array[i], g_recvBuff, MAX_BUFF_SIZE, 0);
                    if ( nRet == SOCKET_ERROR )
                    {
                        closesocket( g_fdClientSock.fd_array[i]);
                        clientNum--;
                        FD_CLR( g_fdClientSock.fd_array[i], &g_fdClientSock );
                    }
                    else if ( nRet == 0 )
                    {
                        closesocket( g_fdClientSock.fd_array[i]);
                        clientNum--;
                        FD_CLR( g_fdClientSock.fd_array[i], &g_fdClientSock );
                    }
                    else
                    {
						MSG_COMM *pMsg = (MSG_COMM *)g_recvBuff;
						if (pMsg->type == 1)
						{
							//本机通信
							char szData[100] = {0};
							strcpy(szData, pMsg->data);
							printf("Recv data :%s.\n", szData);
							if (strcmp(szData,"start") == 0)
							{
								g_bAVCatpure = TRUE;
								printf("AV  flag  true .\n");
							}
							else if (strcmp(szData, "stop") == 0)
							{
								g_bAVCatpure = FALSE;
								printf("AV  flag  false .\n");
								if (NULL != g_myTcpServer)
								{
									g_myTcpServer->disConnectAll();
								}
							}
						}
                        //send(g_fdClientSock.fd_array[i], g_recvBuff, strlen(g_recvBuff), 0);
                    }
				}
			}
		}
		Sleep(10);
	}
	return 0;
}

BOOL SimpleServer::_gethostAddress(TCHAR *address)
{
	/*
	char name[255];
	if( gethostname ( name, sizeof(name)) == 0)
	{
		hostent* pHostent = gethostbyname(name);
		hostent& he = *pHostent;
		sockaddr_in sa;
		for (int nAdapter = 0; he.h_addr_list[nAdapter]; nAdapter++)
		{
			memcpy ( &sa.sin_addr.s_addr, he.h_addr_list[nAdapter],he.h_length);
			strcpy(address,inet_ntoa(*(struct in_addr *)he.h_addr_list[nAdapter]));
		}
	}
	return TRUE;
	*/

	HOSTENT *LocalAddress;
	char *Buff = NULL;
	TCHAR *wBuff = NULL;
	TCHAR* strReturn = L"";
	//创建新的缓冲区
	Buff = new char[ADDRESS_BUF_LEN];
	wBuff = new TCHAR[ADDRESS_BUF_LEN];

	//置空缓冲区
	memset(Buff, '\0', ADDRESS_BUF_LEN);
	memset(wBuff, TEXT('\0'), ADDRESS_BUF_LEN *sizeof(TCHAR));

	//得到本地计算机名
	if (gethostname(Buff, ADDRESS_BUF_LEN) == 0)
	{
		//转换成双字节字符串
		//mbstowcs_s(wBuff, Buff, 256);
		AnsiToUnicode(Buff, wBuff);
		//得到本地地址
		LocalAddress = gethostbyname(Buff);

		//置空buff
		memset(Buff, '\0', ADDRESS_BUF_LEN);
		//组合本地IP地址
		sprintf_s(Buff, ADDRESS_BUF_LEN, "%d.%d.%d.%d\0", LocalAddress->h_addr_list[0][0] & 0xFF,
		LocalAddress->h_addr_list[0][1] & 0x00FF, LocalAddress->h_addr_list[0][2] & 0x0000FF, LocalAddress->h_addr_list[0][3] & 0x000000FF);

		//char* Buff =inet_ntoa(*((in_addr*) (*LocalAddress->h_addr_list)));    //也可以用这个函数获得IP地址的字符串，具体参考inet_ntoa函数

		//置空wBuff
		memset(wBuff, TEXT('\0'), ADDRESS_BUF_LEN *sizeof(TCHAR));
		//转换成双字节字符串
		//mbstowcs(wBuff, Buff, 256);
		AnsiToUnicode(Buff, wBuff);
		//设置返回值
		wcscpy_s(address, ADDRESS_BUF_LEN, wBuff);
	}
	else
	{
		printf("获取IP地址失败\n");
	}
	//释放Buff缓冲区
	delete[] Buff;
	//释放wBuff缓冲区
	delete[] wBuff;
	
	return TRUE;
}