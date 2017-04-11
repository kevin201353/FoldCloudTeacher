#ifndef STREAMPARSEXML_H
#define STREAMPARSEXML_H
#include <QXmlStreamReader>
#include <QObject>
#include "global.h"
#include <QMap>

class StreamParseXml
{
public:
    StreamParseXml();
    ~StreamParseXml();
    int readNetConfig(const QString filename, NetConfig *pconfig);
    int readxmlclass(const QString xmlData);
    int readxmlComm(const QString xmlData, const QString szKey, QString& value, int nType);
    int readXmlstuinfo(const QString xmlData);
    inline void setType(QMap<QString, int> type){
        m_ntype = type;
    };
    void noticeMsgWindow(StruInfo info);
    inline void setStop(bool bstop){
        m_stop = bstop;
    };
private:
    void parseUserInformation();
    QString getValue(const QString &name);
    QString getAttribute(const QString &name);
private:
    QXmlStreamReader *reader;
    QMap<QString, int> m_ntype;  //0: 举手  1：学生
    bool  m_stop;
};

#endif // STREAMPARSEXML_H
