#include <windows.h>
#include "MyRingBuffer.h"


MyRingBuffer::MyRingBuffer(void)
{
	memset(m_dataBuf, 0, sizeof(char)*MAX_BUF_SIZE);
	m_realData = NULL;
	m_tatol_wLen = 0;
	m_tatol_rLen = 0;
	m_len = 0;
	m_ncount = 0;
}


MyRingBuffer::~MyRingBuffer(void)
{
}

void MyRingBuffer::write(char *data, int len)
{
	m_myCriticalSection.enter();
	if (m_tatol_wLen + len >= MAX_BUF_SIZE)
	{
		m_tatol_wLen = 0;
	}
	if (m_ncount >= 65535)
		m_ncount = 0;
	m_len = len;
	memcpy(m_dataBuf + m_tatol_wLen, data, len);
	m_tatol_wLen += len;
	m_ncount++;
	m_myCriticalSection.leave();
}

void MyRingBuffer::read(char *data, int len)
{
	m_myCriticalSection.enter();
	if (m_tatol_rLen + len >= MAX_BUF_SIZE)
	{
		m_tatol_wLen = 0;
	}
	memcpy(data, m_dataBuf + m_tatol_rLen, len);
	m_tatol_rLen +=len;
	m_myCriticalSection.leave();
}

char * MyRingBuffer::GetData(int len)
{
	m_myCriticalSection.enter();
	if (m_tatol_rLen + len >= MAX_BUF_SIZE)
	{
		m_tatol_rLen = 0;
	}
	m_realData  = m_dataBuf + m_tatol_rLen;
	m_tatol_rLen += len;
	m_myCriticalSection.leave();
	return m_realData;
}

BOOL MyRingBuffer::isEmpty()
{
	return (strlen(m_dataBuf) == 0);
}

int MyRingBuffer::GetLength()
{
	return m_len;
}

int MyRingBuffer::GetCount()
{
	int nRet = 0;
	m_myCriticalSection.enter();
	nRet = m_ncount;
	m_myCriticalSection.leave();
	return nRet;
}

/**************************************************************/
MyRingBuffer2::MyRingBuffer2()
{
	memset(g_szDataBuf_A, 0, MAX_BUF_SIZE);
	m_count = 0;
}
MyRingBuffer2::~MyRingBuffer2()
{

}
