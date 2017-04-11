#ifndef CLASSESFORM_H
#define CLASSESFORM_H

#include <QWidget>
#include <QPoint>
#include "global.h"
#include <QLabel>

namespace Ui {
class ClassesForm;
}

class ClickableLabel : public QLabel
{
Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent=0 );
    ~ClickableLabel();
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event);
};

class ClassesForm : public QWidget
{
    Q_OBJECT
signals:
    void class_start();
public:
    explicit ClassesForm(QWidget *parent = 0);
    ~ClassesForm();
public:
    void initclasst();
    void GetSelClassid(class_sel& sel);
    void SetClassState(bool bstate, class_sel sel);
    bool GetClassFlag();
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private slots:
    void exit_widget();
    void on_left();
    void on_right();
    void on_class();
    void on_label_class1();
    void on_label_class2();
    void on_label_class3();
private:
    Ui::ClassesForm *ui;
    bool  m_moving;
    QPoint  m_LastMousePosition;
    QString  m_szRoomId;
    class_sel  m_selcalss;
    ClickableLabel *m_labelClass[3];
    int   m_direction;
    int   m_firtindex;
    bool  m_bclass;
};

#endif // CLASSESFORM_H
