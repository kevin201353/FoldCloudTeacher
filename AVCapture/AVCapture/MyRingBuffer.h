#pragma once
#define  MAX_BUF_SIZE   0x40000
#include "myCriticalSection.h"
#include "global.h"

class MyRingBuffer
{
public:
	MyRingBuffer();
	~MyRingBuffer(void);
public:
	void write(char *data, int len);
	void read(char *data, int len);
	char *GetData(int len);
	BOOL isEmpty();
	int GetLength();
	int GetCount();
private:
	char m_dataBuf[MAX_BUF_SIZE];
	myCriticalSection  m_myCriticalSection;
	char *m_realData;
	long m_tatol_wLen;
	long m_tatol_rLen;
	int  m_len;
	int  m_ncount;
};

class MyRingBuffer2
{
public:
	MyRingBuffer2();
	~MyRingBuffer2(void);

public:
	char g_szDataBuf_A[MAX_BUF_SIZE];
	int  m_len;
	int  m_count;
};

