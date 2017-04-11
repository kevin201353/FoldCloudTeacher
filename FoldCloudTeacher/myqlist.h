#ifndef MYQLIST_H
#define MYQLIST_H

#include <QObject>
#include <QList>
#include "global.h"
#include <QMutex>

class MyQList : public QObject
{
    Q_OBJECT
public:
    explicit MyQList(QObject *parent = 0);
public:
    void append(StruInfo* info);
    void remove(QString id);
    int  getsize() const;
    StruInfo* at(int ndex) const;
    void removeAll();
signals:
public slots:
private:
    QList<StruInfo *> m_myList;
    QMutex  m_mutex;
};

#endif // MYQLIST_H
