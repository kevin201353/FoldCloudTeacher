#include <QCoreApplication>
#include <QDebug>
#include "tcpserver.h"

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    //return a.exec();
    TcpServer  tcpserver;
    qDebug() << "this is main start.";
    tcpserverThread  tcpThread;
    int tmp = getchar();
    return 0;
}
