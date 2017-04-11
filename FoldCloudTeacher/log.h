#ifndef LOG_H
#define LOG_H

#include <QFile>
#include <QDateTime>
#include <QString>
#include <QtGlobal>

#define LOG_FILE_NAME "VmStuLog.txt"
#define GET_TIME (" " + QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") + " ")
#define DEBUG    ("debug: " + GET_TIME + __FILE__ + "(" + __LINE__ + ")\n\t")
#define INFO     ("info: " + GET_TIME + __FILE__ + "(" + __LINE__ + ")\n\t")
#define WARNING  ("warning: " + GET_TIME + __FILE__ + "(" + __LINE__ + ")\n\t")
#define CRITICAL ("critical: " + GET_TIME + __FILE__ + "(" + __LINE__ + ")\n\t")
#define FATAL    ("fatal: " + GET_TIME + __FILE__ + "(" + __LINE__ + ")\n\t")

bool writeLogFile(QtMsgType type, const QString msg);

class mylog
{
public:
    mylog();
    ~mylog();
public:
    bool open();
    void writeLogFile(QtMsgType type, const QString msg);
    void close();
private:
    QFile* m_pLog;
};

extern mylog g_mylog;
#endif // LOG_H
