#ifndef STULIST_H
#define STULIST_H

#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QString>
#include <QList>
#include <QFile>
#include "streamparsexml.h"
#include "thread.h"
#include <QObject>
#include "cmytableview.h"

namespace Ui {
class stulist;
}

class stulist : public QWidget
{
    Q_OBJECT

public:
    explicit stulist(QWidget *parent = 0);
    ~stulist();
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
public:
    void settype(short ntype); //1:学生列表
    inline void setmac(const QString mac) {
        m_szMac = mac;
    };
    inline void setaddress(QString addr) {
        m_addrserver = addr;
    };
    QObject * GetList();
    inline void setWidget(QWidget *widget){
        m_pWidget = widget;
    }
private slots:
    void exit_widget();
private:
    Ui::stulist *ui;
    QPoint  m_LastMousePosition;
    bool  m_moving;
    QString m_szMac;
    int m_ntype;  //0:举手  1:学生
    QString  m_addrserver;
    QWidget *m_pWidget;
    CMytableview *m_mytableview;
};

#endif // STULIST_H
