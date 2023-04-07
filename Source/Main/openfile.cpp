#include "Headers/Main/mainwindow.h"
#include "ui_mainwindow.h"

template <typename theFile>
void ProgWindow::openFile(QString fileType){
    QString openLimiter;
    int mode = 0;
    if(fileType == "VBIN" or fileType == "STL" or fileType == "DAE"){
        openLimiter = "Model files (*." + fileType + ")";
        mode = 2;
    } else if (fileType == "ITF" or fileType == "BMP"){
        openLimiter = "Texture files (*." + fileType + ")";
        mode = 1;
    } else if (fileType == "BMD" or fileType == "TMD"){
        openLimiter = "Definition files (*." + fileType + ")";
        mode = 5;
    }else if (fileType == "BDB" or fileType == "TDB"){
        openLimiter = "Database files (*." + fileType + ")";
        mode = 5;
    }
    QString openSelector = "Select " + fileType;
    fileData.input = true;
    QString fileIn = QFileDialog::getOpenFileName(this, tr(openSelector.toStdString().c_str()), QDir::currentPath() + "/" + fileType + "/", tr(openLimiter.toStdString().c_str()));
    if(!fileIn.isNull()){
        fileMode = fileType;
        changeMode(mode);

        theFile::openFile(fileIn);
    }
}

void ProgWindow::openVBIN(){
    fileMode = "VBIN";
    VBIN vbinFile;
    clearWindow();
    fileData.input = true;
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VBIN"), QDir::currentPath() + "/VBIN/", tr("Model Files (*.vbin)"));
    if (!fileIn.isNull()){
        changeMode(2);
        vbinFile.filePath = fileIn;
        vbinFile.highestLOD = 0;
        vbinFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        vbinFile.fileName = fileInfo.fileName();
        vbinFile.fileWithoutExtension = vbinFile.fileName.left(vbinFile.fileName.indexOf("."));
//        fileData.dataBytes.clear();

//        QFile inputFile(fileIn);
//        inputFile.open(QIODevice::ReadOnly);
//        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        if(vbinFile.readData()){
            messageError("There was an error reading " + vbinFile.fileName);
            return;
        }
        qDebug() << Q_FUNC_INFO << "File data read.";
        vbinFiles.push_back(vbinFile);
        createLevelList(vbinFile.highestLOD);
        createFileList();
        createMultiRadios();
        //createAnimationList(vbinFile.animationSet);
    } else {
        messageError("There was an error opening the file.");
    }

}

void ProgWindow::openITF(){
    fileMode = "ITF";
    ITF itfFile;
    deleteMultiRadios();
    clearWindow();
    fileData.input = true;
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select ITF"), QDir::currentPath() + "/ITF/", tr("Texture Files (*.itf)"));
    if (!fileIn.isNull()){
        changeMode(1);
        itfFile.filePath = fileIn;
        itfFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        itfFile.fileName = fileInfo.fileName();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        itfFile.readData();
        itfFiles.push_back(itfFile);
        createLevelList(itfFile.paletteCount);
        createFileList();
        itfFile.populatePalette();
        qDebug() << Q_FUNC_INFO << "File data read.";
    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openVAC(){
    fileMode = "Tone";
    clearWindow();
    fileData.input = true;
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VAC"), QDir::currentPath() + "/VAC/", tr("Audio Files (*.vac)"));
    if (!fileIn.isNull()){
        changeMode(5);
        vacFile->filePath = fileIn;
        vacFile->parent = this;
        fileData.readFile(fileIn);

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        vacFile->tempRead();
        qDebug() << Q_FUNC_INFO << "File data read.";
    } else {
        messageError("There was an error opening the file.");
    }

}

void ProgWindow::openDefinition(bool binary){
    int passed=0;
    DefinitionFile openedFile;
    QString fileIn;
    clearWindow();
    fileData.input = true;
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
            createDBButtons(4);
            openedFile.createDBTree();
            definitions.push_back(openedFile);
            changeMode(4);
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
    clearWindow();
    fileData.input = true;
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
            createDBButtons(4);
            openedFile.createDBTree();
            databases.push_back(openedFile);
            changeMode(4);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

