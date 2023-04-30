#include "Headers/Main/mainwindow.h"
#include "ui_mainwindow.h"



ProgWindow::ProgWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setPalette(palette);

    MessagePopup = new QMessageBox(this);
    MessagePopup->setGeometry(QRect(QPoint(int(hSize*0.5),int(vSize*0.5)), QSize(120,30)));
    menuBar()->setGeometry(QRect(QPoint(int(hSize*0),int(vSize*0)), QSize(int(hSize*1),25)));

    fileData.parent = this;

    centralContainer = new QWidget(this);
    setCentralWidget(centralContainer);
    centralContainer->setGeometry(QRect(QPoint(hSize*0.15,0), QSize(hSize*0.6,vSize)));
    updateBackground();


    QMenu *menuVBIN = menuBar()->addMenu("VBIN");
    QMenu *menuITF = menuBar()->addMenu("ITF");
    QMenu *menuSFX = menuBar()->addMenu("SFX");
    QMenu *menuDatabase = menuBar() -> addMenu("Database");
    QMenu *menuCalculator = menuBar()->addMenu("Calculator");
    //QMenu *menuSettings = menuBar()->addMenu("Settings");
    QAction *actionLoadVBIN = menuVBIN->addAction("Load VBIN");
    QAction *actionSaveSTL = menuVBIN ->addAction("Export to STL");
    QAction *actionSaveLevelSTL = menuVBIN ->addAction("Export level to STL");
    QAction *actionSaveDAE = menuVBIN ->addAction("Export to DAE");
    QAction *actionLoadMeshVBIN = menuVBIN->addAction("Load Mesh VBIN");
    QAction *actionLoadITF = menuITF ->addAction("Load ITF");
    QAction *actionSaveITF = menuITF->addAction("Save ITF");
    QAction *actionLoadVAC = menuSFX ->addAction("Load VAC");
    QAction *actionSaveVAC = menuSFX->addAction("Save VAC");
    QAction *actionSaveBMP = menuITF ->addAction("Export to BMP");
    QAction *actionLoadTMD = menuDatabase ->addAction("Load TMD");
    QAction *actionLoadTDB = menuDatabase ->addAction("Load TDB");
    QAction *actionLoadBMD = menuDatabase ->addAction("Load BMD");
    QAction *actionLoadBDB = menuDatabase ->addAction("Load BDB");
    QAction *actionSaveTMD = menuDatabase ->addAction("Save TMD");
    QAction *actionSaveTDB = menuDatabase ->addAction("Save TDB");
    QAction *actionSaveBMD = menuDatabase ->addAction("Save BMD");
    QAction *actionSaveBDB = menuDatabase ->addAction("Save BDB");
    QAction *actionOpenCalculator = menuCalculator -> addAction("Warpgate Distance Calculator");
    //QAction *actionSettings = menuSettings -> addAction("Settings");

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



    connect(actionSaveSTL, &QAction::triggered, this, [this] {saveFile("VBIN", "STL");});
    connect(actionSaveDAE, &QAction::triggered, this, [this] {saveFile("VBIN", "DAE");});
    connect(actionSaveBMP, &QAction::triggered, this, [this] {saveFile("ITF", "BMP");});
    connect(actionSaveITF, &QAction::triggered, this, [this] {saveFile("ITF", "ITF");});
    connect(actionSaveLevelSTL, &QAction::triggered, this, [this] {saveFile("MESH.VBIN", "STL");});
    //connect(actionSaveVAC, &QAction::triggered, this, [this] {saveFile("VAC", "VAC");});


    connect(actionLoadVBIN, &QAction::triggered, this, [this] {openFile("VBIN");});
    connect(actionLoadMeshVBIN, &QAction::triggered, this, [this] {openFile("MESH.VBIN");});
    connect(actionLoadITF, &QAction::triggered, this, [this] {openFile("ITF");});
    connect(actionLoadTMD, &QAction::triggered, this, [this] {openFile("TMD");});
    connect(actionLoadBMD, &QAction::triggered, this, [this] {openFile("BMD");});
    connect(actionLoadTDB, &QAction::triggered, this, [this] {openFile("TDB");});
    connect(actionLoadBDB, &QAction::triggered, this, [this] {openFile("BDB");});

    //vac is commented out as it doesn't actually do anything
    //connect(actionLoadVAC, &QAction::triggered, this, [this] {openFile("VAC");});

    //because of databases, change "saveFile" to prompt the user for available save types
    //that way we only need one save button for each category of file instead of a different one for each possible output

    connect(actionOpenCalculator, &QAction::triggered, this, &ProgWindow::openWarpgateCalculator);


    setWindowState(Qt::WindowMaximized);
}

ProgWindow::~ProgWindow()
{
    delete ui;
}

void ProgWindow::saveFile(QString fromType, QString toType){
    QStringList validFiles;
    QString selectedFile;
    qDebug() << Q_FUNC_INFO << "loaded files" << loadedFiles.size();
    for(int i = 0; i < loadedFiles.size(); i++){
        //qDebug() << Q_FUNC_INFO << "loaded file" << loadedFiles[i]->fileName << "with extension" << loadedFiles[i]->fileExtension << "vs" << fromType;
        if(loadedFiles[i]->fileExtension == fromType){
            validFiles.append(loadedFiles[i]->fileName + "." + loadedFiles[i]->fileExtension);
        }
    }
    if(validFiles.isEmpty()){
        messageError("No loaded files of the selected type: " + fromType);
        return;
    }
    selectedFile = QInputDialog::getItem(this, tr("Enter selected value:"), tr("Value:"), validFiles);
    for(int i = 0; i < loadedFiles.size(); i++){
        if(loadedFiles[i]->fileName + "." + loadedFiles[i]->fileExtension == selectedFile){
            loadedFiles[i]->save(toType);
        }
    }

}

std::shared_ptr<TFFile> ProgWindow::matchFile(QString fileNameFull){
    for(int i = 0; i < loadedFiles.size(); i++){
        qDebug() << Q_FUNC_INFO << "loaded file" << loadedFiles[i]->fileName.toUpper() + "." + loadedFiles[i]->fileExtension.toUpper() << "vs" << fileNameFull.toUpper();
        if(loadedFiles[i]->fileName.toUpper() + "." + loadedFiles[i]->fileExtension.toUpper() == fileNameFull.toUpper()){
            return loadedFiles[i];
        }
    }
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
        qDebug() << Q_FUNC_INFO << "hiding widget" << currentModeWidgets[i]->metaObject()->className();;
        currentModeWidgets[i]->setVisible(false);
//        currentModeButtons[i]->setParent(nullptr);
        //layout()->removeWidget(currentModeWidgets[i]);
//        currentModeButtons[i]->deleteLater();
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

