#ifndef MYHTTP_H
#define MYHTTP_H

#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QTextCodec>
#include <QObject>
#include <QEventLoop>
#include <QTimer>

class myHttp : public QObject
{
    Q_OBJECT
public:
    myHttp();
    myHttp(QString json);
    ~myHttp();
public:
    bool Post(QString url, QString append);
    void GetData(QString &Buf);
    bool Get(QString url);
public slots:
    void replyFinished(QNetworkReply* reply); //用于处理响应返回的数据
private:
    QNetworkAccessManager   *m_pNetManager;
    QString m_XmlMessage;
    QEventLoop *m_peventLoop;
    QNetworkReply *m_preply;
    QNetworkRequest m_pRequst;
    QTimer  m_timer;
};

#endif // MYHTTP_H
