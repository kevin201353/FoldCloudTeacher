#ifndef MYTCPCLIENT_H
#define MYTCPCLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTime>

class mytcpclient : public QObject
{
    Q_OBJECT
public:
    explicit mytcpclient(QObject *parent = 0);
    ~mytcpclient();
public:
     void setHostAddress(QHostAddress address, int port=6666);
     int readdata(QTcpSocket* sock_fd , char *buffer, int len);
     void closeTcpClient();
     int send_data(char *buf,  int len);
signals:

public slots:
    void newConnect();
    void readMessage();
    void displayError(QAbstractSocket::SocketError socketError);
private:
    QTcpSocket *tcpClient;
    QHostAddress  hostAddress;
    int  tcpPort;
};

#endif // MYTCPCLIENT_H
