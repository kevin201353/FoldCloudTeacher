#pragma once
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>

//socket  select Ä£ÐÍ
#pragma comment(lib, "Ws2_32.lib")
#define   MSG_DATA_LEN       1024
#define   ADDRESS_BUF_LEN    256
#define   BUF_MAX_SIZE       520

struct MSG_COMM {
	short type;
	int   len;
	char  data[MSG_DATA_LEN];
};

class SimpleServer
{
public:
	SimpleServer(void);
	~SimpleServer(void);
public:
	int _init();
	BOOL _gethostAddress(TCHAR *address);
	static DWORD WINAPI AcceptThread(LPVOID lpParam);
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
private:
	SOCKET m_ListenSocket;
	MSG_COMM   m_msgComm;
};

