#pragma once
#define FRAME_TYPE_MJPEG        0
#define FRAME_TYPE_H264_I       1
#define FRAME_TYPE_H264_P       2
#define FRAME_TYPE_AUDIO        3
#define FRAME_TYPE_CURSOR		4

#define MAX_DATA_BUF            204800  // 256000 //
#define MAX_DATA_BUF_A			102400
#define USER_MSG_AUDIO_START    10001

struct media_header
{
	char magic[4];
	unsigned int type; // 0：MJPEG 1: H264 I frame, 2: H264 P frame, 3: Audio frame
	unsigned int serial; // audio/video id
	unsigned short width; // frame width
	unsigned short height; // frame height
	unsigned short cursor_x; // Mouse pointer X
	unsigned short cursor_y; //  Mouse pointer Y
	unsigned int time_stamp; // 单位：毫秒（ms）
	unsigned int size; // 后续数据长度，不包含此header长度
	unsigned int checksum;
};

struct ReportMsg {
	unsigned int action; //消息类型
	unsigned int len;
};

struct cyclic_buf
{
	int p_index;
	int c_index;
	int num;
	int data_size;
	char valid[256];
	void *data[256];
};

struct Video_Package {
	media_header  header;
	int  len;
	char  pata[MAX_DATA_BUF];
};


struct Audio_Package {
	int len;
	char  pdata[102400];
};

extern char g_szDataBuf[MAX_DATA_BUF];
typedef void(*MsgCallBackFun)(struct ReportMsg msg);
void msg_respose(struct ReportMsg msg);
void call_msg_back(MsgCallBackFun fun, struct ReportMsg msg);

#include "cyclic_buf.h"