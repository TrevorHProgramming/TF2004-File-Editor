#include "Headers/Main/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProgWindow w;
    w.setWindowTitle("TF2004 File Converter");
    w.setGeometry(QRect(QPoint(0,0), QSize(w.hSize,w.vSize)));
    //w.resize(1200, 600);
    w.show();
    return a.exec();
}
