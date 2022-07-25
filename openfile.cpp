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
        vbinFile->highestLOD = 0;
        vbinFile->parent = this;
        fileData.readFile(fileIn);
//        fileData.dataBytes.clear();

//        QFile inputFile(fileIn);
//        inputFile.open(QIODevice::ReadOnly);
//        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        vbinFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";

        createDropdown(vbinFile->highestLOD);
        createMultiRadios();
    } else {
        messageError("There was an error opening the file.");
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
        fileData.readFile(fileIn);
//        fileData.dataBytes.clear();

//        QFile inputFile(fileIn);
//        inputFile.open(QIODevice::ReadOnly);
//        fileData.dataBytes = inputFile.readAll();


        qDebug() << Q_FUNC_INFO << "File data loaded.";
        itfFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openTMD(){
    int passed=0;
    TMDFile openedFile;
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
        //tmdFile[0]->filePath = fileIn;
        openedFile.filePath = fileIn;
        openedFile.parent = this;
        fileData.readFile(fileIn);
//        fileData.dataBytes.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        //tmdFile[0]->fileName = fileInfo.fileName();
        //tmdFile[0]->fileType = "TMD";
        openedFile.fileName = fileInfo.fileName();
        openedFile.fileType = "TMD";
        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        //passed = tmdFile[0]->readData();
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
        if (passed != -1){
            createDBButtons();
            //tmdFile[0]->createDBTree();
            openedFile.createDBTree();
            tmdFile.push_back(&openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openTDB(){
    int passed = 0;
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
        fileData.readFile(fileIn);
        //fileData.dataBytes.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        tdbFile->fileName = fileInfo.fileName();
        tdbFile->fileType = "TDB";
        //fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        //passed = tdbFile->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
       if (passed != -1) {
           tdbFile->createDBTree();
       } else {
           messageError("There was an error reading the file.");
       }
    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openBMD(){
    int passed=0;
    deleteMultiRadios();
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select BMD"), QDir::currentPath() + "/BMD/", tr("Binary Database Definition Files (*.BMD)"));
    if (!fileIn.isNull()){
        bmdFile[0]->filePath = fileIn;
        fileData.readFile(fileIn);
        //fileData.dataBytes.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        bmdFile[0]->fileName = fileInfo.fileName();
        bmdFile[0]->fileType = "TMD";
        //fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        //passed = bmdFile[0]->readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
        /*if (passed != -1){
            createDBButtons();
//            DBClassList->clear();
//            for (int i = 0; i < tmdFile->classList.size(); i++) {
//                DBClassList->addItem(tmdFile->classList[i].name);
//            }
            bmdFile[0]->createDBTree();
        } else {
            messageError("There was an error reading the file.");
        }*/

    } else {
        messageError("There was an error opening the file.");
    }
}
