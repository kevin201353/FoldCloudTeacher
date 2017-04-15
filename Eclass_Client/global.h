#ifndef GLOBAL_H
#define GLOBAL_H
#include <QObject>
#include <QMap>
#include <QList>
#include  "log.h"

#define MESSAGEBOX(msg, parent) ({QMessageBox msgBox(parent); \
                         msgBox.setText(msg); \
                         msgBox.exec();})

#define  USER_MSG_TABLEVIEW          101
#define  USER_MSG_DEALSTULIST        102
#define  USER_MSG_NOTICEWINDOW       103
#define  USER_MSG_CLASSINFO          104
#define  USER_MSG_UPDATEHANDUP       105

struct XSize {
    int x;
    int y;
    int width;
    int height;
};

struct ReportMsg {
    QObject *obj;
    unsigned int action; //消息类型
    unsigned int val1;
    unsigned int val2;
    QString   str;
    QString   strval;
};

struct StruInfo {
    QString id;
    QString name;
    QString noSeat;
    QString apId;
    QString handup;
};

struct NetConfig {
    QString protocol;
    QString addr;
    QString port;
};

struct media_header
{
    char magic[4];
    uint32_t type; // 0：MJPEG 1: H264 I frame, 2: H264 P frame, 3: Audio frame
    uint32_t serial; // audio/video id
    uint16_t width; // frame width
    uint16_t height; // frame height
    uint16_t cursor_x; // Mouse pointer X
    uint16_t cursor_y; //  Mouse pointer Y
    uint32_t time_stamp; // 单位：毫秒（ms）
    uint32_t size; // 后续数据长度，不包含此header长度
    uint32_t checksum;
};

struct class_template {
   QString name; //模板名称
   QString id;   //模板id
};


struct class_Rooms {
   QString id;  //教室id
   QString name;
};

struct class_sel {
  QString  id;
  QString  name;
  int   ndex;
};
#define FRAME_TYPE_MJPEG        0
#define FRAME_TYPE_H264_I       1
#define FRAME_TYPE_H264_P       2
#define FRAME_TYPE_AUDIO        3
#define FRAME_TYPE_CURSOR	4

#define   HTTP_URL_HEAD  "http://"
#define   SERVICE_ADDRESS  "192.168.0.164:9090"

extern QMap<QString, QObject *> g_mapObject;
typedef void (*MsgCallBackFun)(struct ReportMsg msg);
void msg_respose(struct ReportMsg msg);
void call_msg_back(MsgCallBackFun fun, struct ReportMsg msg);

#endif // GLOBAL_H
