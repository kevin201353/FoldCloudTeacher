#ifndef CMYTABLEVIEW_H
#define CMYTABLEVIEW_H
#include <QTableView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QItemDelegate>
#include <QPixmap>
#include <QStandardItemModel>
#include "global.h"
#include <QList>

class MyStuDelegate : public QItemDelegate
{
public:
    MyStuDelegate(QObject *parent = 0);
    virtual ~MyStuDelegate();
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void GetDemoPixSize(struct XSize *size);
    void SetDemoPixSize(int x, int y, int width, int height) const;
    void SetType(int type);
    inline int GetType() const {
        return m_ntype;
    };
    void setpress(bool bpres, int row);
    void setRect0(QRect rect){
        m_rect0 = rect;
    };
private:
    QPixmap favouritePixmap[3];
    QPixmap demoPixmap[2];
    struct XSize  *m_pdemoSize;
    int  m_ntype;
    bool  m_bpress;
    int   m_row;
    QRect  m_rect0;
};

class MyStandardItemModel : public QStandardItemModel
{
public:
    MyStandardItemModel(QObject *parent = 0);
    virtual ~MyStandardItemModel();
public:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const ;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
};

class CMytableview : public QTableView
{
    Q_OBJECT

public:
    explicit CMytableview(QWidget *parent = 0);
    ~CMytableview();

public:
    MyStandardItemModel * GetMyItemModel(int ntype)
    {
        return ntype==1?model:modelhandup;
    }

    void SetType(int type);
    void setCount(int count);
    int getCount() const {
        return ncount;
    };
    void _setCellWidget();
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    MyStuDelegate * delegate;
    MyStandardItemModel * model;
    MyStandardItemModel * modelhandup;
    int ncount;
};
#endif // CMYTABLEVIEW_H
