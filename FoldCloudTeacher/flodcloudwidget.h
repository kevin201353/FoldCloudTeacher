#ifndef FLODCLOUDWIDGET_H
#define FLODCLOUDWIDGET_H

#include <QWidget>
#include "stulist.h"
#include "thread.h"
#include "classesform.h"
#include "tcpserver.h"
#include "mytcpclient.h"
#include "global.h"
#include <QList>
#include <QProcess>
#include "messagebox.h"

namespace Ui {
class FlodCloudWidget;
}

class FlodCloudWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlodCloudWidget(QWidget *parent = 0);
    ~FlodCloudWidget();
public:
    inline Thread* GetThread() const {
        return m_pthread;
    };
    inline DataThread* GetDataThread() const {
        return m_pDataThread;
    };
    QString getHostMacAddress();
    void GetClassTemplate();
private:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
private slots:
    void on_class_clicked();
    void on_demostrate_clicked();
    void on_stuDemostrate_clicked();
    void on_tcp_Msg();
    void on_class_startrp();
    void av_singal_start();
    void av_singal_stop();
private:
    Ui::FlodCloudWidget *ui;
    stulist  *m_pstulist;
    Thread   *m_pthread;
    DataThread *m_pDataThread;
    QString  m_strMac;
    bool    m_bstart;
    ClassesForm  *m_pclassesForm;
    tcpserverThread *m_tcpserverThrd;
    TcpServer  *m_pTcpServer;
    QProcess*   m_myProcess;
    MessageBox*  m_messageBox;
};

#endif // FLODCLOUDWIDGET_H
