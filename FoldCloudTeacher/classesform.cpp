#include "classesform.h"
#include "ui_classesform.h"
#include <QMouseEvent>
#include <QList>
#include <QFont>

QList<class_template> m_list_classt;
QList<class_Rooms> m_list_rooms;

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
}

ClickableLabel::~ClickableLabel()
{

}
void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
    QLabel::mousePressEvent(event);//将该事件传给父类处理
}

ClassesForm::ClassesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClassesForm)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ui->btnStuClose->setStyleSheet("QPushButton{border-image: url(:/images/x.png);}"
                            "QPushButton:pressed{border-image: url(:/images/x.png);}");
    connect(ui->btnStuClose, SIGNAL(clicked(bool)), this, SLOT(exit_widget()));
    connect(ui->btn_left, SIGNAL(clicked(bool)), this, SLOT(on_left()));
    connect(ui->btn_right, SIGNAL(clicked(bool)), this, SLOT(on_right()));
    connect(ui->btn_class, SIGNAL(clicked(bool)), this, SLOT(on_class()));
    //
    connect(ui->label_class1, SIGNAL(clicked()), this, SLOT(on_label_class1()));
    connect(ui->label_class2, SIGNAL(clicked()), this, SLOT(on_label_class2()));
    connect(ui->label_class3, SIGNAL(clicked()), this, SLOT(on_label_class3()));
    ui->btn_class->setText("上课");
    ui->btn_class->setEnabled(false);
    ui->btn_class->setVisible(false);
    setMouseTracking(true);
    m_moving = false;
    m_labelClass[0] = ui->label_class1;
    m_labelClass[1] = ui->label_class2;
    m_labelClass[2] = ui->label_class3;
    m_direction = 0;
    m_firtindex = 0;
    QFont serifFont("Times", 14, QFont::Bold);
    for (int i=0; i<3; i++)
    {
        m_labelClass[i]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
        m_labelClass[i]->setText("");
        m_labelClass[i]->setAlignment(Qt::AlignCenter);
        m_labelClass[i]->setFont(serifFont);
    }
    m_bclass = false;
}

ClassesForm::~ClassesForm()
{
    delete ui;
}

void ClassesForm::exit_widget()
{
    hide();
}

void ClassesForm::mouseMoveEvent(QMouseEvent *event)
{
    if( event->buttons().testFlag(Qt::LeftButton) && m_moving)
    {
      this->move(this->pos() + (event->globalPos() - m_LastMousePosition));
      m_LastMousePosition = event->globalPos();
    }
}

void ClassesForm::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && event->pos().y() <= 31)
    {
        m_moving = true;
        m_LastMousePosition = event->globalPos();
    }
}

void ClassesForm::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_moving = false;
    }
}

void ClassesForm::on_left()
{
    m_direction = -1;
    initclasst();
}

void ClassesForm::on_right()
{
    m_direction = 1;
    initclasst();
}

void ClassesForm::on_class()
{
    //emit class_start();
}


int g_ndex = 0;
void ClassesForm::initclasst()
{
    int size = m_list_classt.size();
    if (size == 0)
        return;
    class_template classt;
    if (g_ndex <= 0)
        g_ndex = 0;
    int dex0 = (3*g_ndex + 0);
    int dex1 = (3*g_ndex + 1);
    int dex2 = (3*g_ndex + 2);
    if (dex0 >=0 && dex0 < size )
    {
        classt = m_list_classt.at(dex0);
        m_labelClass[0]->setText(classt.name);
    }
    if (dex1 >=0 && dex1 < size)
    {
         classt = m_list_classt.at(dex1);
        m_labelClass[1]->setText(classt.name);
    }
    if (dex2 >=0 && dex2 < size)
    {
        classt = m_list_classt.at(dex2);
        m_labelClass[2]->setText(classt.name);
    }
    if (dex2 >= size -1)
    {
        int dex = dex2%3;
        if (dex < 3)
            m_labelClass[2]->setText("");
    }
    if (m_direction == 0 || m_direction == 1)
    {
        if (dex2 < size -1)
            g_ndex++;
    }else
    {
        if (dex0 >= 1)
            g_ndex--;
    }


//    classt = m_list_classt.at(g_ndex);
//    for (int i=0; i<3; i++)
//    {
//       g_ndex = (3*i + i);
//       if (g_ndex < size)
//       {
//           classt = m_list_classt.at(g_ndex);
//            m_labelClass[i]->setText(classt.name);
//       }
//    }
//    if (m_direction == 0)
//    {
//        for (int i=0; i<size; i++)
//        {
//            if (i < 3)
//            {
//                classt = m_list_classt.at(i);
//                m_labelClass[i]->setText(classt.name);
//            }
//        }
//    }
//    if (m_direction == -1)
//    {
//        if (size <= 3)
//            return;
//        int index = size/3;
//        for (int i=0; i<3; i++)
//        {
//           m_firtindex += (index * 3 + i) ;
//           if (m_firtindex < size)
//           {
//                classt = m_list_classt.at(m_firtindex);
//                m_labelClass[i]->setText(classt.name);
//           }
//        }
//    }
//    if (m_direction == 1)
//    {
//        if (size <= 3)
//            return;
//        int index = size/3;
//        for (int i=0; i<3; i++)
//        {
//            m_firtindex -= (index *3 + i);
//            if (m_firtindex < 0)
//                m_firtindex = 0;
//            if (m_firtindex >= size)
//                m_firtindex = size - 1;
//            if (m_firtindex < size)
//            {
//                classt = m_list_classt.at(m_firtindex);
//                m_labelClass[i]->setText(classt.name);
//            }
//        }
//    }
}

void ClassesForm::on_label_class1()
{
    m_selcalss.name = ui->label_class1->text();
    m_labelClass[0]->setStyleSheet("QLabel{ background-color: rgb(255, 255, 0)}");
    m_labelClass[1]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
    m_labelClass[2]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
    m_selcalss.ndex = 0;
    emit class_start();
}

void ClassesForm::on_label_class2()
{
    m_selcalss.name = ui->label_class2->text();
    m_labelClass[1]->setStyleSheet("QLabel{ background-color: rgb(255, 255, 0)}");
    m_labelClass[0]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
    m_labelClass[2]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
    m_selcalss.ndex = 1;
    emit class_start();
}

void ClassesForm::on_label_class3()
{
    m_selcalss.name = ui->label_class3->text();
    m_labelClass[2]->setStyleSheet("QLabel{ background-color: rgb(255, 255, 0)}");
    m_labelClass[0]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
    m_labelClass[1]->setStyleSheet("QLabel{ background-color: rgb(135, 135, 135)}");
    m_selcalss.ndex = 2;
    emit class_start();
}

void ClassesForm::GetSelClassid(class_sel& sel)
{
    int size = m_list_classt.size();
    for (int i=0; i<size; i++)
    {
        class_template classt = m_list_classt.at(i);
        if (classt.name == m_selcalss.name)
        {
            sel.id = classt.id;
            sel.name = classt.name;
            sel.ndex = m_selcalss.ndex;
        }
    }
}

void ClassesForm::SetClassState(bool bstate, class_sel sel)
{
    if (sel.ndex >= 3)
        return;
//    if (bstate)
//    {
//        //ui->btn_class->setText("下课");
//        m_bclass = true;
//    }else
//    {
//        //ui->btn_class->setText("上课");
//        m_bclass = false;
//    }
    m_labelClass[sel.ndex]->setStyleSheet("QLabel{ background-color: rgb(255, 255, 255)}");
}

bool ClassesForm::GetClassFlag()
{
    return m_bclass;
}
