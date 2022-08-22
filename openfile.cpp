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

void ProgWindow::openDefinition(bool binary){
    int passed=0;
    DefinitionFile openedFile;
    QString fileIn;
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    if(binary){
        fileIn = QFileDialog::getOpenFileName(this, tr("Select BMD"), QDir::currentPath(), tr("Database Definition Files (*.BMD)"));
    } else {
        fileIn = QFileDialog::getOpenFileName(this, tr("Select TMD"), QDir::currentPath(), tr("Database Definition Files (*.TMD)"));
    }
    if (!fileIn.isNull()){
        openedFile.filePath = fileIn;
        openedFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.binary = binary;
        openedFile.database = false;
        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        if (passed != -1){
            createDBButtons();
            openedFile.createDBTree();
            definitions.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openDatabase(bool binary){
    int passed=0;
    DatabaseFile openedFile;
    QString fileIn;
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    if(binary){
        fileIn = QFileDialog::getOpenFileName(this, tr("Select BDB"), QDir::currentPath(), tr("Database Definition Files (*.BDB)"));
    } else {
        fileIn = QFileDialog::getOpenFileName(this, tr("Select TDB"), QDir::currentPath(), tr("Database Definition Files (*.TDB)"));
    }
    if (!fileIn.isNull()){
        openedFile.filePath = fileIn;
        openedFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.binary = binary;
        openedFile.database = true;
        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        if (passed != -1){
            createDBButtons();
            openedFile.createDBTree();
            databases.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

/*void ProgWindow::openTMD(){
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
        if (passed != -1){
            createDBButtons();
            //tmdFile[0]->createDBTree();
            openedFile.createDBTree();
            tmdFile.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openTDB(){
    int passed = 0;
    TDBFile openedFile;
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
        openedFile.filePath = fileIn;
        fileData.readFile(fileIn);
        //fileData.dataBytes.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.parent = this;
        openedFile.fileType = "TDB";
        //fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
       if (passed != -1) {
           openedFile.createDBTree();
           tdbFile.push_back(openedFile);
       } else {
           messageError("There was an error reading the file.");
       }
    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openBMD(){
    int passed=0;
    BMDFile openedFile;
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
        openedFile.filePath = fileIn;
        fileData.readFile(fileIn);
        //fileData.dataBytes.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.parent = this;
        openedFile.fileType = "BMD";
        //fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
        if (passed != -1) {
            openedFile.createDBTree();
            bmdFile.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openBDB(){
    int passed=0;
    BDBFile openedFile;
    deleteMultiRadios();
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
    if (ListLevels != nullptr) {
        ListLevels->clear();
        ListLevels->hide();
    }
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select BDB"), QDir::currentPath() + "/BDB/", tr("Binary Database Definition Files (*.BDB)"));
    if (!fileIn.isNull()){
        openedFile.filePath = fileIn;
        fileData.readFile(fileIn);
        //fileData.dataBytes.clear();

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.parent = this;
        openedFile.fileType = "BDB";
        //fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        //after verifying the TMD data is good, create the TDB and BDB file buttons
        if (passed != -1) {
            openedFile.createDBTree();
            bmdFile.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}*/
