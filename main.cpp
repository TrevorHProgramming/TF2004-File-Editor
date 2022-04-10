#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProgWindow w;
    w.setWindowTitle("TF2004 Model and Texture Converter");
    w.show();
    return a.exec();
}
