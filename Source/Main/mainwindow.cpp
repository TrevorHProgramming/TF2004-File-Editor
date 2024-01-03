#include "Headers/Main/mainwindow.h"
#include "ui_mainwindow.h"

ProgWindow::ProgWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setW = new SettingsWindow(this);
    //this->setPalette(palette);

    MessagePopup = new QMessageBox(this);
    MessagePopup->setGeometry(QRect(QPoint(int(hSize*0.5),int(vSize*0.5)), QSize(120,30)));
    menuBar()->setGeometry(QRect(QPoint(int(hSize*0),int(vSize*0)), QSize(int(hSize*1),25)));

    fileData.parent = this;

    centralContainer = new QWidget(this);
    setCentralWidget(centralContainer);
    centralContainer->setGeometry(QRect(QPoint(hSize*0.15,0), QSize(hSize*0.6,vSize)));
    updateBackground();


    QMenu *menuVBIN = menuBar()->addMenu("Model");
    QMenu *menuITF = menuBar()->addMenu("Texture");
    QMenu *menuSFX = menuBar()->addMenu("Sound");
    QMenu *menuDatabase = menuBar() -> addMenu("Database");
    QMenu *menuCalculator = menuBar()->addMenu("Calculator");
    QMenu *menuClear = menuBar()->addMenu("Clear");
    QMenu *menuSettings = menuBar()->addMenu("Settings");

    QAction *actionLoadVBIN = menuVBIN->addAction("Load VBIN");
    QAction *actionLoadMeshVBIN = menuVBIN->addAction("Load Mesh VBIN");
    QAction *actionSaveModel = menuVBIN ->addAction("Export Model");
    QAction *actionBulkLoadModel = menuVBIN->addAction("Bulk Load Model");
    QAction *actionBulkSaveModel = menuVBIN->addAction("Bulk Export Model");

    QAction *actionLoadITF = menuITF ->addAction("Load ITF");
    QAction *actionLoadQImage = menuITF ->addAction("Import Image");
    QAction *actionSaveITF = menuITF->addAction("Export Texture");
    QAction *actionBulkLoadITF = menuITF->addAction("Bulk Load Texture");
    QAction *actionBulkSaveITF = menuITF->addAction("Bulk Export Texture");

    QAction *actionLoadVAC = menuSFX ->addAction("Load VAC");
    QAction *actionSaveVAC = menuSFX->addAction("Export VAC");
    QAction *actionBulkVAC = menuSFX->addAction("Bulk Export Sound");

    QAction *actionLoadTMD = menuDatabase ->addAction("Load TMD");
    QAction *actionLoadTDB = menuDatabase ->addAction("Load TDB");
    QAction *actionLoadBMD = menuDatabase ->addAction("Load BMD");
    QAction *actionLoadBDB = menuDatabase ->addAction("Load BDB");
    QAction *actionSaveDatabase = menuDatabase ->addAction("Export Database");
    QAction *actionBulkDatabase = menuDatabase->addAction("Bulk Export Database");
//    QAction *actionSaveTDB = menuDatabase ->addAction("Save TDB");
//    QAction *actionSaveBMD = menuDatabase ->addAction("Save BMD");
//    QAction *actionSaveBDB = menuDatabase ->addAction("Save BDB");

    QAction *actionOpenCalculator = menuCalculator -> addAction("Warpgate Distance Calculator");

    QAction *actionClearFiles = menuClear->addAction("Clear Loaded Files");
    QAction *actionClearLog = menuClear->addAction("Clear Log");

    QAction *actionSettings = menuSettings -> addAction("Settings");

    leftSidebar = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, leftSidebar);
    leftSidebar->setFloating(false);
    leftSidebar->show();
    fileBrowser = new QListWidget;
    leftSidebar->setWidget(fileBrowser);
    fileBrowser->setGeometry(QRect(QPoint(0,0), QSize(hSize*0.15,vSize)));
    connect(fileBrowser, &QListWidget::itemSelectionChanged, this, &ProgWindow::updateCenter);
    //have to connect fileBrowser to a centralwidget updater

    rightSidebar = new QDockWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, rightSidebar);
    rightSidebar->setFloating(false);
    rightSidebar->show();
    logPrintout = new QListWidget;
    rightSidebar->setWidget(logPrintout);
    logPrintout->setGeometry(QRect(QPoint(0,0), QSize(hSize*0.15,vSize)));


    /*hiding SFX menu for this patch since this system is far from ready*/
    //menuSFX->setVisible(false);

    vacFile = new VACFile;
    vacFile->parent = this;
    ListAnimation = nullptr;
    ListFrame = nullptr;
    DBNewValue = nullptr;
    ButtonCalculate = nullptr;
    CalculateXValue = nullptr;
    CalculateYValue = nullptr;
    CalculateZValue = nullptr;
    ClosestWarpgate = nullptr;
    warpgateCalculator = nullptr;



    connect(actionSaveModel, &QAction::triggered, this, [this] {saveFile("Model");});
    connect(actionBulkSaveModel, &QAction::triggered, this, [this] {bulkSave("Model");});
    //connect(actionSaveDAE, &QAction::triggered, this, [this] {saveFile("VBIN");});
    connect(actionSaveITF, &QAction::triggered, this, [this] {saveFile("Texture");});
    connect(actionBulkSaveITF, &QAction::triggered, this, [this] {bulkSave("Texture");});
    connect(actionSaveDatabase, &QAction::triggered, this, [this] {saveFile("Database");});
    connect(actionBulkDatabase, &QAction::triggered, this, [this] {bulkSave("Database");});
    //connect(actionSaveITF, &QAction::triggered, this, [this] {saveFile("ITF");});
    //connect(actionSaveLevelSTL, &QAction::triggered, this, [this] {saveFile("MESH.VBIN");});
    //connect(actionSaveVAC, &QAction::triggered, this, [this] {saveFile("VAC", "VAC");});


    connect(actionLoadVBIN, &QAction::triggered, this, [this] {openFile("VBIN");});
    connect(actionBulkLoadModel, &QAction::triggered, this, [this] {bulkOpen("VBIN");});
    connect(actionLoadMeshVBIN, &QAction::triggered, this, [this] {openFile("MESH.VBIN");});
    connect(actionLoadITF, &QAction::triggered, this, [this] {openFile("ITF");});
    connect(actionLoadQImage, &QAction::triggered, this, [this] {openFile("Image Files (*." + QImageWriter::supportedImageFormats().join(";*.")+")");});
    //QImageWriter::supportedImageFormats().join(";")
    connect(actionBulkLoadITF, &QAction::triggered, this, [this] {bulkOpen("ITF");});
    connect(actionLoadTMD, &QAction::triggered, this, [this] {openFile("TMD");});
    connect(actionLoadBMD, &QAction::triggered, this, [this] {openFile("BMD");});
    connect(actionLoadTDB, &QAction::triggered, this, [this] {openFile("TDB");});
    connect(actionLoadBDB, &QAction::triggered, this, [this] {openFile("BDB");});

    connect(actionSettings, &QAction::triggered, this, &ProgWindow::handleSettings);

    //vac is commented out as it doesn't actually do anything
    //connect(actionLoadVAC, &QAction::triggered, this, [this] {openFile("VAC");});

    //because of databases, change "saveFile" to prompt the user for available save types
    //that way we only need one save button for each category of file instead of a different one for each possible output

    connect(actionOpenCalculator, &QAction::triggered, this, &ProgWindow::openWarpgateCalculator);

    connect(actionClearFiles, &QAction::triggered, this, &ProgWindow::clearFiles);
    connect(actionClearLog, &QAction::triggered, this, &ProgWindow::clearLog);


    setWindowState(Qt::WindowMaximized);
    /*time_t lastCheck = win_sparkle_get_last_check_time();
    time(&lastCheck);
    messageSuccess("Last update check occurred at " + QString(ctime(&lastCheck)));*/
}

ProgWindow::~ProgWindow()
{
    win_sparkle_cleanup();
    delete ui;
}

void ProgWindow::log(QString message){
    qDebug() << Q_FUNC_INFO << message;
    logPrintout->addItem(message);
}

void ProgWindow::clearLog(){
    logPrintout->clear();
}

void ProgWindow::saveFile(QString fromType, QString givenPath){
    QStringList validFiles;
    QString selectedFile;
    QString currentCenterFile;
    int centerFileIndex = 0;
    bool completed;
    qDebug() << Q_FUNC_INFO << "loaded files" << loadedFiles.size();
    for(int i = 0; i < loadedFiles.size(); i++){
        //qDebug() << Q_FUNC_INFO << "loaded file" << loadedFiles[i]->fileName << "with extension" << loadedFiles[i]->fileExtension << "vs" << fromType;
        if(loadedFiles[i]->fileCategory() == fromType){
            validFiles.append(loadedFiles[i]->fullFileName());
        }
    }
    if(validFiles.isEmpty()){
        messageError("No loaded files of the selected type: " + fromType);
        return;
    }

    currentCenterFile = fileBrowser->currentItem()->text();
    if(validFiles.contains(currentCenterFile)){
        qDebug() << Q_FUNC_INFO << "File was found in the list.";
        //I would use validFiles.indexOf(currentCenterFile) but that doesn't work
        for(int i = 0; i < validFiles.size(); i++){
            qDebug() << Q_FUNC_INFO << "comparing" << validFiles[i] << "to" << currentCenterFile;
            if(validFiles[i] == currentCenterFile){
                centerFileIndex = i;
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "center file index:" << centerFileIndex;

    selectedFile = QInputDialog::getItem(this, tr("Select a file:"), tr("File name:"), validFiles, centerFileIndex, false, &completed);
    if(!completed){
        messageError("File save operation cancelled.");
        return;
    }
    for(int i = 0; i < loadedFiles.size(); i++){
        if(loadedFiles[i]->fullFileName() != selectedFile){
            continue;
        }
        QString selectedType = QInputDialog::getItem(this, tr("Select an output type:"), tr("File type:"), loadedFiles[i]->validOutputs(), 0, false, &completed);
        qDebug() << Q_FUNC_INFO << "completed value:" << completed;
        if(!completed){
            messageError("File save operation cancelled.");
            return;
        }

        QString fileOut;
        if(givenPath == ""){
            fileOut = QFileDialog::getSaveFileName(this, tr(QString("Select Output "  + selectedType).toStdString().c_str()), QDir::currentPath() + "/" + selectedType + "/"
                                                               , tr(QString(loadedFiles[i]->fileCategory() + " Files (*." + selectedType + ")").toStdString().c_str()));
        } else {
            fileOut = givenPath;
        }


        if(fileOut.isEmpty()){
            messageError(selectedType + " export cancelled.");
            return;
        }
        loadedFiles[i]->outputPath = fileOut;
        loadedFiles[i]->save(selectedType);
    }

}

void ProgWindow::bulkSave(QString category){
    QStringList validFiles;
    QString selectedType = "";
    QString currentCenterFile;
    int centerFileIndex = 0;
    bool completed;
    qDebug() << Q_FUNC_INFO << "loaded files" << loadedFiles.size();
    for(int i = 0; i < loadedFiles.size(); i++){
        //qDebug() << Q_FUNC_INFO << "loaded file" << loadedFiles[i]->fileName << "with extension" << loadedFiles[i]->fileExtension << "vs" << fromType;
        if(loadedFiles[i]->fileCategory() != category){
            continue;
        }
        if(selectedType == ""){
            selectedType = QInputDialog::getItem(this, tr("Select an output type:"), tr("File type:"), loadedFiles[i]->validOutputs(), 0, false, &completed);
            if(!completed){
                messageError("File save operation cancelled.");
                return;
            }
        }
        validFiles.append(loadedFiles[i]->fullFileName());
    }
    if(validFiles.isEmpty()){
        messageError("No loaded files of the selected type: " + category);
        return;
    }

    QString filePath = QFileDialog::getExistingDirectory(this, tr("Select output folder"), QDir::currentPath());

    if(filePath.isEmpty()){
        messageError(category + " export cancelled.");
        return;
    }
    for(int i = 0; i < loadedFiles.size(); i++){
        if(loadedFiles[i]->fileCategory() != category){
            continue;
        }
//        if(selectedType == ""){
//            selectedType = QInputDialog::getItem(this, tr("Select an output type:"), tr("File type:"), loadedFiles[0]->validOutputs(), 0, false, &completed);
//            if(!completed){
//                messageError("File save operation cancelled.");
//                return;
//            }
//        }

        loadedFiles[i]->outputPath = filePath + "\\" + loadedFiles[i]->fileName + "." + selectedType;
        loadedFiles[i]->save(selectedType);
        messageSuccess(selectedType + " file saved.");
    }

}

std::shared_ptr<TFFile> ProgWindow::matchFile(QString fileNameFull){
    if(loadedFileNames.contains(fileNameFull.toUpper())){
        return loadedFiles[loadedFileNames.indexOf(fileNameFull.toUpper())];
    }
    /*for(int i = 0; i < loadedFiles.size(); i++){
        qDebug() << Q_FUNC_INFO << "loaded file" << loadedFiles[i]->fullFileName().toUpper() << "vs" << fileNameFull.toUpper();
        if(loadedFiles[i]->fullFileName().toUpper() == fileNameFull.toUpper()){
            return loadedFiles[i];
        }
    }*/
    return nullptr;
}

void ProgWindow::updateBackground(){
    //background.load(QCoreApplication::applicationDirPath() + "/assets/background.png");
    //background = background.scaled(centralContainer->size());
    centralContainer->setAutoFillBackground(true);
    //palette.setBrush(QPalette::Window, QBrush(background));
    palette.setColor(QPalette::Window, Qt::black);
    centralContainer->setPalette(palette);
    centralContainer->show();
}

void ProgWindow::updateCenter(){
    clearWindow();
    if(fileBrowser->selectedItems().empty()){
        return;
    }
    int chosenFile = fileBrowser->selectionModel()->selectedIndexes().first().row(); //that's a mess but it should work
    qDebug() << Q_FUNC_INFO << "chosen file index" << chosenFile;
    loadedFiles[chosenFile]->updateCenter();
}

void ProgWindow::openWarpgateCalculator(){
    clearWindow();
    warpgateCalculator = new DistanceCalculator(this);
}

void ProgWindow::clearWindow(){

    for (int i = 0; i < currentModeWidgets.size(); i++) {
        qDebug() << Q_FUNC_INFO << "checking widget" << currentModeWidgets[i]->metaObject()->className();
        currentModeWidgets[i]->setVisible(false);
        delete currentModeWidgets[i];
    }
    if(warpgateCalculator != nullptr){
        delete warpgateCalculator;
        warpgateCalculator = nullptr;
    }
    currentModeWidgets.clear();
    repaint(); //the buttons will visually remain, despite being set to not visible, until the program is told to repaint.
    //this will only work if the background image is usable
}

void ProgWindow::clearFiles(){
    loadedFiles.clear();
    fileBrowser->clear();
}

void ProgWindow::resizeEvent(QResizeEvent* event){
    /*Resizes the background to fit the window. Will eventually add element placements so it doesn't look terrible if full-screened.*/
    QMainWindow::resizeEvent(event);
    hSize = this->size().width();
    vSize = this->size().height();
    //qDebug() << Q_FUNC_INFO << "hsize" << hSize << "vSize" << vSize;
    menuBar()->setGeometry(QRect(QPoint(int(hSize*0),int(vSize*0)), QSize(int(hSize*1),25)));
    //updateBackground();
    //this->setPalette(palette);
}

void ProgWindow::messageError(QString message){
    MessagePopup->setText(message);
    MessagePopup->setWindowTitle("Error!");
    MessagePopup->exec();
}

void ProgWindow::messageSuccess(QString message){
    MessagePopup->setText(message);
    MessagePopup->setWindowTitle("Success.");
    MessagePopup->exec();
}

void ProgWindow::handleSettings(){
    //open a settings window
    //should probably just make the connection to settingswindow::open but this will do for now
    setW->open();
}

SettingsWindow::SettingsWindow(ProgWindow *sentParent){

    parent = sentParent;
    savedChanges = true;
    setWindowTitle("VBIN Converter Settings");
    sendUpdate = new QPushButton("Save Settings", this);
    sendUpdate -> setGeometry(QRect(QPoint(25,25), QSize(200,25)));

    connect(sendUpdate, &QPushButton::released, this, &SettingsWindow::updateSettings);
}

void SettingsWindow::updateSettings(){
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationDirPath();
    QString fileOut = QCoreApplication::applicationDirPath() + "/settings.txt";
    QFile outputFile(fileOut);

    if (!savedChanges){
        qDebug() << Q_FUNC_INFO << "Settings changed";
        outputFile.open(QIODevice::WriteOnly);
        for(int i = 0; i < settingsNames.length(); i++){
            if(i!= 0){
                outputFile.write("\n");
            }
            outputFile.write(settingsNames[i].toUtf8()+": " + settingsValues[i].toUtf8());
        }
        savedChanges = true;
    }

    qDebug() << Q_FUNC_INFO << "Settings updated";
}

void SettingsWindow::writeSettings(QFile *outputFile){
    qDebug() << Q_FUNC_INFO << "Settings file not found. Creating a default settings file.";
    outputFile->open(QIODevice::WriteOnly);
    for(int i = 0; i < settingsNames.length(); i++){
        if(i!= 0){
            outputFile->write("\n");
        }
        outputFile->write(settingsNames[i].toUtf8()+": " + settingsValues[i].toUtf8());
    }
}

void SettingsWindow::open(){
    settingsEdit = new QTableWidget(settingsNames.size(), 2, this);
    settingsEdit->setGeometry(QRect(QPoint(50,50), QSize(300,300)));
    for (int i =0; i < settingsNames.size();i++) {
        QTableWidgetItem *nextSetName = settingsEdit->item(i ,0);
        QTableWidgetItem *nextSetValue = settingsEdit->item(i ,1);
        qDebug() << Q_FUNC_INFO << settingsNames[i] << ": " << settingsValues[i];
        if(!nextSetName){
            nextSetName = new QTableWidgetItem;
            nextSetName->setFlags(Qt::ItemIsEditable);
            settingsEdit->setItem(i ,0, nextSetName);
        }
        nextSetName->setText(settingsNames[i]);
        if(!nextSetValue){
            nextSetValue = new QTableWidgetItem;
            settingsEdit->setItem(i ,1, nextSetValue);
        }
        nextSetValue->setText(settingsValues[i]);
    }
    connect(settingsEdit, &QTableWidget::cellChanged, this, [this](int row, int column) {changeSetting(row, column);});

    show();
}

void SettingsWindow::changeSetting(int row, int column){
    savedChanges = false;
    settingsValues[row] = settingsEdit->item(row,1)->text();;
}

SettingsWindow::~SettingsWindow()
{
    delete sendUpdate;
}

