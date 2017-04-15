#include "myjson.h"
#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>

MyJson::MyJson(QObject *parent) : QObject(parent)
{

}

void MyJson::GetClassTemplate(const QString jsonData, QList<class_template>& list_class)
{
    if (jsonData.length() == 0)
        return;
    QJsonParseError json_error;
    QByteArray byteData = jsonData.toLatin1();
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byteData, &json_error);
    if (!parse_doucment.isNull() && json_error.error == QJsonParseError::NoError)
    {
        if (parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("success"))
            {
                QJsonValue value = obj.take("success");
                if (value.isBool())
                {
                    if (value.toBool() == true)
                    {
                        if (obj.contains("data"))
                        {
                            value = obj.take("data");
                            if (value.isObject())
                            {
                                obj = value.toObject();
                                if (obj.contains("list"))
                                {
                                    value = obj.take("list");
                                    if (value.isArray())
                                    {
                                        QJsonArray jsonArray = value.toArray();
                                        int size = jsonArray.size();
                                        for (int i=0; i < size; i++)
                                        {
                                            class_template  class_t;
                                            value = jsonArray.at(i);
                                            if (value.isObject())
                                            {
                                                QJsonValue vv;
                                                QJsonObject obj = value.toObject();
                                                if (obj.contains("id"))
                                                {
                                                    vv = obj.take("id");
                                                    if (vv.isString())
                                                        class_t.id = vv.toString();
                                                }
                                                if (obj.contains("name"))
                                                {
                                                    vv = obj.take("name");
                                                    if (vv.isString())
                                                        class_t.name = vv.toString();
                                                }
                                            }
                                            list_class.append(class_t);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MyJson::GetRoomId(const QString jsonData, QList<class_Rooms>& list_rooms)
{
    if (jsonData.length() == 0)
        return;
    QJsonParseError json_error;
    QByteArray byteData = jsonData.toLatin1();
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byteData, &json_error);
    if (!parse_doucment.isNull() && json_error.error == QJsonParseError::NoError)
    {
        if (parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("success"))
            {
                QJsonValue value = obj.take("success");
                if (value.isBool())
                {
                    if (value.toBool() == true)
                    {
                        if (obj.contains("data"))
                        {
                            value = obj.take("data");
                            if (value.isArray())
                            {
                                QJsonArray jsonArray = value.toArray();
                                int size = jsonArray.size();
                                for (int i=0; i < size; i++)
                                {
                                    class_Rooms  room_t;
                                    value = jsonArray.at(i);
                                    if (value.isObject())
                                    {
                                         obj = value.toObject();
                                         if (obj.contains("id"))
                                         {
                                             room_t.id = obj.take("id").toString();
                                         }
                                         if (obj.contains("name"))
                                         {
                                             room_t.name = obj.take("name").toString();
                                         }
                                    }
                                    list_rooms.append(room_t);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


bool MyJson::GetRespose(const QString jsonData, QString& strMsg)
{
    if (jsonData.length() == 0)
        return false;
    QJsonParseError json_error;
    QByteArray byteData = jsonData.toUtf8();
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byteData, &json_error);
    if (!parse_doucment.isNull() && json_error.error == QJsonParseError::NoError)
    {
        if (parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if (obj.contains("success"))
            {
                QJsonValue value = obj.take("success");
                if (obj.contains("data"))
                {
                    value = obj.take("data");
                    if (value.isArray())
                    {
                        QJsonArray jsonArray = value.toArray();
                        int size = jsonArray.size();
                        for (int i=0; i < size; i++)
                        {
                            value = jsonArray.at(i);
                            if (value.isObject())
                            {
                                 obj = value.toObject();
                                 if (obj.contains("message"))
                                 {
                                     QByteArray bb =  obj.take("message").toString().toUtf8();
                                     strMsg = QString::fromUtf8(bb);
                                     qDebug() << "message :" << strMsg;
                                 }
                            }

                        }
                    }
                }
                return value.toBool();
            }
        }
    }
    return true;
}
