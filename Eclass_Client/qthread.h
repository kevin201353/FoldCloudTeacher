#ifndef QTHREAD_H
#define QTHREAD_H

#include <QObject>

class QThread : public QObject, QThread
{
    Q_OBJECT
public:
    explicit QThread(QObject *parent = 0);

signals:
public slots:
};

class tcpserverThread : public QThread
{
    Q_OBJECT
public:
    tcpserverThread();
    ~tcpserverThread();
signals:
    void tcp_msg();
public:
   void stop();
protected:
   void run() Q_DECL_OVERRIDE;
private:
   volatile bool m_stop;
};

#endif // QTHREAD_H
