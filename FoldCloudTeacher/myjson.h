#ifndef MYJSON_H
#define MYJSON_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QList>
#include "global.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class MyJson : public QObject
{
    Q_OBJECT
public:
    explicit MyJson(QObject *parent = 0);

signals:
public slots:
public:
    void GetClassTemplate(const QString jsonData, QList<class_template>& list_class);
    void GetRoomId(const QString jsonData, QList<class_Rooms>& list_rooms);
    bool GetRespose(const QString jsonData, QString& strMsg);  //true or false
};

#endif // MYJSON_H
