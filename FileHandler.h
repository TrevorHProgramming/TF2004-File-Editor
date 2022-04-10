#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QByteArray>
#include <QString>
#include "itf.h"
#include "vbin.h"

class ProgWindow;

class FileHandler{
public slots:
   void loadFile(ProgWindow& MainWindow);
   void saveFile(ProgWindow& MainWindow);
};

#endif // FILEHANDLER_H
