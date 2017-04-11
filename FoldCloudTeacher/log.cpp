#include "log.h"
#include <QMutexLocker>

QMutex g_mutexfile;
bool writeLogFile(QtMsgType type, const QString msg)
{
    QMutexLocker Locker(&g_mutexfile);
    mylog g_mylog;
    g_mylog.open();
    g_mylog.writeLogFile(type, msg);
    g_mylog.close();
    return true;
}
/*******************************************************/
//log 文件
mylog::mylog()
{
    m_pLog = NULL;
    m_pLog = new QFile(LOG_FILE_NAME);
    Q_ASSERT(m_pLog != NULL);
}

mylog::~mylog()
{
    if (m_pLog)
    {
        delete m_pLog;
        m_pLog = NULL;
    }
}

bool mylog::open()
{
    if(!(m_pLog->open(QIODevice::Text | QIODevice::Append)))
        return false;
    return true;
}

void mylog::writeLogFile(QtMsgType type, const QString msg)
{
    if (m_pLog != NULL)
    {
        QString writeData;
        writeData.clear();
        switch(type)
        {
        case QtDebugMsg:
            writeData = DEBUG;
            break;
        case QtInfoMsg:
            writeData = INFO;
            break;
        case QtWarningMsg:
            writeData = WARNING;
            break;
        case QtCriticalMsg:
            writeData = CRITICAL;
            break;
        case QtFatalMsg:
            writeData = FATAL;
            break;
        }

        writeData += msg;
        writeData += "\n";
        m_pLog->write(writeData.toStdString().c_str());
    }
}

void mylog::close()
{
    if (m_pLog != NULL)
    {
        m_pLog->close();
    }
}
