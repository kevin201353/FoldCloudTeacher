#include "global.h"
#include "cmytableview.h"
#include <QDebug>
#include "stulist.h"
#include "cmytableview.h"
#include <QStandardItem>
#include "log.h"
#include "streamparsexml.h"
#include "myhttp.h"
#include "global.h"
#include <QMutexLocker>
#include "flodcloudwidget.h"
#include <QPushButton>

extern QList<StruInfo> g_stu2List;
extern QList<StruInfo> g_handupList;
extern QMap<QString, QObject *> g_mapObject;
extern NetConfig g_config;


bool _isExist_seat(CMytableview *pView, QString seat)
{
     int ncount = pView->GetMyItemModel(1)->rowCount();
     for (int i=0; i<ncount; i++)
     {
         QStandardItem * item = pView->GetMyItemModel(1)->item(i, 0);
         if (item != NULL)
         {
             QString tmp = item->text();
             if (tmp.length() > 0 && !tmp.isEmpty())
             {
                 int index = tmp.indexOf("-");
                 QString seat_tmp = tmp.mid(index + 1);
                 seat_tmp = seat_tmp.trimmed();
                 qDebug() << "add student seat" << seat_tmp;
                 if (seat_tmp == seat)
                    return true;
             }
         }
     }
     return false;
}

void call_msg_back(MsgCallBackFun fun, struct ReportMsg msg)
{
    fun(msg);
}

void msg_respose(struct ReportMsg msg)
{
    switch (msg.action) {
    case USER_MSG_TABLEVIEW:
        {
            CMytableview *myTableView = qobject_cast<CMytableview *>(msg.obj);
            if (myTableView != NULL)
            {
                int col = msg.val1;
                int row = msg.val2;
                if (row >=0 && g_stu2List.size() > 0)
                {
                    char sztmp[100] = {0};
                    sprintf(sztmp, "col = %d, row = %d .", col, row);
                    StruInfo info = g_stu2List.at(row);
                    QString url = g_config.protocol;
                    url += g_config.addr;
                    url += "/service/classes/specify_stu_show";
                    QString data;
                    data = "apId=";
                    data += info.apId;
                    data += "&";
                    data += "onOff=true";
                    myHttp *http = new myHttp;
                    if (http)
                    {
                        QString str = "stu list item select : ";
                        str += url;
                        str += "------";
                        str += data;
                        writeLogFile(QtDebugMsg, str);
                        qDebug()<< str;
//                        if (!http->Post(url, data))
//                        {
//                            writeLogFile(QtDebugMsg, "教室指定学生演示失败.");
//                        }
                        QString strTmp;
                        str = "specify_stu_show -------";
//                        http->GetData(strTmp);
                        str += strTmp;
                        writeLogFile(QtDebugMsg, str);
                        delete http;
                        http = NULL;
                    }
                    //MESSAGEBOX(sztmp, myTableView);
                }
            }
        }
        break;
    case USER_MSG_DEALSTULIST:
        {
            stulist *list = (stulist *)g_mapObject["stulist"];
            if (list != NULL)
            {
                CMytableview *pView = (CMytableview *)list->GetList();
                if (pView != NULL)
                {
                    if (pView->GetMyItemModel(1) != NULL)
                    {
                        for (int i=0; i<g_stu2List.size(); i++)
                        {
                            StruInfo info;
                            info = g_stu2List.at(i);
                            if (!_isExist_seat(pView, info.noSeat))
                            {
                                QStandardItem * item = new QStandardItem;
                                QString str = info.name;
                                str += " - ";
                                str += info.noSeat;
                                item->setText(str);
                                if (pView->GetMyItemModel(msg.val1) != NULL)
                                {
                                   pView->setRowHeight(i, 30);
                                   if ( i*30 + 30 > pView->height())
                                   {
                                       //出现滚动条
                                       pView->setColumnWidth(0,  200);
                                       pView->setColumnWidth(1, 60);
                                   }
                                   QString s00 = "7777777777777777777777 add list -------    ";
                                   s00 += str;
                                   qDebug() << s00;
                                   pView->GetMyItemModel(msg.val1)->setItem(i, 0, item);
                                }
                            }
                        }
                    }
                }//if pview
            }//if list
        }
        break;
    case USER_MSG_CLASSINFO:
        {
//             FlodCloudWidget *pwin = (FlodCloudWidget *)g_mapObject["widget"];
//             if (pwin)
//             {
//                pwin->setvmclass(msg.str, msg.strval);
//             }
        }
        break;
    case USER_MSG_UPDATEHANDUP:
        {
            stulist *list = (stulist *)g_mapObject["stulist"];
            if (list != NULL)
            {
                CMytableview *pView = (CMytableview *)list->GetList();
                if (pView != NULL)
                {
                    if (msg.val1 == 0)
                    {
                        QString seat = msg.strval;
                        if (pView->GetMyItemModel(msg.val1) != NULL)
                        {
                            int nCount = pView->GetMyItemModel(msg.val1)->rowCount();
                            if (nCount > 0)
                            {
                                for (int i=0; i<nCount; i++)
                                {
                                    QStandardItem * item = pView->GetMyItemModel(msg.val1)->item(i, 0);
                                    if (item != NULL)
                                    {
                                        QString tmp = item->text();
                                        if (tmp.length() > 0 && !tmp.isEmpty())
                                        {
                                            int index = tmp.indexOf("-");
                                            QString seat_tmp = tmp.mid(index + 1);
                                            seat_tmp = seat_tmp.trimmed();
                                            qDebug() << seat_tmp;
                                            if (seat_tmp == seat)
                                            {
                                                pView->GetMyItemModel(msg.val1)->removeRow(i);
                                            }
                                        }
                                    }
                                }//for
                            }
                        }
                    }//handup
                }
            }
        }
        break;
      case USER_MSG_CLOSEDEMO:
        {
            CMytableview *myTableView = qobject_cast<CMytableview *>(msg.obj);
            if (myTableView != NULL)
            {
                int col = msg.val1;
                int row = msg.val2;
                if (row >=0 && g_stu2List.size() > 0)
                {
                    char sztmp[100] = {0};
                    sprintf(sztmp, "col = %d, row = %d .", col, row);
                    StruInfo info = g_stu2List.at(row);
                    QString url = g_config.protocol;
                    url += g_config.addr;
                    url += "/service/classes/specify_stu_show";
                    QString data;
                    data = "apId=";
                    data += info.apId;
                    data += "&";
                    data += "onOff=false";
                    myHttp *http = new myHttp;
                    if (http)
                    {
                        QString str = "stu list item select : ";
                        str += url;
                        str += "------";
                        str += data;
                        writeLogFile(QtDebugMsg, str);
                        qDebug()<< str;
//                        if (!http->Post(url, data))
//                        {
//                            writeLogFile(QtDebugMsg, "学生结束演示失败.");
//                        }
                        QString strTmp;
                        str = "specify_stu_show -------";
//                        http->GetData(strTmp);
                        str += strTmp;
                        writeLogFile(QtDebugMsg, str);
                        delete http;
                        http = NULL;
                    }
                    //MESSAGEBOX(sztmp, myTableView);
                }
            }
        }
        break;
    default:
        break;
    }
}
