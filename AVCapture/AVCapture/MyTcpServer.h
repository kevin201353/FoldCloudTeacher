#pragma once

#include <winsock2.h>  
#include <stdio.h>  
#define PORT 22222
#define MSGSIZE 1024  
#pragma comment (lib, "Ws2_32.lib")

typedef struct  
{  
   WSAOVERLAPPED overlap;  
   WSABUF Buffer;  
   char szMessage[MSGSIZE];  
   DWORD NumberOfBytesRecvd;  
   DWORD Flags;  
}PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA; 

class MyTcpServer
{
public:
	MyTcpServer(void);
	~MyTcpServer(void);
public:
	BOOL WinSockInit();
	BOOL StartTcpServer();
	void Cleanup(int index);
	static DWORD WINAPI AcceptThread(LPVOID lpParam);
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
	static DWORD WINAPI AVCaptureThrd(LPVOID lpParam);
	static DWORD WINAPI AudioCaptureThrd(LPVOID lpParam);
	static DWORD WINAPI AVProcess(LPVOID lpParam);
	BOOL send_data(char* buf, int len);
	int  senddata(SOCKET socket, char* buf, int len);
	int  send_data2(SOCKET socket, char *buf, int len);
	void deal_video(SOCKET socket);
	void stat_catputVideo();
	void stat_catputAudio();
	static int get_random_by_pid();
	void start_av_process();
	void sendAudio(MyTcpServer *pServer);
	void disConnectAll();
private:
	SOCKET	m_sListen, m_sClient;
    SOCKADDR_IN	m_local, m_client;
	int  m_iaddrSize;
	WSAOVERLAPPED  m_sendOverlapped;
	char  m_szServerData[1024];
};

