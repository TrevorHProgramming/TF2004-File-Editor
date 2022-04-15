#include "mainwindow.h"
#include "ui_mainwindow.h"

void ProgWindow::openVBIN(){
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VBIN"), QDir::currentPath() + "/VBIN/", tr("Model Files (*.vbin)"));
    vbinFile->filePath = fileIn;
    fileData.clear();

    QFile inputFile(fileIn);
    inputFile.open(QIODevice::ReadOnly);
    fileData = inputFile.readAll();

    qDebug() << Q_FUNC_INFO << "File data loaded.";
    vbinFile->readData();
    qDebug() << Q_FUNC_INFO << "File data read.";

    ListLODLevels->clear();
    //highestLOD = this->vbinFile->nodeList[1].meshList[0].lodInfo.levels;
    //qDebug() << Q_FUNC_INFO << "highest LOD: " << highestLOD;
    if (highestLOD == 0){
        ListLODLevels->insertItem(0, "1");
    } else {
        for(int i=0; i<highestLOD; ++i){
            ListLODLevels->insertItem(i, QString::number(i+1));
        }
    }


}

void ProgWindow::openITF(){
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select ITF"), QDir::currentPath() + "/ITF/", tr("Texture Files (*.itf)"));
    if (!fileIn.isNull()){
        itfFile->filePath = fileIn;
        fileData.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        fileData = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        itfFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
    }
}
