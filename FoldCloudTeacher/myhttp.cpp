#include "myhttp.h"
#include <QDebug>
#include <QByteArray>
#include "log.h"

myHttp::myHttp()
{
    m_pNetManager = new QNetworkAccessManager(this);
    m_peventLoop = new QEventLoop();
    m_pRequst.setHeader(QNetworkRequest::ContentTypeHeader,
                        QString("application/x-www-form-urlencoded"));
    m_pRequst.setRawHeader("eduToken", "ABCDEF0123456789");
    m_pRequst.setRawHeader("Accept", "application/xml");
    //m_pRequst.setRawHeader(QByteArray("Content-Type"), QByteArray("charset=UTF-8"));
    m_timer.setInterval(5000);
    m_timer.setSingleShot(true);
    connect(&m_timer,  SIGNAL(timeout()), m_peventLoop, SLOT(quit()));
    connect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), m_peventLoop, SLOT(quit()));
}

myHttp::myHttp(QString json)
{
    m_pNetManager = new QNetworkAccessManager(this);
    m_peventLoop = new QEventLoop();
    m_pRequst.setHeader(QNetworkRequest::ContentTypeHeader,
                        QString("application/x-www-form-urlencoded"));
    m_pRequst.setRawHeader("eduToken", "ABCDEF0123456789");
    //m_pRequst.setRawHeader(QByteArray("Content-Type"), QByteArray("charset=UTF-8"));
    QString header = "application/";
    header += json;
    QByteArray bb = header.toLatin1();
    m_pRequst.setRawHeader("Accept", bb);
    m_timer.setInterval(5000);
    m_timer.setSingleShot(true);
    connect(&m_timer,  SIGNAL(timeout()), m_peventLoop, SLOT(quit()));
    connect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), m_peventLoop, SLOT(quit()));
}

myHttp::~myHttp()
{
    m_timer.stop();
    if (m_pNetManager)
    {
        m_pNetManager->deleteLater();
        delete m_pNetManager;
    }
    if (m_peventLoop)
    {
        m_peventLoop->exit();
        m_peventLoop->deleteLater();
        delete m_peventLoop;
    }
}
bool myHttp::Post(QString url, QString append)
{
    QUrl    Tempurl(url);
    QByteArray  appen(append.toLatin1());
    m_pRequst.setUrl(Tempurl);
    //QNetworkRequest head;
    //head.setRawHeader(QByteArray("eduToken"), QByteArray("ABCDEF0123456789"));
    //head.setRawHeader(QByteArray("Accept"), QByteArray("application/xml"));
    //head.setRawHeader(QByteArray("Content-Type"), QByteArray("application/xml;charset=UTF-8"));
    //head.setUrl(Tempurl);
    QString sTmp(appen);
    qDebug() << sTmp.toStdString().c_str();
    m_preply = m_pNetManager->post(m_pRequst, appen);
    if(NULL == m_preply)
        return false;
    return true;
}
bool myHttp::Get(QString url)
{
    QUrl    Tempurl(url);
    m_pRequst.setUrl(Tempurl);
    //QNetworkRequest head;
    //head.setRawHeader(QByteArray("eduToken"), QByteArray("ABCDEF0123456789"));
    //head.setRawHeader(QByteArray("Accept"), QByteArray("application/xml"));
    //head.setUrl(Tempurl);
    m_preply = m_pNetManager->get(m_pRequst);
    if(NULL == m_preply)
        return false;
    return true;
}

//同步处理 http
void myHttp::GetData(QString &Buf)
{
    if(m_preply == NULL)
        return ;
    m_timer.start();
    m_peventLoop->exec();
    if (m_timer.isActive())  //处理响应
    {
        m_timer.stop();
        if (m_preply->error() != QNetworkReply::NoError)
        {
            // 错误处理
            qDebug() << "Error String : " << m_preply->errorString();
            writeLogFile(QtDebugMsg, m_preply->errorString());
        } else {
            QVariant variant = m_preply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            int nStatusCode = variant.toInt();
            // 根据状态码做进一步数据处理
            qDebug() << "http m_preply Status Code : " << nStatusCode;
            if (nStatusCode == 200)
            {
                m_XmlMessage = m_preply->readAll();
                Buf = m_XmlMessage;
                writeLogFile(QtDebugMsg, Buf);
                qDebug() << Buf;
            }
        }
    }else //time out
    {
        disconnect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), m_peventLoop, SLOT(quit()));
        qDebug() << "http request Timeout ........";
    }
}

//异步处理 http
void myHttp::replyFinished(QNetworkReply *reply)
{
    qDebug()<<reply->readAll();
}
