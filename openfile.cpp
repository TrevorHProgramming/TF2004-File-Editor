#include "mainwindow.h"
#include "ui_mainwindow.h"

void ProgWindow::openVBIN(){
    fileMode = "VBIN";
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VBIN"), QDir::currentPath() + "/VBIN/", tr("Model Files (*.vbin)"));
    if (!fileIn.isNull()){
        vbinFile->filePath = fileIn;
        fileData.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        fileData = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        vbinFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";

        createDropdown(vbinFile->highestLOD);
        createMultiRadios();
    }

}

void ProgWindow::openITF(){
    fileMode = "ITF";
    deleteMultiRadios();
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
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

void ProgWindow::openTMD(){
    fileMode = "TMD";
    deleteMultiRadios();
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select TMD"), QDir::currentPath() + "/TMD/", tr("Database Definition Files (*.TMD)"));
    if (!fileIn.isNull()){
        tmdFile->filePath = fileIn;
        fileData.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        fileData = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        tmdFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
        createDBButtons();
    }
}

void ProgWindow::openTDB(){
    fileMode = "TDB";
    deleteMultiRadios();
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select TDB"), QDir::currentPath() + "/TDB/", tr("Text Database Files (*.TDB)"));
    if (!fileIn.isNull()){
        tdbFile->filePath = fileIn;
        fileData.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        fileData = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        tdbFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
    }
}
