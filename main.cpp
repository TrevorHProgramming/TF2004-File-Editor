#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProgWindow w;
    w.setWindowTitle("TF2004 Model and Texture Converter");
    w.setGeometry(QRect(QPoint(160,90), QSize(w.hSize,w.vSize)));
    //w.resize(1200, 600);
    w.show();
    return a.exec();
}
