#include "tcpserver.h"
#include <QDebug>
#include <QHostAddress>
#include <QMutexLocker>
#include <QMutex>
#include "global.h"

const int kPort = 22343;
const QString connectflag = "server return message:connect server is ok!";
const QString hasconnect = "I'm still connect";
const int kBase = 1000;

QMap<QString, QTcpSocket*> g_userlist;
QMutex  g_tcpMutex;
//char* magic = "BEAF";
//uint32_t *data;
//uint32_t checksum;
//int fend  = 0x45464353; //SCFE

const char *sockerErrors[100] = {
    "ConnectionRefusedError",
    "RemoteHostClosedError",
    "HostNotFoundError",
    "SocketAccessError",
    "SocketResourceError",
    "SocketTimeoutError",
    "DatagramTooLargeError",
    "NetworkError",
    "AddressInUseError",
    "SocketAddressNotAvailableError",
    "UnsupportedSocketOperationError",
    "ProxyAuthenticationRequiredError",
    "SslHandshakeFailedError",
    "UnfinishedSocketOperationError",
    "ProxyConnectionRefusedError",
    "ProxyConnectionClosedError",
    "ProxyConnectionTimeoutError",
    "ProxyNotFoundError",
    "ProxyProtocolError",
    "UnknownSocketError"
};

TcpServer::TcpServer(QObject *parent) :
    QObject(parent),
    m_tcpserver(NULL),
    m_hearttime(5),
    m_startHeartTime(false)
{
    g_userlist.clear();
    initserver();
}

TcpServer::~TcpServer()
{
    if (NULL != m_tcpserver)
    {
        m_tcpserver->close();
        m_tcpserver = NULL;
    }
}

void TcpServer::slot_newConnection()
{
    //得到每一个连接成功的客户端套接字
    QTcpSocket *newtcpsocket = m_tcpserver->nextPendingConnection();
    if (NULL == newtcpsocket)
    {
        qDebug() << "socket is error";
        return;
    }
    m_tcpsocket = newtcpsocket;
    QHostAddress address = newtcpsocket->peerAddress();
    QString clientIp = address.toString();
    qDebug() << "clientIp: " << clientIp;
    qDebug() << "current user count: " << g_userlist.size();
    if (g_userlist.size() > 0)
    {
        qDebug() << "they is :";
    }
    QMap<QString, QTcpSocket*>::iterator iter;
    for (iter = g_userlist.begin(); iter != g_userlist.end(); ++iter)
    {
        qDebug() << iter.key();
    }
    if (NULL == g_userlist[clientIp])
    {
        qDebug() << "new user join it's ip: " << clientIp;
        g_userlist.insert(clientIp, newtcpsocket);
        qDebug() << "current user count 222: " << g_userlist.size();
        connect(newtcpsocket, SIGNAL(readyRead()), this, SLOT(slot_readdata()));
        connect(newtcpsocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slot_errors(QAbstractSocket::SocketError)));
//        if ("" != clientIp)
//        {
//            senddata(newtcpsocket, connectflag);
//        }
        if (false == m_startHeartTime)
        {
            qDebug() << "start heart time";
            startHeart();
            m_startHeartTime = true;
        }
    }
}

void TcpServer::slot_readdata()
{
    QTcpSocket *newtcpsocket = static_cast<QTcpSocket*>(this->sender());
    QString clientip = newtcpsocket->peerAddress().toString();
    qDebug() << "clientip :" << clientip;
    QString str = newtcpsocket->readAll();
    QByteArray dd = str.toLatin1();
    QString strTmp(dd.data());
    qDebug() << "recv from eclass_client data :" << strTmp;
    if (strTmp == "1")
    {

    }
}

void TcpServer::slot_errors(QAbstractSocket::SocketError errors)
{
    QString displays = "";
    QTcpSocket *newtcpsocket = static_cast<QTcpSocket*>(this->sender());
    //发生错误时，服务端踢掉出错socket
    g_userlist.remove(newtcpsocket->peerAddress().toString());
    qDebug() << "user " << newtcpsocket->peerAddress().toString() << " is left..";
    qDebug("socket error code :%d.\n", errors);
    if( errors == 1 )
    {
        return;
    }
    else if( -1 == errors )
    {
        displays = sockerErrors[19];
    }
    else
    {
        displays = sockerErrors[errors];
    }
    qDebug() << "socket error string:" << displays;
}

void TcpServer::slot_updateUserList()
{
    QMap<QString, QTcpSocket*>::iterator iter;
    for (iter = g_userlist.begin(); iter != g_userlist.end(); ++iter)
    {
        QAbstractSocket::SocketState curstate = (*iter)->state();
        qDebug("updateUserList connect socket state :%d.\n", curstate);
        if (QAbstractSocket::UnconnectedState == curstate ||
                QAbstractSocket::ClosingState == curstate)
        {
            qDebug() << "erase close socket";
            iter = g_userlist.erase(iter);
        }
    }
}

void TcpServer::initserver()
{
    m_tcpserver = new QTcpServer(this);
    if (!m_tcpserver->listen(QHostAddress::Any, kPort))
    {
        qDebug() << "listen error: " << m_tcpserver->errorString();
    }else
    {
        qDebug() << "server start is ok!";
    }
    //来一个客户端连接， 就会产生一个slot
    connect(m_tcpserver, SIGNAL(newConnection()), this, SLOT(slot_newConnection()));
    m_timer = new QTimer();
    //5s检测一次连接状态并更新连接用户
    connect(m_timer, SIGNAL(timeout()), this , SLOT(slot_updateUserList()));
}

void TcpServer::senddata(QTcpSocket *newtcpsocket, const QString &str)
{
    char tmp[512] = {0};
    QByteArray ba = str.toLatin1();
    strcpy(tmp, ba.data());
    int len = newtcpsocket->write(tmp);
    if (len > 0)
    {
        QString userip = newtcpsocket->peerAddress().toString();
        qDebug() << "user " << userip << "has connected ok.";
    }
}

int TcpServer::send_data(QTcpSocket *newtcpsocket, char *buf,  int len)
{
    int l = 0;
    int ret = 0;
    char *d = NULL;
    d = buf;
    l = len;
    while(l){
        ret = newtcpsocket->write(buf, l);
        if(ret <= 0){
            qDebug() << "server write data faild.";
            break;
        }
        l -= ret;
        d += ret;
        qDebug("send socket State:%d\n", newtcpsocket->state()); // State: 3（ConnectedState）正确
    }
    //newtcpsocket->waitForBytesWritten(3000);
    return 0;
}

void TcpServer::startHeart()
{
    int socketcount = g_userlist.size();
    if (socketcount > 0)
    {
        m_timer->start(m_hearttime *kBase);
    }else
    {
        m_startHeartTime = false;
        m_timer->stop();
    }
}

int TcpServer::readdata(QTcpSocket* sock_fd , char *buffer, int len)
{
    int l, ret = 0;
    char *d;
    d = buffer;
    l = len;

    while(l>0){
        ret = sock_fd->read(d, l);
        qDebug("state 22222:%d\n", sock_fd->state());
        if( ret < 0 ){
            qDebug() << "Socket read data error.";
            return -1;
        }
        if (ret == 0 && !sock_fd->waitForReadyRead())
            break;
        l -= ret;
        d += ret;
    }
    return 0;
}
