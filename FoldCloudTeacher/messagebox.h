#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>

namespace Ui {
class MessageBox;
}

class MessageBox : public QWidget
{
    Q_OBJECT

public:
    explicit MessageBox(QWidget *parent = 0);
    ~MessageBox();
private slots:
    void On_Close();
public:
    void SetText(QString stext);
private:
    Ui::MessageBox *ui;
};

#endif // MESSAGEBOX_H
