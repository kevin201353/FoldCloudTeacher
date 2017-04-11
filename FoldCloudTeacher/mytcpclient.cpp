#include "mytcpclient.h"

mytcpclient::mytcpclient(QObject *parent) :
    QObject(parent),
    tcpClient(NULL),
    tcpPort(6666)
{
    tcpClient = new QTcpSocket(this);
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

mytcpclient::~mytcpclient()
{
    closeTcpClient();
}

void mytcpclient::setHostAddress(QHostAddress address, int port)
{
     hostAddress = address;
     tcpPort = port;
}

void mytcpclient::newConnect()
{
    if (NULL != tcpClient)
    {
        tcpClient->abort(); //取消已有的连接
        QString str = hostAddress.toString();
        tcpClient->connectToHost(str, tcpPort); //连接到指定ip地址和端口的主机
        if (!tcpClient->waitForConnected(5000))
        {
            return;
        }
        qDebug("state :%d\n", tcpClient->state());
    }
}

void mytcpclient::readMessage()
{
    char buffer[1024] = {0};
    int nRet = readdata(tcpClient, buffer, sizeof(buffer));
    if ( nRet < 0)
    {
        qDebug() << "readMessage read data failed.";
    }else
    {
        QString str(buffer);
        qDebug() << "recv data success, data: " << str;
    }
}

void mytcpclient::displayError(QAbstractSocket::SocketError  socketError)
{
    switch(socketError)
    {
        //RemoteHostClosedError为远处主机关闭了连接时发出的错误信号
        case QAbstractSocket::RemoteHostClosedError :
            break;
        default :
            qDebug() << tcpClient->errorString();
    }
}

int mytcpclient::readdata(QTcpSocket* sock_fd , char *buffer, int len)
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

void mytcpclient::closeTcpClient()
{
    if (NULL != tcpClient)
    {
        tcpClient->close();
        tcpClient = NULL;
    }
}

int mytcpclient::send_data(char *buf,  int len)
{
    int l = 0;
    int ret = 0;
    char *d = NULL;
    d = buf;
    l = len;
    while(l){
        ret = tcpClient->write(buf, l);
        if(ret <= 0){
            qDebug() << "server write data faild.";
            break;
        }
        l -= ret;
        d += ret;
        qDebug("State:%d\n", tcpClient->state()); // State: 3（ConnectedState）正确
    }
    if (!tcpClient->waitForBytesWritten())
    {
       tcpClient->disconnectFromHost();
       tcpClient->deleteLater();
       qDebug() << "client send_data failed, tcpclient close.";
    }
    return 0;
}
