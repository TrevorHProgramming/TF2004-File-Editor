#include "Headers/Main/mainwindow.h"

//https://doc.qt.io/qt-6/qprocess.html

void IsoBuilder::setCopyPath(QString folderName){
    QDir gameParent(parent->setW->getValue("Game extract path"));
    gameParent.cdUp();
    copyOutputPath = gameParent.absolutePath() + "/" + folderName;
    //get containing directory
    //create Randomizer folder in that directory
    qDebug() << Q_FUNC_INFO << "path being checked:" << copyOutputPath << "based on " << gameParent.absolutePath();
    QDir checkDir(copyOutputPath);
    if(!checkDir.exists()){
        checkDir.mkpath(".");
    }
}

int IsoBuilder::unpackISO(){
    //calls TF04 ISO Manager to unpack and attempt to unzip TFA etc
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationDirPath();
    QString isoManagerPath = QCoreApplication::applicationDirPath() + "/TF04_ISO_Manager.exe";
    qDebug() << Q_FUNC_INFO << isoManagerPath;

    QStringList args;
    args.append("Unpack");
    QString isoPath = QFileDialog::getOpenFileName(parent, parent->tr("Choose game ISO"), QDir::currentPath(), parent->tr("*.iso"));
    if(isoPath == ""){
        qDebug() << Q_FUNC_INFO << "Process cancelled.";
        return 1;
    }
    parent->log("Unpacking ISO from: " + isoPath);
    args.append(isoPath);

    QProcess *isoManager = new QProcess();
    QObject::connect(isoManager, &QProcess::readyReadStandardOutput, [this,isoManager]() {
        //below only works if python output is flushed.
        QString debugOutput = isoManager->readAllStandardOutput();
        QDir dirCorrection("");
        qDebug() << debugOutput;
        int outputCode = 0;
        if(debugOutput.left(4) == "Code"){
            debugOutput = debugOutput.right(debugOutput.length() - 5);
            qDebug() << "trimmed debug output:" << debugOutput;
            outputCode = debugOutput.left(4).toInt();
        } else {
            outputCode = 0;
        }
        switch(outputCode){
        case 0:
            //nothing to worry about
            break;
        case 1:
            //ISOManager was unable to unzip the archives. Call 7zip unzipper
            parent->log("ISO Manager was not able to extract ZIP files. Calling 7zip.");
            unzipSpecial();
            break;
        case 2:
            //ISOManager has provided the clean extract directory
            debugOutput = debugOutput.right(debugOutput.length() - 5).trimmed();
            dirCorrection.setPath(debugOutput);
            //qDebug() << Q_FUNC_INFO << "current path" << debugOutput << "Corrected game path" << dirCorrection.absolutePath();
            parent->setW->setValue("Game extract path", dirCorrection.absolutePath());
            parent->setW->setValue("Modded game path", dirCorrection.absolutePath());
            break;
        default:
            //either nothing to worry about or something we haven't handled yet. output it.
            qDebug() << "Unhandled output code:" << debugOutput;
            break;
        }
    });
    isoManager->start(isoManagerPath, args);
    parent->log("Calling ISO Manager.");

    return 0;
}

void IsoBuilder::handleOutputCode(QString output){
    /*This was originally called from each of the readyReadStandardOutput connections, but it kept getting empty strings*/
    /*using this, we can check for specified codes or keywords at the start of the python debug output
    for now, I think these outputs will be simply "Code ____: human definition"
    then QT can read the first 4 for Code, skip the space, and qstring::number() the next four to react accordingly*/

}

int IsoBuilder::unzipSpecial(){
    //calls 7zip to unzip TFA if the ISO Manager was unable to
    parent->setW->loadSettings();
    QString sevenZipPath = parent->setW->getValue("7Zip EXE path");
    if(sevenZipPath == ""){
        //prompt user for 7zip location
        parent->log("7Zip executable path is not set. Please locate 7z.exe.");
        sevenZipPath = QFileDialog::getOpenFileName(parent, parent->tr("Locate 7Zip EXE"), QDir::currentPath(), parent->tr("7z.exe"));
        QDir dirCorrection(sevenZipPath);
        parent->setW->setValue("7Zip EXE path", dirCorrection.absolutePath());
    }

    QString outputPath = parent->setW->getValue("Game extract path");

    qDebug() << Q_FUNC_INFO << "Loading 7zip from:" << sevenZipPath;
    qDebug() << Q_FUNC_INFO <<"Targetting directory:" << outputPath;

    QStringList args;
    args.append("x");
    args.append(outputPath + "\\*.zip");
    args.append("-o" + outputPath + "\\*");
    args.append("-tzip");

    qDebug() << Q_FUNC_INFO << "args:" << args;

    QProcess *sevenZip = new QProcess();
    parent->updateLoadingBar(0,4);
    QObject::connect(sevenZip, &QProcess::readyReadStandardOutput, [this, sevenZip]() {
        QString debugOutput = sevenZip->readAllStandardOutput();
        qDebug() << debugOutput;
        int outputCode = 0;
        if(debugOutput.contains("Archives with Errors: 4")){
            outputCode = 1005;
        } else if (debugOutput.contains("Would you like to replace the existing file")){
            outputCode = 1007;
        } else if (debugOutput.contains("Unexpected end of archive")){
            outputCode = 1006;
        } else {
            outputCode = 0;
        }
        switch(outputCode){
        case 0:
            //nothing to worry about
            break;
        case 1005:
            //All extraction has completed
            qDebug() << Q_FUNC_INFO << "Extraction completed";
            break;
        case 1006:
            //One extraction has been completed
            qDebug() << Q_FUNC_INFO << "Extraction for file has completed";
            parent->updateLoadingBar();
            break;
        case 1007:
            //Files are already extracted
            qDebug() << Q_FUNC_INFO << "Extraction already completed.";
            sevenZip->close();
            parent->updateLoadingBar(4);
            break;
        default:
            //either nothing to worry about or something we haven't handled yet. output it.
            qDebug() << "Unhandled output code:" << debugOutput;
            break;
        }
    });
    sevenZip->start(sevenZipPath, args);
    return 0;
}
int IsoBuilder::rezipTFA_isoManager(bool removeFiles){
    //calls TF04 ISO Manager to repack TFA etc
    //removeFiles currently does nothing in this function - only in the ISO rebuild
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationDirPath();
    QString isoManagerPath = QCoreApplication::applicationDirPath() + "/TF04_ISO_Manager.exe";
    qDebug() << Q_FUNC_INFO << isoManagerPath;

    QStringList args;
    args.append("Build");
    QString extractPath = parent->setW->getValue("Modded game path");
    if(extractPath == ""){
        parent->log("Process cancelled. Extract path is not set.");
        return 1;
    }
    args.append(extractPath);

    QProcess *isoManager = new QProcess();
    QObject::connect(isoManager, &QProcess::readyReadStandardOutput, [this,isoManager, &removeFiles]() {
        //below only works if python output is flushed.
        QString debugOutput = isoManager->readAllStandardOutput();
        qDebug() << debugOutput;
        int outputCode = 0;
        if(debugOutput.left(4) == "Code"){
            debugOutput = debugOutput.right(debugOutput.length() - 5);
            qDebug() << "trimmed debug output:" << debugOutput;
            outputCode = debugOutput.left(4).toInt();
        } else {
            outputCode = 0;
        }
        switch(outputCode){
        case 0:
            //nothing to worry about
            break;
        case 3:
            //ISOManager is unable to build a bootable ISO. Call ImgBurn
            parent->log("ISOManager is unable to make a bootable ISO. Calling ImgBurn.");
            repackISO(removeFiles);
        default:
            //either nothing to worry about or something we haven't handled yet. output it.
            qDebug() << "Unhandled output code:" << debugOutput;
            break;
        }
    });
    parent->log("Starting ISOManager to rezip archives.");
    isoManager->start(isoManagerPath, args);

    return 0;
}

int IsoBuilder::rezipTFA_sevenZip(bool removeFiles){
    //calls 7zip to repack TFA etc
    //removeFiles currently does nothing in this function - only in the ISO rebuild
    parent->setW->loadSettings();
    QString sevenZipPath = parent->setW->getValue("7Zip EXE path");
    if(sevenZipPath == ""){
        //prompt user for 7zip location
        parent->log("7Zip executable path is not set. Please locate 7z.exe.");
        sevenZipPath = QFileDialog::getOpenFileName(parent, parent->tr("Locate 7Zip EXE"), QDir::currentPath(), parent->tr("7z.exe"));
        if(sevenZipPath == ""){
            parent->messageError("7zip was not located. Build process cancelled.");
            return 0;
        }
        QDir dirCorrection(sevenZipPath);
        parent->setW->setValue("7Zip EXE path", dirCorrection.absolutePath());
    }

    QString inputPath;
    if(randomizerOutput == ""){
        inputPath = parent->setW->getValue("Modded game path");
    } else {
        inputPath = randomizerOutput;
    }

    qDebug() << Q_FUNC_INFO << "Loading 7zip from:" << sevenZipPath;
    qDebug() << Q_FUNC_INFO <<"Targetting directory:" << inputPath;

    const static QStringList buildArchivesDeflate = {"TFA", "TFA2", "TA_XTRAS"};
    const static QStringList buildArchivesStore = {"SOUNDE"};

    QProcess *sevenZip = new QProcess();
    QObject::connect(sevenZip, &QProcess::readyReadStandardOutput, [this,sevenZip, &removeFiles]() {
        //below only works if python output is flushed.
        QString debugOutput = sevenZip->readAllStandardOutput();
        qDebug() << debugOutput;
        int outputCode = 0;
        if(debugOutput.left(4) == "Code"){
            debugOutput = debugOutput.right(debugOutput.length() - 5);
            qDebug() << "trimmed debug output:" << debugOutput;
            outputCode = debugOutput.left(4).toInt();
        } else {
            outputCode = 0;
        }
        switch(outputCode){
        case 0:
            //nothing to worry about
            break;
        case 3:
            //ISOManager is unable to build a bootable ISO. Call ImgBurn
            parent->log("ISOManager is unable to make a bootable ISO. Calling ImgBurn.");
            repackISO(removeFiles);
        default:
            //either nothing to worry about or something we haven't handled yet. output it.
            qDebug() << "Unhandled output code:" << debugOutput;
            break;
        }
    });
    parent->log("Starting 7Zip to rezip archives.");
    for(int i = 0; i < buildArchivesDeflate.size(); i++){
        QStringList args;
        args.append("a");
        //name and path of destination - [archive].zip
        //folder of source = folder/*
        args.append(inputPath + "\\" + buildArchivesDeflate[i] + ".zip");
        args.append(inputPath + "\\" + buildArchivesDeflate[i] + "\\*");
        args.append("-sdel");
        //qDebug() << Q_FUNC_INFO << "Here we would start 7zip with the commands:" << args;
        sevenZip->start(sevenZipPath, args);
        sevenZip->waitForFinished();
    }
    for(int i = 0; i < buildArchivesStore.size(); i++){
        QStringList args;
        args.append("a");
        //name and path of destination - [archive].zip
        //folder of source = folder/*
        args.append(inputPath + "\\" + buildArchivesStore[i] + ".zip");
        args.append(inputPath + "\\" + buildArchivesStore[i] + "\\*");
        args.append("-m0=Copy");
        args.append("-sdel");
        //qDebug() << Q_FUNC_INFO << "Here we would start 7zip with the commands:" << args;
        sevenZip->start(sevenZipPath, args);
        sevenZip->waitForFinished();
    }
    parent->log("All archives zipped.");
    repackISO(removeFiles);

    return 0;
}

int IsoBuilder::repackISO(bool removeFiles){
    //calls imgburn to rebuild a new ISO
    static QStringList elfVersions = {"SLUS_206.68","SLES_523.88","SLES_533.09","SLKA_251.75"};
    int currentELF = 0;
    parent->setW->loadSettings();
    QString imgBurnPath = parent->setW->getValue("ImgBurn EXE path");
    if(imgBurnPath == ""){
        //prompt user for imgBurn location
        parent->log("ImgBurn executable path is not set. Please locate ImgBurn.exe.");
        imgBurnPath = QFileDialog::getOpenFileName(parent, parent->tr("Locate ImgBurn EXE"), QDir::currentPath(), parent->tr("*.exe"));
        if(imgBurnPath == ""){
            parent->messageError("ImgBurn was not located. Build process cancelled.");
            return 0;
        }
        QDir dirCorrection(imgBurnPath);
        parent->setW->setValue("ImgBurn EXE path", dirCorrection.absolutePath());
    }

    QString inputPath;
    if(randomizerOutput == ""){
        inputPath = parent->setW->getValue("Modded game path");
    } else {
        inputPath = randomizerOutput;
    }
    QDir inputDir(inputPath);
    qDebug() << Q_FUNC_INFO << "testing path:" << inputDir.absolutePath() << "vs" << inputPath;
    inputPath = inputDir.absolutePath();
    inputPath.replace("/", "\\");
    QString elfPath;
    for(int i = 0; i < elfVersions.size(); i++){
        elfPath = inputPath + "\\" + elfVersions[i];
        QFileInfo elfFile(elfPath);
        qDebug() << Q_FUNC_INFO << "checking if ELF file" << elfPath << "exists:" << elfFile.isFile();
        if(elfFile.isFile()){
            currentELF = i;
        }
    }
    inputDir.cdUp();
    QString targetELF = elfVersions[currentELF];
    QString outputPath = inputDir.absolutePath() + "/Transformers " + targetELF.remove(".") + " Rebuild.iso";

    //there's probably a better way to do this but I don't feel like researching at the moment
    QString slashType;
    if(inputPath.contains("/")){
        slashType = "/";
    } else {
        slashType = "\\";
    }

    qDebug() << Q_FUNC_INFO << "Loading imgBurn from:" << imgBurnPath;
    qDebug() << Q_FUNC_INFO <<"Targetting directory:" << inputPath;
    qDebug() << Q_FUNC_INFO << "creating output" << outputPath;

    QString fileDeleteLog = "";

    QStringList args;
    QString fileListArg;
    fileListArg = inputPath + slashType + "SYSTEM.CNF";
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "SYSTEM.CNF");
    fileListArg += "|" + inputPath + slashType + elfVersions[currentELF];
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "SLUS_206.68");
    fileListArg += "|" + inputPath + slashType + "TA_XTRAS.ZIP";
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "TA_XTRAS.ZIP");
    fileListArg += "|" + inputPath + slashType + "TFA.ZIP";
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "TFA.ZIP");
    fileListArg += "|" + inputPath + slashType + "TFA2.ZIP";
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "TFA2.ZIP");
    fileListArg += "|" + inputPath + slashType + "SOUNDE.ZIP";
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "SOUNDE.ZIP");
    fileListArg += "|" + inputPath + slashType + "IOP" + slashType;
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "IOP" + slashType);
    fileListArg += "|" + inputPath + slashType + "FMV" + slashType;
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "FMV" + slashType);
    fileListArg += "|" + inputPath + slashType + "SOUND" + slashType;
    //parent->log("adding file or path to ISO build list: " + inputPath + slashType + "SOUND" + slashType);
    args.append("/MODE");
    args.append("BUILD");
    args.append("/OUTPUTMODE");
    args.append("IMAGEFILE");
    args.append("/SRC");
    args.append(fileListArg);
    args.append("/DEST");
    args.append(outputPath);
    args.append("/FILESYSTEM");
    args.append("ISO9660 + UDF");
    args.append("/UDFREVISION");
    args.append("\"1.02\"");
    args.append("/VOLUMELABEL");
    args.append(elfVersions[currentELF].remove("."));
    if(removeFiles){
        fileDeleteLog = " Deleting loose files once complete.";
        args.append("/DELETESOURCE");
    }
    args.append("/START");

    qDebug() << Q_FUNC_INFO << "args:" << args;

    QProcess *imgBurn = new QProcess();
    QObject::connect(imgBurn, &QProcess::readyReadStandardOutput, [imgBurn]() {
        QString debugOutput = imgBurn->readAllStandardOutput();
        qDebug() << debugOutput;
        int outputCode = 0;
        if(debugOutput.left(4) == "Code"){
            debugOutput = debugOutput.right(debugOutput.length() - 5);
            qDebug() << "trimmed debug output:" << debugOutput;
            outputCode = debugOutput.left(4).toInt();
        } else {
            outputCode = 0;
        }
        switch(outputCode){
        case 0:
            //nothing to worry about
            break;
        default:
            //either nothing to worry about or something we haven't handled yet. output it.
            qDebug() << "Unhandled output code:" << debugOutput;
            break;
        }
    });
    parent->log("Building ISO." + fileDeleteLog);
    imgBurn->start(imgBurnPath, args);
    return 0;
}

int IsoBuilder::packRandomizer(){
    randomizerInput = parent->setW->getValue("Modded game path");
    bool manualDirectory = false;
    if(randomizerInput == ""){
        randomizerInput = QFileDialog::getExistingDirectory(parent, parent->tr("Choose un-randomized source."), QDir::currentPath());
        manualDirectory = true;
    }
    QDir inputDir(randomizerInput);
    QDir findRandomizer(randomizerInput);
    findRandomizer.cdUp();
    QString randomizerOutput = findRandomizer.absolutePath() + "/Randomizer";
    if(randomizerInput == randomizerOutput){
        //randomizer directory is already the modded game directory - we need the game path instead
        randomizerInput = parent->setW->getValue("Game extract path");
        inputDir.setPath(randomizerInput);
    }
    QMessageBox confirmPath;

    confirmPath.setText("Is the input (non-randomized) path correct?");
    confirmPath.setInformativeText("Path: " + randomizerInput);
    confirmPath.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    int confirmation = 0;
    if(!manualDirectory){
        confirmation = confirmPath.exec();
    }
    if(confirmation == QMessageBox::No){
        randomizerInput = QFileDialog::getExistingDirectory(parent, parent->tr("Choose un-randomized source."), QDir::currentPath());
    }
    if(randomizerInput == ""){
        parent->log("Process cancelled.");
    } else {
        parent->setW->setValue("Game extract path", randomizerInput);
    }

    confirmPath.setText("Is the randomizer path correct?");
    confirmPath.setInformativeText("Path: " + randomizerOutput);

    confirmation = confirmPath.exec();
    if(confirmation == QMessageBox::No){
        randomizerOutput = QFileDialog::getExistingDirectory(parent, parent->tr("Choose randomizer source."), QDir::currentPath());
    }
    if(randomizerOutput == ""){
        parent->log("Process cancelled.");
    }

    qDebug() << Q_FUNC_INFO << "input path read as" << randomizerInput;
    qDebug() << Q_FUNC_INFO << "absolute up path read as" << findRandomizer.absolutePath();
    qDebug() << Q_FUNC_INFO << "Path confirmed?" << confirmation;

    parent->log("Copying files from " + randomizerInput + " to " + randomizerOutput);
    parent->log("Copying game files for Randomizer build. (This can take a couple minutes)");

    int totalFileCount = getFileCount(inputDir);
    int fileCount = 0;

    qDebug() << Q_FUNC_INFO << "Total files:" << totalFileCount;

    QDirIterator currentFile(inputDir, QDirIterator::Subdirectories);
    int sourcePathLength = inputDir.absoluteFilePath(randomizerInput).length();
    const static QStringList skipCopy = {"CREATURE.BDB", "TFA.ZIP", "TFA2.ZIP", "TA_XTRAS.ZIP", "SOUNDE.ZIP"};


    while(currentFile.hasNext()){
        currentFile.next();
        QFileInfo currentFileInfo = currentFile.fileInfo();
        if(!currentFileInfo.isHidden()){
            QString subPathStructure = currentFileInfo.absoluteFilePath().mid(sourcePathLength);
            QString outputPath = randomizerOutput + subPathStructure;
            qDebug() << Q_FUNC_INFO << "file name is" << currentFileInfo.fileName() << "does skipcopy contain?" << skipCopy.contains(currentFileInfo.fileName().toUpper());
            if(currentFileInfo.isDir()){
                qDebug() << Q_FUNC_INFO << "creating directory:" << outputPath;
                inputDir.mkpath(outputPath);
                fileCount++;
            } else if (currentFileInfo.isFile()
                       && !skipCopy.contains(currentFileInfo.fileName().toUpper())){
                qDebug() << Q_FUNC_INFO << "copying file" << fileCount << "/" << totalFileCount << ":" << currentFileInfo.fileName();
                QFile::copy(currentFileInfo.absoluteFilePath(), outputPath);
                fileCount++;
            }
            parent->updateLoadingBar(fileCount, totalFileCount);
        }
    }

    parent->updateLoadingBar(totalFileCount, totalFileCount);

    QDir dirCorrection(randomizerOutput);
    parent->setW->setValue("Modded game path", dirCorrection.absolutePath());
    rezipTFA_sevenZip(true);

    randomizerInput = "";
    randomizerOutput = "";

    return 0;
}

int IsoBuilder::getFileCount(QDir dirToCount){
    int fileCount = 0;
    QDirIterator fileCounter(dirToCount, QDirIterator::Subdirectories);
    while(fileCounter.hasNext()){
        fileCounter.next();
        fileCount++;
    }
    qDebug() << Q_FUNC_INFO << "directory" << dirToCount.absolutePath() << "has" << fileCount << "files";
    return fileCount;
}
