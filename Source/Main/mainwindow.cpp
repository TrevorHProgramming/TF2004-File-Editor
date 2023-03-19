#include "Headers/Main/mainwindow.h"
#include "ui_mainwindow.h"



ProgWindow::ProgWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    background.load(QCoreApplication::applicationDirPath() + "/assets/background.png");
    background = background.scaled(this->size());
    palette.setBrush(QPalette::Window, background);
    this->setPalette(palette);

    menuMain = new QMenuBar(this);
    MessagePopup = new QMessageBox(this);
    MessagePopup->setGeometry(QRect(QPoint(int(hSize*0.5),int(vSize*0.5)), QSize(120,30)));
    menuMain->setGeometry(QRect(QPoint(int(hSize*0),int(vSize*0)), QSize(int(hSize*1),25)));

    fileData.parent = this;

    QMenu *menuVBIN = menuMain->addMenu("VBIN");
    QMenu *menuITF = menuMain->addMenu("ITF");
    QMenu *menuSFX = menuMain->addMenu("SFX");
    QMenu *menuDatabase = menuMain -> addMenu("Database");
    QMenu *menuCalculator = menuMain->addMenu("Calculator");
    //QMenu *menuSettings = menuMain->addMenu("Settings");
    QAction *actionLoadVBIN = menuVBIN->addAction("Load VBIN");
    QAction *actionSaveSTL = menuVBIN ->addAction("Export to STL");
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


    /*hiding SFX menu for this patch since this system is far from ready*/
    //menuSFX->setVisible(false);

    //vbinFile = new VBIN;
    //vbinFile->parent = this;
    //itfFile = new ITF;
    //itfFile->parent = this;
    vacFile = new VACFile;
    vacFile->parent = this;
    //tmdFile.push_back(new TMDFile);
    //tmdFile[0]->parent = this;
    levelGeo = new MeshVBIN;
    levelGeo->parent = this;
    warpgateCalculator = new DistanceCalculator;
    warpgateCalculator->parent = this;
    PaletteTable = nullptr;
    ListLevels = nullptr;
    ListFiles = nullptr;
    ListAnimation = nullptr;
    ListFrame = nullptr;
    radioSingle = nullptr;
    radioMultiple = nullptr;
    ButtonOpenTDB = nullptr;
    ButtonWriteTMD = nullptr;
    DBClassList = nullptr;
    DBItemList = nullptr;
    DBDetailList = nullptr;
    DBValueList = nullptr;
    ButtonEditDB = nullptr;
    DBNewValue = nullptr;
    testView = nullptr;
    testModel = nullptr;
    LabelMode = nullptr;
    LabelName = nullptr;
    ButtonCalculate = nullptr;
    CalculateXValue = nullptr;
    CalculateYValue = nullptr;
    CalculateZValue = nullptr;
    ClosestWarpgate = nullptr;


    changeMode(0);


    connect(actionSaveSTL, &QAction::triggered, this, &ProgWindow::convertVBINToSTL);
    connect(actionSaveDAE, &QAction::triggered, this, &ProgWindow::convertVBINToDAE);
    connect(actionLoadVBIN, &QAction::triggered, this, &ProgWindow::openVBIN);
    connect(actionLoadMeshVBIN, &QAction::triggered, this, [this] {levelGeo->openMeshVBINFile();});
    //connect(ButtonITFtoBMP, &QPushButton::released, this, &ProgWindow::convertITFToBMP);
    connect(actionLoadITF, &QAction::triggered, this, &ProgWindow::openITF);
    connect(actionSaveITF, &QAction::triggered, this, [this] {saveITFFile();});
    connect(actionSaveBMP, &QAction::triggered, this, [this] {saveBMPFile();});
    //connect(actionSaveITF, &QAction::triggered, this, [this] {itfFile->writeITF();});
    connect(actionLoadVAC, &QAction::triggered, this, &ProgWindow::openVAC);
    connect(actionSaveVAC, &QAction::triggered, this, [this] {vacFile->tempWrite();});
    //connect(actionSaveBMP, &QAction::triggered, this, [this] {itfFile->writeBMP();});
    connect(actionLoadTMD, &QAction::triggered, this, [this] {openDefinition(false);});
    connect(actionLoadBMD, &QAction::triggered, this, [this] {openDefinition(true);});
    connect(actionLoadTDB, &QAction::triggered, this, [this] {openDatabase(false);});
    connect(actionLoadBDB, &QAction::triggered, this, [this] {openDatabase(true);});
    connect(actionSaveTMD, &QAction::triggered, this, [this] {saveDefinitionFile(false);});
    connect(actionSaveBMD, &QAction::triggered, this, [this] {saveDefinitionFile(true);});
    connect(actionSaveBDB, &QAction::triggered, this, [this] {saveDatabaseFile(true);});
    connect(actionOpenCalculator, &QAction::triggered, this, &ProgWindow::openWarpgateCalculator);
    /*going to need a ProgWindow function for writing TMD and BMD files since they're in lists
    use this:
    bool cancelled;
    QInputDialog::getInt(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal,0, parent->tmdFile.size(), 1, &cancelled);*/
    //connect(actionSaveTDB, &QAction::triggered, this, [this] {tdbFile[0].writeData();});
    connect(actionSaveTDB, &QAction::triggered, this, &ProgWindow::saveDatabaseFile);
    //uncomment once this function actually exists
    //connect(actionSaveTDB, &QAction::triggered, this, [this] {tdbFile->writeData();});

    setWindowState(Qt::WindowMaximized);
}

ProgWindow::~ProgWindow()
{
    delete ui;
}

void ProgWindow::openWarpgateCalculator(){
    changeMode(6);
    if(ButtonCalculate == nullptr){
        ButtonCalculate = new QPushButton("Calculate", this);
        ButtonCalculate->setGeometry(QRect(QPoint(50,320), QSize(150,30)));
        connect(ButtonCalculate, &QPushButton::released, this, [this] {warpgateCalculator->calculateWarpgateDistance();});
        ButtonCalculate->show();
    }
    if(CalculateXValue == nullptr){
        CalculateXValue = new QLineEdit("X Value", this);
        CalculateXValue->setGeometry(QRect(QPoint(200,320), QSize(150,30)));
        CalculateXValue->show();
    }
    if(CalculateYValue == nullptr){
        CalculateYValue = new QLineEdit("Y Value", this);
        CalculateYValue->setGeometry(QRect(QPoint(350,320), QSize(150,30)));
        CalculateYValue->show();
    }
    if(CalculateZValue == nullptr){
        CalculateZValue = new QLineEdit("Z Value", this);
        CalculateZValue->setGeometry(QRect(QPoint(500,320), QSize(150,30)));
        CalculateZValue->show();
    }

}

void ProgWindow::changeName(QString newName){
    if(LabelName == nullptr){
        LabelName = new QLabel("", this);
        LabelName->setGeometry(QRect(QPoint(50,100), QSize(150,30)));
        LabelName->setStyleSheet("QLabel { background-color: rgb(105,140,187) }");
        LabelName->show();
    }
    LabelName->setText(newName);
}

int ProgWindow::changeMode(int newMode){
    if(LabelMode == nullptr){
        LabelMode = new QLabel("", this);
        LabelMode->setGeometry(QRect(QPoint(50,50), QSize(150,30)));
        LabelMode->setStyleSheet("QLabel { background-color: rgb(105,140,187) }");
        LabelMode->show();
    }
    if (newMode != mode){
        switch(newMode){
        case 0: mode = 0;
            LabelMode->setText("No Mode Active");
            break; //default mode - no file type
        case 1: mode = 1;
            LabelMode->setText("Texture Mode");
            break; //ITF, Texture mode
        case 2: mode = 2;
            LabelMode->setText("Model Mode");
            break; //VBIN, actor model mode
        case 3: mode = 3;
            LabelMode->setText("Level Mode");
            break; //VBIN.mesh, level model mode
        case 4: mode = 4;
            LabelMode->setText("Database Mode");
            break; //TMD, TDB, BMD, BDB, Database mode
        case 5: mode = 5;
            LabelMode->setText("Tone Mode");
            break; //TLA, TLB, VAC, tone mode
        case 6: mode = 6;
            LabelMode->setText("Calculator Mode");

        default: return -1; //undefined type
        }
    }
    return 0;
}

void ProgWindow::createAnimationList(AnimationSourceSet animations){
    if(ListAnimation == nullptr){
           qDebug() << Q_FUNC_INFO << "The dropdown does not exist.";
        ListAnimation = new QComboBox(this);
        ListAnimation -> setGeometry(QRect(QPoint(650,50), QSize(150,30)));
        if (animations.sourceCount <= 0){
            ListAnimation->insertItem(0, "No animations");
        } else {
            for(int i=0; i<animations.sourceCount; ++i){
                ListAnimation->insertItem(i, animations.streamArray[i]->name);
            }
        }
        connect(ListAnimation, &QComboBox::currentIndexChanged, this, [this] {animationSelectChange();});
        ListAnimation->show();
    } else {
        ListAnimation->clear();
        ListAnimation -> setGeometry(QRect(QPoint(650,50), QSize(150,30)));
        if (animations.sourceCount <= 0){
            ListAnimation->insertItem(0, "No animations");
        } else {
            for(int i=0; i<animations.sourceCount; ++i){
                ListAnimation->insertItem(i, animations.streamArray[i]->name);
            }
        }
        ListAnimation->show();
    }
    qDebug() << Q_FUNC_INFO << "animation list size" << animations.streamArray.size() << "name" << animations.name;
    if (animations.streamArray.size() > 0) {
        qDebug() << Q_FUNC_INFO << "channel array size" << animations.streamArray[0]->channelArray.size() << "name" << animations.streamArray[0]->name;
        if (animations.streamArray[0]->channelArray.size() > 0) {
            qDebug() << Q_FUNC_INFO << "keyframe count for channel" << animations.streamArray[0]->channelArray[0]->name << ":" << animations.streamArray[0]->channelArray[0]->keyframeCount;
            createFrameList(animations.streamArray[0]->channelArray[0]->keyframeCount);
        }
    }
}

void ProgWindow::createFrameList(int frames){
    qDebug() << Q_FUNC_INFO << "CREATING THE LIST WITH " << frames << "LEVELS";
    if(ListFrame == nullptr){
           qDebug() << Q_FUNC_INFO << "The dropdown does not exist.";
        ListFrame = new QComboBox(this);
        ListFrame -> setGeometry(QRect(QPoint(850,50), QSize(150,30)));
        if (frames <= 0){
            ListFrame->insertItem(0, "1");
        } else {
            for(int i=0; i<frames; ++i){
                ListFrame->insertItem(i, QString::number(i+1));
            }
        }
        ListFrame->show();
    } else {
        ListFrame->clear();
        ListFrame -> setGeometry(QRect(QPoint(850,50), QSize(150,30)));
        if (frames <= 0){
            ListFrame->insertItem(0, "1");
        } else {
            for(int i=0; i<frames; ++i){
                ListFrame->insertItem(i, QString::number(i+1));
            }
        }
        ListFrame->show();
    }
}

void ProgWindow::levelSelectChange(){
    if (fileMode == "ITF"){
        itfFiles[ListFiles->currentIndex()].populatePalette();
    }
}

void ProgWindow::fileSelectChange(){
    if (fileMode == "VBIN" && ListFiles->currentIndex() >= 0) {
        createLevelList(vbinFiles[ListFiles->currentIndex()].highestLOD);
    } else if (fileMode == "ITF" && ListFiles->currentIndex() >= 0) {
        createLevelList(itfFiles[ListFiles->currentIndex()].paletteCount);
        ListLevels->setCurrentIndex(0);
        itfFiles[ListFiles->currentIndex()].populatePalette();
    }
}

void ProgWindow::animationSelectChange(){
    if (fileMode == "VBIN" && ListAnimation->currentIndex() >= 0) {
        createFrameList(vbinFiles[ListFiles->currentIndex()].animationSet.streamArray[ListAnimation->currentIndex()]->channelArray[0]->keyframeCount);
    }
}


void ProgWindow::createMultiRadios(){
    if(radioSingle == nullptr){
        radioSingle = new QRadioButton("Single file output", this);
        radioSingle -> setGeometry(QRect(QPoint(340,120), QSize(200,30)));
        radioSingle->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
        radioSingle-> toggle();
        radioSingle->show();
    }
    if(radioMultiple == nullptr){
        radioMultiple = new QRadioButton("Multi-file output", this);
        radioMultiple -> setGeometry(QRect(QPoint(540,120), QSize(120,30)));
        radioMultiple->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
        radioMultiple->show();
    }
}

void ProgWindow::deleteMultiRadios(){
    if(radioSingle != nullptr){
        delete(radioSingle);
        radioSingle = nullptr;
    }
    if(radioMultiple != nullptr){
        delete(radioMultiple);
        radioMultiple = nullptr;
    }
}

void ProgWindow::createTable(int rows, int columns){
    if(PaletteTable == nullptr){
        qDebug() << "The table does not already exist.";
        PaletteTable = new QTableWidget(rows, columns, this);
        PaletteTable->setGeometry(QRect(QPoint(50,250), QSize(125*columns,300)));
        connect(PaletteTable, &QTableWidget::cellChanged, this, [this](int row, int column) {itfFiles[ListFiles->currentIndex()].editPalette(row, column);});
        PaletteTable->show();
    } else {
        qDebug() << "The table already exists.";
        PaletteTable->setGeometry(QRect(QPoint(50,250), QSize(125*columns,300)));
        PaletteTable->show();
        //clear and resize table
    }
}

void ProgWindow::createLevelList(int levels){
    qDebug() << Q_FUNC_INFO << "CREATING THE LIST WITH " << levels << "LEVELS";
    if(ListLevels == nullptr){
           qDebug() << Q_FUNC_INFO << "The dropdown does not exist.";
        ListLevels = new QComboBox(this);
        ListLevels -> setGeometry(QRect(QPoint(250,50), QSize(150,30)));
        if (levels <= 0){
            ListLevels->insertItem(0, "1");
        } else {
            for(int i=0; i<levels; ++i){
                ListLevels->insertItem(i, QString::number(i+1));
            }
        }
        connect(ListLevels, &QComboBox::currentIndexChanged, this, [this] {levelSelectChange();});
        ListLevels->show();
    } else {
        ListLevels->clear();
        ListLevels -> setGeometry(QRect(QPoint(250,50), QSize(150,30)));
        if (levels <= 0){
            ListLevels->insertItem(0, "1");
        } else {
            for(int i=0; i<levels; ++i){
                ListLevels->insertItem(i, QString::number(i+1));
            }
        }
        ListLevels->show();
    }
}

void ProgWindow::createFileList(){
    if(ListFiles == nullptr){
           qDebug() << Q_FUNC_INFO << "The dropdown does not exist.";
        ListFiles = new QComboBox(this);
        ListFiles -> setGeometry(QRect(QPoint(450,50), QSize(150,30)));
        if(fileMode == "VBIN"){
            for (int i = 0; i < vbinFiles.size(); i++) {
                ListFiles->insertItem(i, vbinFiles[i].fileName);
            }
        } else if (fileMode == "ITF"){
            for (int i = 0; i < itfFiles.size(); i++) {
                ListFiles->insertItem(i, itfFiles[i].fileName);
            }
        }

        ListFiles->setCurrentIndex(ListFiles->count()-1);
        connect(ListFiles, &QComboBox::currentIndexChanged, this, [this] {fileSelectChange();});
        ListFiles->show();
    } else {
        ListFiles->clear();
        ListFiles -> setGeometry(QRect(QPoint(450,50), QSize(150,30)));
        if(fileMode == "VBIN"){
            for (int i = 0; i < vbinFiles.size(); i++) {
                ListFiles->insertItem(i, vbinFiles[i].fileName);
            }
        } else if (fileMode == "ITF"){
            for (int i = 0; i < itfFiles.size(); i++) {
                ListFiles->insertItem(i, itfFiles[i].fileName);
            }
        }
        ListFiles->setCurrentIndex(ListFiles->count()-1);
        ListFiles->show();
    }
}

void ProgWindow::clearWindow(){
    if (PaletteTable != nullptr) {
        PaletteTable->clear();
        PaletteTable->hide();
    }
//    if (ListLevels != nullptr) {
//        ListLevels->clear();
//        ListLevels->hide();
//    }
//    if (ListFiles != nullptr){
//        ListFiles->clear();
//        ListFiles->hide();
//    }
}

void ProgWindow::createDBButtons(){
    if(ButtonEditDB == nullptr){
        ButtonEditDB = new QPushButton("Edit item Data", this);
        ButtonEditDB->setGeometry(QRect(QPoint(50,320), QSize(150,30)));
        connect(ButtonEditDB, &QPushButton::released, this, [this]{editDatabaseItem(testView->currentIndex(), testView->currentIndex().row());});
        ButtonEditDB->show();
        //I don't think this button is really that useful
        /*ButtonRemoveItem = new QPushButton("Remove item", this);
        ButtonRemoveItem->setGeometry(QRect(QPoint(50,370), QSize(150,30)));
        connect(ButtonRemoveItem, &QPushButton::released, this, [this]{removeDatabaseItem(testView->currentIndex(), testView->currentIndex().row());});
        ButtonRemoveItem->show();*/
        ButtonRemoveClass = new QPushButton("Remove class/instance", this);
        ButtonRemoveClass->setGeometry(QRect(QPoint(50,420), QSize(150,30)));
        connect(ButtonRemoveClass, &QPushButton::released, this, [this]{removeDatabaseClass(testView->currentIndex());});
        ButtonRemoveClass->show();
    }
}

void ProgWindow::removeDatabaseClass(QModelIndex item){
    for(int i = 0; i < definitions.size(); i++){
        if(item.parent().data().toString() == definitions[i].fileName){
            definitions[i].removeClass(item.row());
        }
    }
    for(int i = 0; i < databases.size(); i++){
        if(item.parent().data().toString() == databases[0].fileName){
            databases[i].removeInstance(item.row());
        }
    }
    testModel->removeRows(item.row(), 1, item.parent());
}

void ProgWindow::removeDatabaseItem(QModelIndex item, int itemIndex){
    for(int i = 0; i < definitions.size(); i++){
        if(item.parent().parent().data().toString() == definitions[i].fileName){
            definitions[i].removeItem(item.parent().data().toString(), itemIndex);
        }
    }
    for(int i = 0; i < databases.size(); i++){
        if(item.parent().parent().data().toString() == databases[0].fileName){
            databases[i].removeItem(item.parent().siblingAtColumn(1).data().toInt(), itemIndex);
        }
    }

    testModel->removeRows(item.row(), 1, item.parent());

    //then update DB Tree
}

void ProgWindow::editDatabaseItem(QModelIndex item, int itemIndex){
    QStringList newValue; //using a qstringlist to make things easier on myself
    for(int i = 0; i < definitions.size(); i++){
        if(item.parent().parent().data().toString() == definitions[i].fileName){
            newValue = definitions[i].editItem(item.parent().data().toString(), itemIndex);
        }
    }
    for(int i = 0; i < databases.size(); i++){
        if(item.parent().parent().data().toString() == databases[i].fileName){
            newValue = databases[i].editItem(item.parent().siblingAtColumn(1).data().toInt(), itemIndex);
        }
    }

    if(newValue[0] == "SINGLEVALUE"){
        testModel->setData(item.siblingAtColumn(2), newValue[1]);
    } else if (newValue[0] == "ENUM"){
        testModel->setData(item.siblingAtColumn(2), newValue[1]);
        newValue.remove(0,2);
        testModel->setData(item.siblingAtColumn(3), newValue.join(','));
    } else if (newValue[0] == "SETDEFAULT"){
        testModel->setData(item.siblingAtColumn(4), true);
    } else {
        testModel->setData(item.siblingAtColumn(3), newValue.join(','));
    }

}

void ProgWindow::resizeEvent(QResizeEvent* event){
    /*Resizes the background to fit the window. Will eventually add element placements so it doesn't look terrible if full-screened.*/
    QMainWindow::resizeEvent(event);
    hSize = this->size().width();
    vSize = this->size().height();
    //qDebug() << Q_FUNC_INFO << "hsize" << hSize << "vSize" << vSize;
    menuMain->setGeometry(QRect(QPoint(int(hSize*0),int(vSize*0)), QSize(int(hSize*1),25)));
    background.load(QCoreApplication::applicationDirPath() + "/assets/background.png");
    background = background.scaled(this->size());
    palette.setBrush(QPalette::Window, background);
    this->setPalette(palette);
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

void ProgWindow::saveDefinitionFile(bool binary){
    if(definitions.empty()){
       messageError("No definition files available to save. Please load a definition file.");
       return;
    }

    bool cancelled;
    QStringList options;
    for(int i = 0; i < definitions.size(); i++){
        options.append(definitions[i].fileName);
    }

    QString chosenFile = QInputDialog::getItem(this, this->tr("Select TMD File:"), this->tr("File Name:"), options, 0, false, &cancelled);
    qDebug() << Q_FUNC_INFO << "chosen file:" << chosenFile << cancelled;

    if(!cancelled){
        return;
    }

    for(int i = 0; i < definitions.size(); i++){
        if(definitions[i].fileName == chosenFile){
            if(binary){
                definitions[i].writeBinary();
            } else {
                definitions[i].writeText();
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "Definition output complete.";

}


void ProgWindow::saveDatabaseFile(bool binary){
    if(databases.empty()){
       messageError("No database files available to save. Please load a database file.");
       return;
    }

    bool cancelled;
    QStringList options;
    for(int i = 0; i < databases.size(); i++){
        options.append(databases[i].fileName);
    }

    QString chosenFile = QInputDialog::getItem(this, this->tr("Select TDB File:"), this->tr("File Name:"), options, 0, false, &cancelled);
    qDebug() << Q_FUNC_INFO << "chosen file:" << chosenFile << cancelled;

    if(!cancelled){
        return;
    }

    for(int i = 0; i < databases.size(); i++){
        if(databases[i].fileName == chosenFile){
            if(binary){
                databases[i].writeBinary();
            } else {
                databases[i].writeText();
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "Database output complete.";

}

void ProgWindow::saveITFFile(){
    if(itfFiles.empty()){
       messageError("No texture files available to save. Please load a texture file.");
       return;
    }
    itfFiles[ListFiles->currentIndex()].writeITF();
}

void ProgWindow::saveBMPFile(){
    if(itfFiles.empty()){
       messageError("No texture files available to export. Please load a texture file.");
       return;
    }
    itfFiles[ListFiles->currentIndex()].writeBMP();
}
