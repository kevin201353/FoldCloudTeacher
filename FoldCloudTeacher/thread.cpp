#include "thread.h"
#include <QDebug>
#include "log.h"
#include "myhttp.h"
#include "global.h"
#include "thread.h"
#include <QMutex>
#include <QMutexLocker>
#include "myqlist.h"
#include "tcpserver.h"
#include <QMutexLocker>
#include <QMutex>

QString g_xmldata;
QMutex g_mutex;
extern QMap<QString, QObject *> g_mapObject;
extern MyQList   g_NoticeList;
Thread::Thread()
{
    m_stopped = false;
}

Thread::~Thread()
{
}

void Thread::run()
{
    while (!m_stopped)
    {
        QMutexLocker Locker(&g_mutex);
        SendMessage();
        sleep(5);
    }
    m_stopped = false;
}

void Thread::stop()
{
    m_stopped = true;
}

void Thread::setMessage(QList<QString> msglist)
{
    m_msgList = msglist;
}

void Thread::SendMessage()
{
    //qDebug()<<"start send message.";
    if (m_msgList.size() > 0)
    {
        qDebug()<<"start send request.";
        QString szUrl = m_msgList.at(0);
        QString szMac = m_msgList.at(1);
        szUrl += "?";
        szUrl += szMac;
        writeLogFile(QtDebugMsg, szUrl);
        myHttp http;
        if (!http.Get(szUrl))
        {
            writeLogFile(QtDebugMsg, "sendMessage http request failed.");
        }
        http.GetData(g_xmldata);
        qDebug() << g_xmldata;
    }
}

void Thread::settype(QString skey, int ntype)
{
     m_ntype[skey] = ntype;
}

/***********************************************************************************************/
//data process
DataThread::DataThread()
{
    m_stopped = false;
}

DataThread::~DataThread()
{

}

void DataThread::stop()
{
     m_stopped = true;
     parsexml.setStop(m_stopped);
}

void DataThread::run()
{
    while (!m_stopped)
    {
        processdata();
        sleep(1);
    }
    m_stopped = false;
}

void DataThread::processdata()
{
    QMutexLocker Locker(&g_mutex);
    if (g_xmldata.length() > 0 && !g_xmldata.isEmpty())
    {
        parsexml.setType(m_ntype);
        parsexml.readXmlstuinfo(g_xmldata);
        g_xmldata = "";
    }
}

void DataThread::settype(QString skey, int ntype)
{
    m_ntype[skey] = ntype;
}

/*****************************************************************************/
//class thread
classThread::classThread()
{
    m_stop = false;
}

classThread::~classThread()
{

}

void classThread::stop()
{
    m_stop = true;
}

void classThread::run()
{
    while (!m_stop)
    {
        processdata();
        sleep(3);
    }
}

void classThread::processdata()
{
    QString url = HTTP_URL_HEAD;
    url += SERVICE_ADDRESS;
    url += "/service/desktops/classinfo";
    //m_strMac = "00:1a:4a:16:01:57";
    QString data = "vmMac=";
    data += m_strMac;
    myHttp http;
    QString strdeg = "getclassinfo : ";
    strdeg += url;
    strdeg += "------";
    strdeg += data;
    writeLogFile(QtDebugMsg, strdeg);
    if (!http.Post(url, data))
    {
        writeLogFile(QtDebugMsg, "getclassinfo failed.");
    }
    QString strBuf;
    http.GetData(strBuf);
    if (strBuf.length() > 0 && !strBuf.isEmpty())
    {
        StreamParseXml xmlparse;
        xmlparse.readxmlclass(strBuf);
    }
}

void classThread::setMac(QString szMac)
{
    m_strMac = szMac;
}


/***************************************************************************************************/
NoticeThread::NoticeThread()
{
    m_stop = false;
}

NoticeThread::~NoticeThread()
{

}

void NoticeThread::stop()
{
    m_stop = true;
}

void NoticeThread::run()
{
    while(!m_stop)
    {
        processdata();
        sleep(1);
    }
}

void  NoticeThread::processdata()
{
//    int nsize = g_NoticeList.getsize();
//    for (int i=0; i<nsize; i++)
//    {
//        StruInfo *info = g_NoticeList.at(i);
//        if (info != NULL)
//        {
//            emit SINoticeShow(info);
//        }
//    }
}


/**********************************************************************/
   //tcp server
/***********************************************************************/
tcpserverThread::tcpserverThread()
{
    m_stop = false;
}

tcpserverThread::~tcpserverThread()
{

}

void tcpserverThread::stop()
{
    m_stop = true;
}

void tcpserverThread::run()
{
    while(!m_stop)
    {
        emit tcp_msg();
        sleep(1);
    }
}


 tcpDataThread::tcpDataThread()
 {
     m_stop = false;
 }

 tcpDataThread::~tcpDataThread()
 {

 }

 void tcpDataThread::stop()
 {
     m_stop = true;
 }

 void tcpDataThread::run()
 {
    while(!m_stop)
    {
        //delay
        sleep(1);
    }
 }
