#include <QDebug>
#include "streamparsexml.h"
#include <QFile>
#include <QMessageBox>
#include "global.h"
#include "thread.h"
#include <QMutex>

//<?xml version="1.0" encoding="UTF-8">
//<COMMAND>
//    <OBJECT>USER</OBJECT>
//    <ACTION>LOGIN</ACTION>
//    <DATA>
//        <USER NAME="root" PASSWORD="123456" />
//    </DATA>
//</COMMAND>

QList<StruInfo> g_stu2List;
QList<StruInfo> g_handupList;
QList<StruInfo> g_stuTmp;
StreamParseXml::StreamParseXml()
{
    m_stop = false;
}

StreamParseXml::~StreamParseXml()
{

}

int StreamParseXml::readxmlclass(const QString xmlData)
{
    reader = new QXmlStreamReader(xmlData);
    while(!reader->atEnd() && !reader->hasError())
    {
        if (m_stop)
           break;
        QXmlStreamReader::TokenType token = reader->readNext();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        if (reader->isStartElement() && reader->name() == "data")
        {
            QString className = getValue("className");
            QString desktopName = getValue("desktopName");
            QString roomName = getValue("roomName");
            StruInfo info;
            info.apId = desktopName;
            info.name = roomName;
            info.noSeat = className;
            ReportMsg msg;
            msg.action = USER_MSG_CLASSINFO;
            msg.str = info.apId;  //桌面名称
            msg.strval = info.noSeat; //班级
            call_msg_back(msg_respose, msg);
        }
    }
    if (reader->hasError())
        qDebug() << reader->errorString();
    reader->clear();
    delete reader;
    reader = NULL;
    return 0;
}

int StreamParseXml::readNetConfig(const QString filename, NetConfig *pconfig)
{
    if(filename.isEmpty())
        return -1;
    if (pconfig == NULL)
        return -1;
    QFile *file = new QFile(filename);
    if(!file->open(QFile::ReadOnly | QFile::Text))
    {
       QMessageBox::information(NULL, QString("Net Config"), QString("open error!"));
       return -1;
    }
    reader = new QXmlStreamReader(file);
    while(!reader->atEnd() && !reader->hasError())
    {
        QXmlStreamReader::TokenType token = reader->readNext();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        if (reader->isStartElement() && reader->name() == "net")
        {
            pconfig->protocol = getValue("protocol");
            pconfig->addr = getValue("addr");
            pconfig->port = getValue("port");
            pconfig->sclass = getValue("class");
        }
    }
    if (reader->hasError()) {
       qDebug() << reader->errorString();
       //QMessageBox::information(NULL, QString("parseXML"), reader->errorString());
    }
    file->close();
    delete file;
    reader->clear();
    delete reader;
    reader = NULL;
    return 0;
}

void StreamParseXml::parseUserInformation()
{
    QString elementString = getValue("ACTION");
    if (elementString == "LOGIN")
    {
        while(!reader->atEnd())
        {
            reader->readNext();
            if (reader->name() == "USER")
            {
                QXmlStreamAttributes attributes = reader->attributes();
                if(attributes.hasAttribute("NAME"))
                    qDebug() << "USER=" << attributes.value("NAME").toString();
                if(attributes.hasAttribute("PASSWORD"))
                    qDebug() << "PASSWORD=" << attributes.value("PASSWORD").toString();
            }
        }
    }
}

QString StreamParseXml::getValue(const QString &name)
{
    while(!reader->atEnd())
    {
        if (m_stop)
            break;
        reader->readNext();
        if (reader->isStartElement() && reader->name() == name)
        {
            //qDebug() << reader->readElementText();
            return reader->readElementText();
        }
    }
    return "";
}

void StreamParseXml::noticeMsgWindow(StruInfo info)
{
    //ReportMsg msg;
    if (m_ntype["stu"] == 0 || m_ntype["stu"] == 1)
    {
        if (m_ntype["stu"] == 0)
        {
            StruInfo *pinfo = new StruInfo;
            *pinfo = info;
            DataThread *pthread = (DataThread *)g_mapObject["DataThread"];
            emit pthread->NoticeShow(pinfo);
        }
    }
}

int StreamParseXml::readxmlComm(const QString xmlData, const QString szKey, QString& value, int nType)
{
    reader = new QXmlStreamReader(xmlData);
    while(!reader->atEnd() && !reader->hasError())
    {
        if (m_stop)
           break;
        QXmlStreamReader::TokenType token = reader->readNext();
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        if (reader->isStartElement() && reader->name() == "data")
        {
            if (nType == 0)
            {
                value = getValue(szKey);
                break;
            }
        }
    }
    if (reader->hasError())
        qDebug() << reader->errorString();
    reader->clear();
    delete reader;
    reader = NULL;
    return 0;
}

int StreamParseXml::readXmlstuinfo(const QString xmlData)
{
    reader = new QXmlStreamReader(xmlData);
    if (reader == NULL)
        return 0;
    ReportMsg msg;
    StruInfo info;
    g_stu2List.clear();
    while(!reader->atEnd())
    {
        if (m_ntype["stu"] == 1)
        {
            if (reader->isStartElement())
            {
                if (reader->name() == "apId")
                {
                    info.apId = reader->readElementText();
                }
                if (reader->name() == "id")
                {
                    info.id = reader->readElementText();
                }
                if (reader->name() == "dSeat")
                {
                    info.noSeat = reader->readElementText();
                }
                if (reader->name() == "dStuName")
                {
                    info.name = reader->readElementText();
                }

            }else if (reader->isEndElement() && reader->name() == "desktopRoomAps")
            {
                bool insert = 1;
                for (int i=0; i< g_stu2List.size(); i++)
                {
                    StruInfo infotmp = g_stu2List.at(i);
                    if (infotmp.id == info.id)
                    {
                        QString strTmp2 = "xxxxxxxxxx ---  ";
                        strTmp2 += infotmp.id;
                        qDebug() << strTmp2;
                        insert = 0;
                        break;
                    }
                }
                if (insert == 1)
                {
                    StruInfo info2;
                    info2 = info;
                    g_stu2List.append(info2);
                    QString strTp;
                    strTp+= info2.id;
                    strTp += "  ";
                    strTp += "name=";
                    strTp += info2.name;
                    strTp += "  ";
                    strTp += "noSeat=";
                    strTp += info2.noSeat;
                    strTp += "  ";
                    strTp += "apId=";
                    strTp += "  ";
                    strTp += info2.apId;
                    qDebug() << strTp;
                }
            }
        }//if mtype == 1
        if (m_ntype["stu"] == 0)
        {
            if (reader->isStartElement())
            {
                if (reader->name() == "enableHandUp")
                {
                    info.handup = reader->readElementText();
                }
                if (reader->name() == "id")
                {
                    info.id = reader->readElementText();
                }
                if (reader->name() == "name")
                {
                    info.name = reader->readElementText();
                }
                if (reader->name() == "seat")
                {
                    info.noSeat = reader->readElementText();
                }
            }if (reader->isEndElement() && reader->name() == "studentHandUpStatus")
            {
                if (info.handup == "true")
                {
                    if (!info.id.isEmpty())
                    {
                        bool insert = 1;
                        for (int i=0; i< g_handupList.size(); i++)
                        {
                            StruInfo infotmp = g_handupList.at(i);
                            if (infotmp.id == info.id)
                            {
                                insert = 0;
                                break;
                            }
                        }
                        if (insert == 1)
                        {
                            StruInfo info2;
                            info2 = info;
                            g_handupList.append(info2);
                            noticeMsgWindow(info2);
                        }
                    }
                }//handup
                else if (info.handup == "false")
                {
                    for (int i=0; i< g_handupList.size(); i++)
                    {
                        StruInfo infotmp = g_handupList.at(i);
                        if (infotmp.id == info.id)
                        {
                            g_handupList.removeAt(i);
                            ReportMsg msg;
                            msg.action = USER_MSG_UPDATEHANDUP;
                            msg.val1 = m_ntype["stu"];
                            msg.strval = infotmp.noSeat;
                            call_msg_back(msg_respose, msg);
                            break;
                        }
                    }
                }
            }
        }
        reader->readNext();
    }
    msg.action = USER_MSG_DEALSTULIST;
    msg.val1 = m_ntype["stu"];
    call_msg_back(msg_respose, msg);
    reader->clear();
    delete reader;
    reader = NULL;
    return 0;
}
