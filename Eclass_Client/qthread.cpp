#include "qthread.h"

QThread::QThread(QObject *parent) : QObject(parent)
{

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
