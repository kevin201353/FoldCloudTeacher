#include "MyTcpServer.h"
#include <Ws2tcpip.h>
#include "global.h"
#include "ScreenCapture.h"
#include <Windows.h>
#include "audio.h"
#include "myCriticalSection.h"
#include "MyRingBuffer.h"


int g_iTotalConn = 0;
SOCKET g_CliSocketArr[MAXIMUM_WAIT_OBJECTS];
WSAEVENT g_CliEventArr[MAXIMUM_WAIT_OBJECTS];
LPPER_IO_OPERATION_DATA g_pPerIODataArr[MAXIMUM_WAIT_OBJECTS];
char g_szDataBuf[MAX_DATA_BUF] = { 0 };
char g_szDataBuf_A[MAX_DATA_BUF_A] = { 0 };
char g_szDataBuf_A2[102400] = { 0 };
char g_szDataBuf_V[MAX_DATA_BUF] = { 0 };

HANDLE  g_hEvent = NULL;
unsigned int  g_frank_xor = 0;
extern int g_screenWith;
extern int g_screenHeight;
MyTcpServer *g_myTcpServer = NULL;
myCriticalSection  g_criticalSection;
MyRingBuffer   g_myRingBuffer_Au;
struct cyclic_buf  g_cb;
struct cyclic_buf  g_cbAudio;

MyRingBuffer2  g_myRingRuffer2;

BOOL  g_bstopThrd;
BOOL  g_bAVCatpure = TRUE;  
MyTcpServer::MyTcpServer(void):
m_iaddrSize(0)
{
	memset(m_szServerData, 0, sizeof(char) * 1024);
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_frank_xor = MyTcpServer::get_random_by_pid();
	cyclic_buf_init(&g_cb, 32, sizeof(Video_Package));
	dump_cyclic_buf(&g_cb, "after init");
	//cyclic_buf_init(&g_cbAudio, 32, sizeof(Audio_Package));
	g_bstopThrd = FALSE;
	g_bAVCatpure = TRUE;
	for (int i = 0; i < MAXIMUM_WAIT_OBJECTS; i++)
	{
		g_CliSocketArr[i] = INVALID_SOCKET;
		g_CliEventArr[i] = NULL;
		g_pPerIODataArr[i] = NULL;
	}
}


MyTcpServer::~MyTcpServer(void)
{
	g_bstopThrd = TRUE;
	if (NULL != g_hEvent)
	{
		SetEvent(g_hEvent);
		CloseHandle(g_hEvent);
		g_hEvent = NULL;
	}
	for (int i = 0; i < MAXIMUM_WAIT_OBJECTS; i++)
	{
		Cleanup(i);
	}
	cyclic_buf_free(&g_cb);
	g_bAVCatpure = FALSE;
	//cyclic_buf_free(&g_cbAudio);
}

BOOL MyTcpServer::WinSockInit()
{
	WSADATA data = {0};
    if(WSAStartup(MAKEWORD(2, 2), &data))
        return FALSE;
    if ( LOBYTE(data.wVersion) !=2 || HIBYTE(data.wVersion) != 2 ){
        WSACleanup();
        return FALSE;
    }
    return TRUE;
}

void MyTcpServer::Cleanup(int index)
{
	if (INVALID_SOCKET != g_CliSocketArr[index])
	{
		closesocket(g_CliSocketArr[index]);
		g_CliSocketArr[index] = INVALID_SOCKET;
	}
	if (NULL != g_CliEventArr[index])
	{
		WSACloseEvent(g_CliEventArr[index]);
		g_CliEventArr[index] = NULL;
	}
	if (NULL != g_pPerIODataArr[index])
	{
		HeapFree(GetProcessHeap(), 0, g_pPerIODataArr[index]);
	}
    if (index < g_iTotalConn - 1)
    {    
        g_CliSocketArr[index] = g_CliSocketArr[g_iTotalConn - 1];
        g_CliEventArr[index] = g_CliEventArr[g_iTotalConn - 1];
        g_pPerIODataArr[index] = g_pPerIODataArr[g_iTotalConn - 1]; 
    }
    g_pPerIODataArr[--g_iTotalConn] = NULL;
}

BOOL MyTcpServer::StartTcpServer()
{
    DWORD dwThreadId;
	DWORD dwAccThradId;
    m_iaddrSize = sizeof(SOCKADDR_IN);
    // 初始化环境  
    WinSockInit();
	SecureZeroMemory((PVOID)&m_sendOverlapped, sizeof(WSAOVERLAPPED));
	m_sendOverlapped.hEvent = WSACreateEvent();
    // 创建监听socket  
    m_sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // 绑定  
    m_local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    m_local.sin_family = AF_INET;
    m_local.sin_port = htons(PORT);
    bind(m_sListen, (struct sockaddr *)&m_local, sizeof(SOCKADDR_IN));
    // 监听  
    listen(m_sListen, 3);
	//创建连接线程
	CreateThread(NULL, 0, AcceptThread, this, 0, &dwAccThradId);
    // 创建工作者线程  
    CreateThread(NULL, 0, WorkerThread, this, 0, &dwThreadId);
	//开始抓捕桌面线程
	stat_catputVideo();
	//stat_catputAudio();
	start_av_process();
	return TRUE;
}


DWORD WINAPI MyTcpServer::AcceptThread(LPVOID lpParam)
{
	MyTcpServer *pserver = (MyTcpServer *)lpParam;
	if (NULL == pserver)
		return 0;
	int rc = 0;
	int err = 0;
	while (TRUE)
    {  
        // 接受连接  
        pserver->m_sClient = accept(pserver->m_sListen, (struct sockaddr *)&pserver->m_client, &pserver->m_iaddrSize);
		char IPdotdec[20];
		inet_ntop(AF_INET, &pserver->m_client.sin_addr, IPdotdec, 16);
		printf("Accepted client:%s:%d\n", IPdotdec, ntohs(pserver->m_client.sin_port));
        g_CliSocketArr[g_iTotalConn] = pserver->m_sClient;
        // 分配一个单io操作数据结构  
        g_pPerIODataArr[g_iTotalConn] = (LPPER_IO_OPERATION_DATA)HeapAlloc(
																	GetProcessHeap(),
																	HEAP_ZERO_MEMORY,
																	sizeof(PER_IO_OPERATION_DATA));
		if (NULL == g_pPerIODataArr[g_iTotalConn])
		{
			printf("AcceptThread:  heapAlloc failed.\n");
			break;
		}
        //初始化单io结构  
        g_pPerIODataArr[g_iTotalConn]->Buffer.len = MSGSIZE;
        g_pPerIODataArr[g_iTotalConn]->Buffer.buf = g_pPerIODataArr[g_iTotalConn]->szMessage;
        g_CliEventArr[g_iTotalConn] = g_pPerIODataArr[g_iTotalConn]->overlap.hEvent = WSACreateEvent();
		g_pPerIODataArr[g_iTotalConn]->Flags = 0;
		g_pPerIODataArr[g_iTotalConn]->NumberOfBytesRecvd = 0;
        // 开始一个异步操作  
        rc = WSARecv(
            g_CliSocketArr[g_iTotalConn],
            &g_pPerIODataArr[g_iTotalConn]->Buffer,
            1,
            &g_pPerIODataArr[g_iTotalConn]->NumberOfBytesRecvd,
            &g_pPerIODataArr[g_iTotalConn]->Flags, &g_pPerIODataArr[g_iTotalConn]->overlap,
            NULL);
		if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
			wprintf(L"WSARecv failed with error: %d\n", err);
			break;
		}
        g_iTotalConn++;
		Sleep(1);
    }
	printf("AcceptThread end.\n");
    closesocket(pserver->m_sListen);
    WSACleanup();  
	return 0;
}

DWORD WINAPI MyTcpServer::WorkerThread(LPVOID lpParam)
{
	MyTcpServer *pserver = (MyTcpServer *)lpParam;
	if (NULL == pserver)
		return 0;
	int ret, index;
    DWORD cbTransferred;
	int rc = 0;
	int err = 0;
    while (TRUE)
    {
        //判断是否有信号
        ret = WSAWaitForMultipleEvents(g_iTotalConn, g_CliEventArr, FALSE, WSA_INFINITE, FALSE);
		if (ret == WSA_WAIT_FAILED || ret == WSA_WAIT_TIMEOUT)
		{
			printf("wait socket event failed or timeout.\n");
			Sleep(100);
			continue;
		}
        index = ret - WSA_WAIT_EVENT_0;
		printf("recv thread event index : %d.\n", index);
        //手动设置为无信号  
        WSAResetEvent(g_CliEventArr[index]);
        //判断该重叠调用到底是成功，还是失败  
        BOOL bRet = WSAGetOverlappedResult(
			g_CliSocketArr[index],
			&g_pPerIODataArr[index]->overlap,
			&cbTransferred,
			TRUE,
			&g_pPerIODataArr[index]->Flags);
		if (!bRet)
		{
			printf("recv data WSAGetOverlappedResult return = %d.\n", WSAGetLastError());
			int nTmp = 0;
		}
        //若调用失败  
		if (cbTransferred == 0)
		{
			printf("recv data WSAGetOverlappedResult cbTransferred == 0 return = %d.\n", WSAGetLastError());
			pserver->Cleanup(index);//关闭客户端连接
		}
        else  
        {
            //将数据保存到szMessage里边           
            g_pPerIODataArr[index]->szMessage[cbTransferred] = '\0';
            //这里直接就转发回去了  
            //send(g_CliSocketArr[index], g_pPerIODataArr[index]->szMessage,cbTransferred, 0);
			strcpy_s(pserver->m_szServerData, (char *)g_pPerIODataArr[index]->szMessage);
			printf("recv client data : %s\n", pserver->m_szServerData);
			//do here , send video data
			//pserver->deal_video(g_CliSocketArr[index]);
			SetEvent(g_hEvent);
            // 进行另一个异步操作
			g_pPerIODataArr[index]->Flags = 0;
			g_pPerIODataArr[index]->NumberOfBytesRecvd = 0;
            rc = WSARecv(g_CliSocketArr[index],
				&g_pPerIODataArr[index]->Buffer,
				1,
				&g_pPerIODataArr[index]->NumberOfBytesRecvd,
				&g_pPerIODataArr[index]->Flags,
				&g_pPerIODataArr[index]->overlap,
				NULL);
			if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
				wprintf(L"WSARecv failed with error: %d\n", err);
				break;
			}
        }
		Sleep(1);
    }
	return 0;
}


//广播所有客户端
BOOL  MyTcpServer::send_data(char* buf, int len)
{
	if (NULL == m_sendOverlapped.hEvent)
	{
		printf("WSACreateEvent failed with error : %d\n", WSAGetLastError());
		return FALSE;
	}
	int rc = 0;
	for (int i=0; i<g_iTotalConn; i++)
	{
		if (NULL != g_CliSocketArr[i])
		{
			//senddata(g_CliSocketArr[i], buf, len);
			int nRet = send_data2(g_CliSocketArr[i], buf, len);
			if (nRet == -1)
			{
				Cleanup(i);
			}
		}
	}
	return TRUE;
}

int MyTcpServer::senddata(SOCKET socket, char* buf, int len)
{
	int l = 0;
	int rc = 0;
	char *d = NULL;
	d = buf;
	l = len;
	DWORD sendbytes = 0;
	DWORD  flags = 0;
	int err = 0;
	WSABUF  DataBuf;
	DataBuf.len = len;
	DataBuf.buf = buf;
	while (l) {
		rc = WSASend(socket, &DataBuf, 1, &sendbytes, 0, &m_sendOverlapped, NULL);
		if (SOCKET_ERROR == rc &&
			(WSA_IO_PENDING != (err = WSAGetLastError()))) 
		{
			printf("WSASend failed with error: %d\n", err);
			break;
		}
		rc = WSAWaitForMultipleEvents(1, &m_sendOverlapped.hEvent, TRUE, INFINITE, TRUE);
		if (WSA_WAIT_FAILED == rc)
		{
			printf("WSAWaitForMultipleEvents failed with error : %d\n", WSAGetLastError());
			break;
		}
		rc = WSAGetOverlappedResult(socket, &m_sendOverlapped, &sendbytes, FALSE, &flags);
		if (FALSE == rc)
		{
			printf("WSASend failed with error : %d\n", WSAGetLastError());
		}
		printf("send %d bytes\n", sendbytes);
		l -= sendbytes;
		d += sendbytes;
		WSAResetEvent(m_sendOverlapped.hEvent);
		Sleep(1);
	}
	return 0;
}

int MyTcpServer::send_data2(SOCKET socket, char *buf, int len)
{
	int l = 0;
	int ret = 0;
	char *d = NULL;
	d = buf;
	l = len;
	int nRet = 0;
	while (l) {
		if (g_bstopThrd)
			break;
		if (g_bAVCatpure == FALSE)
		{
			nRet = -1;
			break;
		}
		ret = send(socket, buf, len, 0);
		if (ret <= 0) {
			printf("send data failed, error = %d.\n", WSAGetLastError());
			nRet = -1;
			break;
		}
		l -= ret;
		d += ret;
		printf("send data bytes: %d\n", ret);
		Sleep(1);
	}
	return nRet;
}


void MyTcpServer::deal_video(SOCKET socket)
{
	char data[1024] = { 0 };
	strcpy_s(data, "222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222");
	media_header  mediaHeader;
	mediaHeader.magic[0] = 'B';
	mediaHeader.magic[1] = 'E';
	mediaHeader.magic[2] = 'A';
	mediaHeader.magic[3] = 'F';
	mediaHeader.type = 1;
	mediaHeader.width = 1920;
	mediaHeader.height = 1080;
	mediaHeader.serial = 1;
	mediaHeader.cursor_x = 20;
	mediaHeader.cursor_y = 30;
	mediaHeader.time_stamp = 0;
	mediaHeader.size = strlen(data);
	mediaHeader.checksum = 7;
	send_data((char*)&mediaHeader, sizeof(media_header));
	send_data(data, mediaHeader.size);
}

DWORD WINAPI MyTcpServer::AVCaptureThrd(LPVOID lpParam)
{
	MyTcpServer *pserver = (MyTcpServer *)lpParam;
	if (NULL == pserver)
		return 0;
	captureScreen   captureScreen;
	captureScreen.Init();
	media_header  mediaHeader;
	mediaHeader.magic[0] = 'B';
	mediaHeader.magic[1] = 'E';
	mediaHeader.magic[2] = 'A';
	mediaHeader.magic[3] = 'F';
	int count_p = 0;
	int  nloop = 0;
	int nSize = 0;
	bool  bflag = true;
	bool  bfirst = true;
	DWORD  dwStart = 0;
	DWORD  dwEnd = 0;
	while (1)
	{
		//printf("AVCaptureThrd  enter, wait event .\n");
		DWORD dReturn = WaitForSingleObject(g_hEvent, INFINITE);
		//printf("AVCaptureThrd  enter, wait event  00000 .\n");
		if (WAIT_OBJECT_0 == dReturn)
		{
			if (g_bstopThrd)
			{
				break;
			}
			if (g_bAVCatpure == FALSE)
			{
				bfirst = true;
				count_p = 0;
				Sleep(1);
				continue;
			}
			//printf("AVCaptureThrd  enter, wait event  1111 .\n");
			memset(g_szDataBuf, 0, sizeof(char) * MAX_DATA_BUF);
			dwStart = GetTickCount();
			if (count_p <= 3 && bfirst)
			{
				bflag = true;
				bfirst = false;
			}
			else if (count_p > 3)
			{
				DWORD dwInter = dwEnd - dwStart;
				if (nloop == 5 || (dwInter >= (3000 - 1) && dwInter <= (3000 + 1)))
				{
					bflag = true;
					nloop = 0;
				}
				else
					bflag = false;

				nloop++;
			}
			if (count_p >= 65535)
			{
				bfirst = true;
				count_p = 0;
			}
			//printf("capture video data start bflag : %d.\n", bflag);
			nSize = captureScreen.GetH264FrameData(g_szDataBuf, MAX_DATA_BUF, bflag);
			if (bflag)
				mediaHeader.type = 1;
			else
				mediaHeader.type = 2;
			printf("capture video data len : %d\n", nSize);
			mediaHeader.width = g_screenWith;
			mediaHeader.height = g_screenHeight;
			mediaHeader.cursor_x = 20;
			mediaHeader.cursor_y = 30;
			mediaHeader.time_stamp = 0;
			mediaHeader.serial = count_p;
			mediaHeader.size = nSize;
			Video_Package  vipck;
			vipck.len = 0;
			vipck.header = mediaHeader;
			memcpy(vipck.pata, g_szDataBuf, mediaHeader.size);
			vipck.len = mediaHeader.size;
			int len = sizeof(Video_Package);
			cyclic_buf_produce_data(&g_cb, &vipck, len);
			//dump_cyclic_buf(&g_cb, "after produce");
			count_p++;
			dwEnd = GetTickCount();
			/*
			FILE *stream;
			fopen_s(&stream, "D:\\zhaosenhua\\fwrite2", "a+");
			fwrite(g_szDataBuf, 1, mediaHeader.size, stream);
			fclose(stream);
			*/
		}
		Sleep(1);
	}

	//capture end
	//free
	printf("capture video end ..........\n");
	printf("free captureScreen resource ..........\n");
	captureScreen.Destory();
	return 0;
}

void MyTcpServer::stat_catputVideo()
{
	memset(g_szDataBuf, 0, sizeof(char) * MAX_DATA_BUF);
	DWORD dwAccThradId;
	CreateThread(NULL, 0, AVCaptureThrd, this, 0, &dwAccThradId);
}

int  MyTcpServer::get_random_by_pid()
{
	/*
	DWORD dwID = GetCurrentProcessId();
	srand((unsigned int)dwID);
	return rand();
	*/
	return 100000;
}

void MyTcpServer::stat_catputAudio()
{
	DWORD dwAccThradId;
	CreateThread(NULL, 0, AudioCaptureThrd, this, 0, &dwAccThradId);
}


DWORD WINAPI MyTcpServer::AudioCaptureThrd(LPVOID lpParam)
{
	MyTcpServer *pserver = (MyTcpServer *)lpParam;
	if (NULL == pserver)
		return 0;
	MyAudioSink  audioSink;
	audioSink.RecordAudioStream();
	return 0;
}

void call_msg_back(MsgCallBackFun fun, struct ReportMsg msg)
{
	fun(msg);
}

void msg_respose(struct ReportMsg msg)
{
	switch (msg.action)
	{
		case USER_MSG_AUDIO_START:
		{
			MyTcpServer *pTcpServer = g_myTcpServer;
			if (NULL != pTcpServer)
			{
				//header
				if (strlen(g_szDataBuf_A) > 0)
				{
					media_header  mediaHeader;
					mediaHeader.magic[0] = 'B';
					mediaHeader.magic[1] = 'E';
					mediaHeader.magic[2] = 'A';
					mediaHeader.magic[3] = 'F';
					mediaHeader.type = 3;
					mediaHeader.width = g_screenWith;
					mediaHeader.height = g_screenHeight;
					mediaHeader.cursor_x = 20;
					mediaHeader.cursor_y = 30;
					mediaHeader.time_stamp = 0;
					mediaHeader.serial = 0;
					mediaHeader.size = msg.len;
					mediaHeader.checksum = 0;
					if (mediaHeader.size % 4)
					{
						mediaHeader.size = ((mediaHeader.size + 4) / 4) * 4;
					}
					unsigned int *pata = (unsigned int*)g_szDataBuf_A;
					for (int i = 0; i < (int)(mediaHeader.size / 4); i++)
					{
						pata[i] ^= g_frank_xor;
						mediaHeader.checksum += pata[i];
					}
					//g_criticalSection.enter();
					//pTcpServer->send_data((char*)&mediaHeader, sizeof(media_header));
					//do send audio data
					printf("get audio data len: %d\n", msg.len);
					//pTcpServer->send_data(g_szDataBuf_A, msg.len);
					//g_criticalSection.leave();
					//test
					/*
					FILE *stream;
					fopen_s(&stream, "D:\\zhaosenhua\\fwrite2", "a+");
					fwrite(g_szDataBuf_A, 1, msg.len, stream);
					fclose(stream);
					*/
					//test end
				}
			}
		}
		break;
		default:break;
	}
}

DWORD WINAPI MyTcpServer::AVProcess(LPVOID lpParam)
{
	MyTcpServer *pTcpServer = (MyTcpServer *)lpParam;
	if (NULL == pTcpServer)
		return 0;
	while (1)
	{
		if (g_bstopThrd)
		{
			break;
		}
		Video_Package *vpk = NULL;
		vpk = (Video_Package *)cyclic_buf_consume_get(&g_cb);
		if (NULL != vpk && g_bAVCatpure == TRUE)
		{
			media_header  mediaHeader;
			mediaHeader = vpk->header;
			mediaHeader.size = vpk->len;
			mediaHeader.checksum = 0;
			if (mediaHeader.size % 4)
			{
				mediaHeader.size = ((mediaHeader.size + 4) / 4) * 4;
			}
			memcpy(g_szDataBuf_V, vpk->pata, vpk->len);
			unsigned int *pata = (unsigned int*)g_szDataBuf_V;
			for (int i = 0; i < (int)(mediaHeader.size / 4); i++)
			{
				pata[i] ^= g_frank_xor;
				mediaHeader.checksum += pata[i];
			}
			pTcpServer->send_data((char*)&mediaHeader, sizeof(media_header));
		    pTcpServer->send_data(g_szDataBuf_V, mediaHeader.size);
			cyclic_buf_consume(&g_cb);
		}
		//pTcpServer->sendAudio(pTcpServer);
		Sleep(1);
	}
	return 0;
}

void MyTcpServer::start_av_process()
{
	DWORD dwAVThradId;
	CreateThread(NULL, 0, AVProcess, this, 0, &dwAVThradId);
}

int g_nCount = 0;
void MyTcpServer::sendAudio(MyTcpServer *pServer)
{
	if (NULL != pServer)
	{
		Audio_Package *aupk = NULL;
		aupk = (Audio_Package *)cyclic_buf_consume_get(&g_cbAudio);
		if (NULL != aupk)
		{
			memcpy(g_szDataBuf_A2, aupk->pdata, aupk->len);
			media_header  mediaHeader;
			mediaHeader.magic[0] = 'B';
			mediaHeader.magic[1] = 'E';
			mediaHeader.magic[2] = 'A';
			mediaHeader.magic[3] = 'F';
			mediaHeader.type = 3;
			mediaHeader.width = g_screenWith;
			mediaHeader.height = g_screenHeight;
			mediaHeader.cursor_x = 20;
			mediaHeader.cursor_y = 30;
			mediaHeader.time_stamp = 0;
			mediaHeader.serial = 0;
			mediaHeader.size = aupk->len;
			mediaHeader.checksum = 0;
			if (mediaHeader.size % 4)
			{
				mediaHeader.size = ((mediaHeader.size + 4) / 4) * 4;
			}
			unsigned int *pata = (unsigned int*)g_szDataBuf_A2;
			for (int i = 0; i < (int)(mediaHeader.size / 4); i++)
			{
				pata[i] ^= g_frank_xor;
				mediaHeader.checksum += pata[i];
			}
			pServer->send_data((char*)&mediaHeader, sizeof(media_header));
			//do send audio data
			printf("get audio data len: %d\n", aupk->len);
			pServer->send_data((char *)g_szDataBuf_A2, aupk->len);
			cyclic_buf_consume(&g_cbAudio);
		}
	}
}

void MyTcpServer::disConnectAll()
{
	for (int i = 0; i<g_iTotalConn; i++)
	{
		if (NULL != g_CliSocketArr[i])
		{
			Cleanup(i);
		}
	}
	cyclic_buf_consume(&g_cb);
}