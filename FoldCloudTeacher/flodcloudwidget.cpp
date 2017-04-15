#include "flodcloudwidget.h"
#include "ui_flodcloudwidget.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QNetworkInterface>
#include "global.h"
#include "streamparsexml.h"
#include "myhttp.h"
#include <QMutexLocker>
#include <QMutex>
#include <QTcpSocket>
#include "tcpserver.h"
#include "myjson.h"

#define  WIDGET_MAIN_HEIGHT   100
#define  WIDGET_MAIN_WIDTH    300

QMap<QString, QObject *> g_mapObject;
NetConfig g_config;
extern QMap<QString, QTcpSocket*> g_userlist;
extern QMutex  g_tcpMutex;
extern QList<class_template> m_list_classt;
extern QList<class_Rooms> m_list_rooms;

FlodCloudWidget::FlodCloudWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlodCloudWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect deskRect = desktopWidget->availableGeometry();
    setGeometry(QRect((deskRect.width() - width()) / 2, 4 - height(), width(), height()));
    setMouseTracking(true);
    connect(ui->btn_class, SIGNAL(clicked(bool)), this, SLOT(on_class_clicked()));
    connect(ui->btn_demostrate, SIGNAL(clicked(bool)), this, SLOT(on_demostrate_clicked()));
    connect(ui->btn_stuDemostrate, SIGNAL(clicked(bool)), this, SLOT(on_stuDemostrate_clicked()));
    ui->btn_demostrate->setText("开始演示");
    ui->btn_class->setText("选课");
    //读取网络配置文件
    StreamParseXml netxl;
    QString  str_net_config = QCoreApplication::applicationDirPath() + "/" + "netconfig.xml";
    netxl.readNetConfig(str_net_config, &g_config);
    m_pstulist = NULL;
    m_pstulist = new stulist(NULL);
    m_pstulist->setWidget(this);
    m_pthread = NULL;
    m_pthread = new Thread();
    m_pthread->start();
    m_pDataThread = NULL;
    m_pDataThread = new DataThread();
    connect(m_pDataThread, SIGNAL(NoticeShow(StruInfo)), this, SLOT(NoticeMsg(StruInfo)));
    m_pDataThread->start();
    g_mapObject["widget"] = this;
    g_mapObject["stulist"] = m_pstulist;
    g_mapObject["DataThread"] = m_pDataThread;
    //g_mylog.open();
    m_strMac = "00:00:00:00";//getHostMacAddress().toLower();
    m_pstulist->setmac(m_strMac);
    m_pstulist->setaddress(g_config.addr);
    m_bstart = false;
    m_pclassesForm = NULL;
    m_pclassesForm = new ClassesForm();
    connect(m_pclassesForm, SIGNAL(class_start()), this, SLOT(on_class_startrp()));
    //m_pTcpServer = NULL;
    //m_pTcpServer = new TcpServer();
    //m_tcpserverThrd = NULL;
    //m_tcpserverThrd = new tcpserverThread();
    //connect(m_tcpserverThrd, SIGNAL(tcp_msg()), this, SLOT(on_tcp_Msg()));
    //m_tcpserverThrd->start();
    //
    GetClassTemplate();
    QString  str_av = QCoreApplication::applicationDirPath() + "/" + "AVCapture.exe";
    m_myProcess = NULL;
    m_myProcess = new QProcess();
    m_myProcess->startDetached(str_av, QStringList(str_av));
    writeLogFile(QtDebugMsg, str_av);
    m_messageBox = new MessageBox();
    m_messageBox->hide();
}

FlodCloudWidget::~FlodCloudWidget()

{
    delete ui;
    if (m_pthread)
    {
        m_pthread->stop();
        m_pthread->wait();
        delete m_pthread;
        m_pthread = NULL;
    }
    if (m_pDataThread)
    {
        m_pDataThread->stop();
        m_pDataThread->wait();
        delete m_pDataThread;
        m_pDataThread = NULL;
    }
    if (m_tcpserverThrd)
    {
        m_tcpserverThrd->stop();
        m_tcpserverThrd->wait();
        delete m_tcpserverThrd;
        m_tcpserverThrd = NULL;
    }
    //if (NULL != m_pTcpServer)
    //{
    //    delete m_pTcpServer;
    //    m_pTcpServer = NULL;
    //}

    if (m_pstulist != NULL)
    {
        delete m_pstulist;
        m_pstulist = NULL;
    }
    if (m_pDataThread != NULL)
    {
        delete m_pDataThread;
        m_pDataThread = NULL;
    }
    if (m_pclassesForm != NULL)
    {
        delete m_pclassesForm;
        m_pclassesForm = NULL;
    }
    //g_mylog.close();
    if (m_myProcess)
    {
        m_myProcess->kill();
        delete m_myProcess;
        m_myProcess = NULL;
    }
    if (m_messageBox)
    {
        delete m_messageBox;
        m_messageBox = NULL;
    }
    qDebug() << "flodcloudwidget main over!";
}

void FlodCloudWidget::enterEvent(QEvent *)
{
    move(x(), 0);
}

void FlodCloudWidget::leaveEvent(QEvent *)
{
    move(x(), 4 - height());
}

void FlodCloudWidget::on_class_clicked()
{
    m_pclassesForm->show();
}

void FlodCloudWidget::on_demostrate_clicked()
{
    //MESSAGEBOX("this is demo.", this);
    int nsize = m_list_classt.size();
    if (nsize == 0)
        return;

    class_Rooms  classRoom = m_list_rooms.at(0);
    classRoom.name = g_config.sclass;
    QString url = g_config.protocol;
    url += g_config.addr;
    url += "/service/classes/show";
    QString data;
    data = "vmMac=";
    data += m_strMac;
    data += "&";
    data += "type=teacher";
    data += "&";
    data +="pcRoomName=";
    data += classRoom.name;
    data += "&";
    data +="pcXor=";
    data += "100000";
    data += "&";
    bool bDemoRunning = false;
    if (!m_bstart)
    {
        data += "onOff=true";
        m_bstart = true;
    }else
    {
        data += "onOff=false";
        m_bstart = false;
    }
    myHttp *http = new myHttp;
    if (http)
    {
        QString str = "demo clicked : ";
        str += url;
        str += "------";
        str += data;
        writeLogFile(QtDebugMsg, str);
        qDebug() << "show url : " << url;
        if (!http->Post(url, data))
        {
            writeLogFile(QtDebugMsg, "teacher demo failed.");
            delete http;
            http = NULL;
            return;
        }
        QString strTmp;
        str = "demo clicked -------";
        http->GetData(strTmp);
        str += strTmp;
        writeLogFile(QtDebugMsg, str);
        StreamParseXml parsexml;
        QString result;
        parsexml.readxmlComm(strTmp, "success", result, 0);
        delete http;
        http = NULL;
        if (result == "true" || result == "操作成功")
        {
            bDemoRunning = true;
        }
        if (bDemoRunning && m_bstart)
        {
            /*ui->btn_demostrate->setStyleSheet("QPushButton{border-image: url(:/images/demo_stop_nor.png);}"
                                          "QPushButton:hover{border-image: url(:/images/demo_stop_nor.png);}"
                                        "QPushButton:pressed{border-image: url(:/images/demo_stop_press.png);}");*/
            ui->btn_demostrate->setText("结束演示");
        }
        if (bDemoRunning && !m_bstart)
        {
            /*ui->btn_demostrate->setStyleSheet("QPushButton{border-image: url(:/images/demo_nor.png);}"
                                         "QPushButton:hover{border-image: url(:/images/demo_press.png);}"
                                        "QPushButton:pressed{border-image: url(:/images/demo_nor.png);}");*/
            ui->btn_demostrate->setText("开始演示");

        }
    }
}

void FlodCloudWidget::on_stuDemostrate_clicked()
{
    m_pstulist->settype(1);
    m_pstulist->show();
}

QString FlodCloudWidget::getHostMacAddress()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }
    return strMacAddr;
}


void FlodCloudWidget::on_tcp_Msg()
{
    if (!g_userlist.isEmpty())
    {
        QMap<QString, QTcpSocket*>::iterator iter;
        for (iter = g_userlist.begin(); iter != g_userlist.end(); ++iter)
        {
            qDebug() << iter.key();
            QTcpSocket *connect = (QTcpSocket *)iter.value();
            if (NULL != connect)
            {
                if (connect->isValid())
                {
                    char buf[512] = {0};
                    strcpy(buf, "client connect server success, @@@@@@@!");
                    //发送数据到eclass_client
                    char data[1024] = {0};
                    strcpy(data, "222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222");
                    media_header  mediaHeader;
                    strcpy(mediaHeader.magic, "BEAF");
                    mediaHeader.type = 1;
                    mediaHeader.width = 1920;
                    mediaHeader.height = 1080;
                    mediaHeader.serial = 1;
                    mediaHeader.cursor_x = 20;
                    mediaHeader.cursor_y = 30;
                    mediaHeader.time_stamp = 0;
                    mediaHeader.size = strlen(data);
                    mediaHeader.checksum = 7;
                    //m_pTcpServer->send_data(connect, (char*)&mediaHeader, sizeof(media_header));
                    //m_pTcpServer->send_data(connect, data, mediaHeader.size);
                }
            }
        }
    }
    qDebug() << "on_tcp_msg";
}

void FlodCloudWidget::on_class_startrp()
{
    qDebug() << "this is on_class_startrp";
//    mytcpclient tcpclient;
//    tcpclient.setHostAddress(QHostAddress("192.168.0.199"), 8080);
//    tcpclient.newConnect();
//    char szbuf[100] = {0};
//    strcpy(szbuf, "my is a client");
//    tcpclient.send_data(szbuf, sizeof(szbuf));
//    qDebug() << "send finish.";

    /*******************************************************/
        //class code
    /*******************************************************/
    class_sel  class_sel_t;
    int nsize = m_list_rooms.size();
    if (nsize == 0)
        return;
    m_pclassesForm->GetSelClassid(class_sel_t);
    class_Rooms  classRoom = m_list_rooms.at(0);
    classRoom.name = g_config.sclass;
    QString url = g_config.protocol;
    url += g_config.addr;
    url += "/service/classes/switch_stu_template?";
    url += "templateId=";
    url += class_sel_t.id;
    url += "&";
    url += "roomId=";
    url += classRoom.id;
    qDebug() << "start class : " << url;
    myHttp *http = new myHttp("json");
    if (http)
    {
        QString str;
        str = "start class request: ";
        str += url;
        writeLogFile(QtDebugMsg, str);
        if (!http->Get(url))
        {
            writeLogFile(QtDebugMsg, "start class request failed.");
            delete http;
            http = NULL;
            return;
        }
        QString strTmp;
        str = "start class request -------";
        http->GetData(strTmp);
        //strTmp = "{\"success\":false,\"data\":[{\"code\":\"CLASSES_OPERATIONS-001\",\"message\":\"下一课\"}]}";
        str += strTmp;
        delete http;
        http = NULL;
        writeLogFile(QtDebugMsg, str);
//        MyJson mjson;
//        QString strMsg;
//        bool bRet = mjson.GetRespose(strTmp, strMsg);
//        m_pclassesForm->SetClassState(bRet, class_sel_t);
        MyJson mjson;
        QString strMsg;
        bool bRet = mjson.GetRespose(strTmp, strMsg);
        if (!bRet)
        {
            if (strMsg == "")
                strMsg = "消息为空!";
            m_messageBox->SetText(strMsg);
            m_messageBox->show();
            m_pclassesForm->SetClassState(bRet, class_sel_t);
        }
    }//if

//    /*****************************************************/
//    bool bclassFlag = m_pclassesForm->GetClassFlag();
//    if (!bclassFlag)
//    {
//        url += "/service/classes/switch_stu_template?";
//        url += "templateId=";
//        url += class_sel_t.id;
//        url += "&";
//        url += "roomId=";
//        url += classRoom.id;
//        qDebug() << "start class : " << url;
//    }else
//    {
//        url += "/service/classes/over?";
//        url += "roomName=";
//        url += classRoom.name;
//        qDebug() << "Over class : " << url;
//    }
//    myHttp *http = new myHttp("json");
//    if (http)
//    {
//        QString str;
//        if (!bclassFlag)
//            str = "start class request: ";
//        else
//            str = "over class request : ";
//        str += url;
//        writeLogFile(QtDebugMsg, str);
//        if (!http->Get(url))
//        {
//            if (!bclassFlag)
//                writeLogFile(QtDebugMsg, "start class request failed.");
//            else
//                writeLogFile(QtDebugMsg, "over class request failed.");
//            delete http;
//            http = NULL;
//            return;
//        }
//        QString strTmp;
//        if (!bclassFlag)
//            str = "start class request -------";
//        else
//            str = "over class request -------";
//        http->GetData(strTmp);
//        str += strTmp;
//        delete http;
//        http = NULL;
//        writeLogFile(QtDebugMsg, str);
//        MyJson mjson;
//        bool bRet = mjson.GetRespose(strTmp);
//        m_pclassesForm->SetClassState(bRet, class_sel_t);
//    }
}

void FlodCloudWidget::GetClassTemplate()
{
    QString url = g_config.protocol;
    url += g_config.addr;
    url += "/service/templates/list?qp.page=1&qp.max=1000";
    MyJson mjson;
    myHttp *http = new myHttp("json");
    if (http)
    {
        QString str = "Get class template : ";
        str += url;
        writeLogFile(QtDebugMsg, str);
        if (!http->Get(url))
        {
            writeLogFile(QtDebugMsg, "Get class template failed.");
            delete http;
            http = NULL;
            return;
        }
        QString strTmp;
        str = "Get class template -------";
        http->GetData(strTmp);
        str += strTmp;
        delete http;
        http = NULL;
        writeLogFile(QtDebugMsg, str);
        mjson.GetClassTemplate(strTmp, m_list_classt);
        m_pclassesForm->initclasst();
    }
    //获取教室id
    http = new myHttp("json");
    url = "";
    url = g_config.protocol;
    url += g_config.addr;
    url += "/service/rooms/list";
    if (http)
    {
        QString str = "Get Room id : ";
        str += url;
        writeLogFile(QtDebugMsg, str);
        if (!http->Get(url))
        {
            writeLogFile(QtDebugMsg, "Get Room id failed.");
            delete http;
            http = NULL;
            return;
        }
        QString strTmp;
        str = "Get Room id -------";
        http->GetData(strTmp);
        delete http;
        http = NULL;
        str += strTmp;
        writeLogFile(QtDebugMsg, str);
        mjson.GetRoomId(strTmp, m_list_rooms);
    }
}

