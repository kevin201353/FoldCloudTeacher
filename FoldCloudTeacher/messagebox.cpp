#include "messagebox.h"
#include "ui_messagebox.h"

MessageBox::MessageBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    ui->btnStuClose->setStyleSheet("QPushButton{border-image: url(:/images/x.png);}"
                            "QPushButton:pressed{border-image: url(:/images/x.png);}");
    connect(ui->btnStuClose, SIGNAL(clicked(bool)), this, SLOT(On_Close()));
}

MessageBox::~MessageBox()
{
    delete ui;
}

void MessageBox::On_Close()
{
    hide();
}

void MessageBox::SetText(QString stext)
{
    ui->label_msg->setText(stext);
}
