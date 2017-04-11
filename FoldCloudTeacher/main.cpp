#include "flodcloudwidget.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FlodCloudWidget w;
    w.show();
    int nRet = a.exec();
    qDebug() << "main exit.";
    return nRet;
}
