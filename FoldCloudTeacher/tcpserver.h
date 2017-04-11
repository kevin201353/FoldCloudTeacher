#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMap>
#include <QTimer>


class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    ~TcpServer();
signals:

public slots:
    void slot_newConnection();
    void slot_readdata();
    void slot_errors(QAbstractSocket::SocketError errors);
    void slot_updateUserList();
public:
    int send_data(QTcpSocket *newtcpsocket, char *buf,  int len);
    int readdata(QTcpSocket* sock_fd , char *buffer, int len);
private:
    QTcpServer  *m_tcpserver;
    QTcpSocket  *m_tcpsocket;
    QTimer  *m_timer;
    int m_hearttime;
    bool  m_startHeartTime;
private:
    void initserver();
    void senddata(QTcpSocket *newtcpsocket, const QString &str);
    void startHeart();
};

#endif // TCPSERVER_H
