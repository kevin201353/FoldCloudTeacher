#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QList>
#include <QObject>
#include "streamparsexml.h"
#include <QMap>
#include <QString>

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread();
    ~Thread();
public:
    void stop();
    void setMessage(QList<QString> msglist);
    void settype(QString skey, int ntype);
protected:
    void run();
    void SendMessage();
private:
    QList<QString>  m_msgList;
    volatile bool m_stopped;
    QMap<QString, int> m_ntype;
};

class DataThread :  public QThread
{
    Q_OBJECT
public:
    DataThread();
    ~DataThread();
signals:
    void NoticeShow(StruInfo* szMsg);
public:
    void stop();
    void processdata();
    void settype(QString skey, int ntype);
protected:
    void run();
private:
    volatile bool m_stopped;
    QMap<QString, int> m_ntype;
    StreamParseXml  parsexml;
};

class classThread : public QThread
{
    Q_OBJECT
public:
    classThread();
    ~classThread();
public:
    void stop();
    void setMac(QString szMac);
protected:
    void run() Q_DECL_OVERRIDE;
    void processdata();
private:
    volatile bool  m_stop;
    QString m_strMac;

};


class NoticeThread : public  QThread
{
    Q_OBJECT
 public:
    NoticeThread();
    ~NoticeThread();
signals:
    void SINoticeShow(StruInfo* info);
 public:
    void stop();
 protected:
    void run() Q_DECL_OVERRIDE;
    void processdata();
 private:
    volatile bool m_stop;
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

class tcpDataThread : public QThread
{
    Q_OBJECT
public:
    tcpDataThread();
    ~tcpDataThread();
public:
   void stop();
protected:
   void run() Q_DECL_OVERRIDE;
private:
   volatile bool m_stop;
};
#endif // THREAD_H
