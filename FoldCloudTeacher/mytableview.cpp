#include "cmytableview.h"
#include <QPoint>
#include <QModelIndex>
#include <QRect>
#include <QPainter>
#include <QStyleOptionViewItem>
#include "global.h"
#include "log.h"
#include <QScrollBar>
#include <QRect>
#include <QString>

char szTmp[100] = {0};
struct ReportMsg g_Msg = {NULL, 0, 0, 0, "", ""};
MyStuDelegate::MyStuDelegate(QObject *parent):
    QItemDelegate(parent)
{
    favouritePixmap[0] = QPixmap(":/images/handed_nor.png");
    favouritePixmap[1] = QPixmap(":/images/list_demo.png");
    favouritePixmap[2] = QPixmap(":/images/list_demo_press.png");
    m_pdemoSize = NULL;
    m_pdemoSize = new struct XSize;
    m_ntype = 0;
}

MyStuDelegate::~MyStuDelegate()
{
    if (m_pdemoSize != NULL)
    {
        delete m_pdemoSize;
        m_pdemoSize = NULL;
    }
}

void MyStuDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == 0){
       QItemDelegate::paint(painter,option,index);
       return;
    }
    if (index.column() == 1)
    {
        //sprintf(szTmp, "m_ntype = %d.\n", m_ntype);
        //writeLogFile(QtDebugMsg, szTmp);
        QRect rect = option.rect;
        int x = rect.x() + rect.width()/2 - favouritePixmap[m_ntype].width()/2;
        int y = rect.y() + rect.height()/2 - favouritePixmap[m_ntype].height()/2;
        SetDemoPixSize(x, y, favouritePixmap[m_ntype].width(), favouritePixmap[m_ntype].height());
        QPixmap pixmap = ( m_bpress && (index.row() == m_row) && m_ntype == 1 )? favouritePixmap[2]:favouritePixmap[m_ntype];
        painter->drawPixmap(x, y, pixmap);
    }
}

void MyStuDelegate::SetType(int type)
{
    m_ntype = type;
}

void MyStuDelegate::GetDemoPixSize(struct XSize *psize)
{
    if (psize != NULL)
    {
        *psize = *m_pdemoSize;
    }
}

void MyStuDelegate::SetDemoPixSize(int x, int y, int width, int height) const
{
     //sprintf(szTmp, "x = %d, y = %d, width = %d, height = %d .\n", x, y, width, height);
     //writeLogFile(QtDebugMsg, szTmp);
     m_pdemoSize->x = x;
     m_pdemoSize->y = y;
     m_pdemoSize->width = width;
     m_pdemoSize->height = height;
}

void MyStuDelegate::setpress(bool bpres, int row)
{
    m_bpress = bpres;
    m_row = row;
}
/*****************************************************************************************/
//MyStandardItemModel
MyStandardItemModel::MyStandardItemModel(QObject *parent):
    QStandardItemModel(parent)
{

}
MyStandardItemModel::~MyStandardItemModel()
{

}

QVariant MyStandardItemModel::data(const QModelIndex &index, int role) const
{
//     int column = index.column();
//    if(role == Qt::DisplayRole && column == 0)
//    {
//        return tr("stu");
//    }
//    if (role == Qt::DecorationRole && column == 1)
//    {
//        return QIcon(":/images/list_demo_press.png");
//    }
//    if(role == Qt::ToolTipRole && column == 0)
//        return tr("love");
    return QStandardItemModel::data(index,role);
}

QVariant MyStandardItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section == 1 && orientation == Qt::Horizontal){
       if(role == Qt::DecorationRole)
           return QIcon(":/images/list_demo.png");
       if(role == Qt::DisplayRole)
           return "";
       if(role == Qt::ToolTipRole)
           return tr("love");
    }
    return QStandardItemModel::headerData(section, orientation, role);
}
/*************************************************************************************************/
//CMytableview
CMytableview::CMytableview(QWidget *parent):
    QTableView(parent)
{
    delegate = NULL;
    model = NULL;
    modelhandup = NULL;
    delegate = new MyStuDelegate;
    model = new MyStandardItemModel;
    modelhandup = new MyStandardItemModel;
    //model->setRowCount(6);
    model->setColumnCount(2);
    //modelhandup->setRowCount(6);
    modelhandup->setColumnCount(2);
    ncount = 0;
    this->setItemDelegate(delegate);
    this->resizeColumnsToContents();
    this->resizeRowsToContents();
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setMouseTracking(true);//important
    this->setColumnWidth(0,  522);
    this->setColumnWidth(1, 60);
}

CMytableview::~CMytableview()
{
    if (delegate)
    {
        delete delegate;
        delegate = NULL;
    }
    if (model)
    {
        delete model;
        model = NULL;
    }
    if (modelhandup)
    {
        delete modelhandup;
        modelhandup = NULL;
    }
}

void CMytableview::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint &p = event->pos();
    QModelIndex modelIndex = indexAt(p);
    int row = 0;
    int col = 0;
    if (modelIndex.isValid())
    {
        col = modelIndex.column();
        row = modelIndex.row();
        if (col == 1)
        {
            if (delegate != NULL)
            {
                delegate->setpress(true, row);
            }
        }
    }
   QTableView::mouseMoveEvent(event);
}

void CMytableview::mousePressEvent(QMouseEvent *event)
{
    QTableView::mousePressEvent(event);
}

void CMytableview::mouseReleaseEvent(QMouseEvent *event)
{
    if (delegate == NULL)
        return;

    if (delegate->GetType() == 1)
    {
        const QPoint &p = event->pos();
        QModelIndex modelIndex = indexAt(p);
        int col = 0;
        int row = 0;
        if (modelIndex.isValid())
        {
            col = modelIndex.column();
            row = modelIndex.row();
            if (col == 1)
            {
                struct XSize size;
                delegate->GetDemoPixSize(&size);
                //sprintf(szTmp, "size.x = %d, size.y = %d, size.width = %d, size.height = %d .\n", size.x, size.y, size.width, size.height);
                //writeLogFile(QtDebugMsg, szTmp);
                if ( (p.x() >= size.x - 2 && p.x() <= size.x - 2 + size.width) &&
                     (p.y() >= row * 30 + 4 && p.y() <= row * 30 + size.height - 4 ))
                {
                    g_Msg.obj = qobject_cast<QObject *>(this);
                    g_Msg.action = USER_MSG_TABLEVIEW;
                    g_Msg.val1 = col;
                    g_Msg.val2 = row;
                    call_msg_back(msg_respose, g_Msg);
                }
            }//if col
        }
    }//if ( delegate->m_ntype == 1 )
    QTableView::mouseReleaseEvent(event);
}

void CMytableview::SetType(int type)
{
    delegate->SetType(type);
    if (type == 1)
    {
        model->removeRows(0, model->rowCount());
        setModel(model);
    }
    if (type == 0)
    {
        modelhandup->removeRows(0, modelhandup->rowCount());
        setModel(modelhandup);
    }
}

void CMytableview::setCount(int count)
{
    ncount = count;
}
