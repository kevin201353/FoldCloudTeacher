#include "myqlist.h"

MyQList::MyQList(QObject *parent) : QObject(parent)
{

}

void MyQList::append(StruInfo* info)
{
    m_mutex.lock();
    m_myList.append(info);
    m_mutex.unlock();
}

void MyQList::remove(QString id)
{
    for (int i = 0; i< m_myList.size(); i++)
    {
        m_mutex.lock();
        StruInfo *info = m_myList.at(i);
        m_mutex.unlock();
        if (info != NULL)
        {
            if (info->id == id)
            {
                m_mutex.lock();
                m_myList.removeAt(i);
                if (info != NULL)
                {
                    delete info;
                    info = NULL;
                }
                m_mutex.unlock();
            }
        }
    }
}

int  MyQList::getsize() const
{
    return m_myList.size();
}

StruInfo* MyQList::at(int ndex) const
{
    return m_myList.at(ndex);
}

void MyQList::removeAll()
{
    for (int i=0; i<m_myList.size(); i++)
    {
        m_mutex.lock();
        StruInfo *info = m_myList.at(i);
        if (info != NULL)
        {
            delete info;
            info = NULL;
        }
        m_myList.clear();
        m_mutex.unlock();
    }
}
