#include "mainwindow.h"
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
    menuMain->setGeometry(QRect(QPoint(int(hSize*0),int(vSize*0)), QSize(int(hSize*1),int(vSize*0.03))));

    fileData.parent = this;

    QMenu *menuVBIN = menuMain->addMenu("VBIN");
    QMenu *menuITF = menuMain->addMenu("ITF");
    QMenu *menuDatabase = menuMain -> addMenu("Database");
    QMenu *menuSettings = menuMain->addMenu("Settings");
    QAction *actionLoadVBIN = menuVBIN->addAction("Load VBIN");
    QAction *actionSaveSTL = menuVBIN ->addAction("Export to STL");
    QAction *actionLoadMeshVBIN = menuVBIN->addAction("Load Mesh VBIN");
    QAction *actionLoadITF = menuITF ->addAction("Load ITF");
    QAction *actionSaveITF = menuITF->addAction("Save ITF");
    QAction *actionSaveBMP = menuITF ->addAction("Export to BMP");
    QAction *actionLoadTMD = menuDatabase ->addAction("Load TMD");
    QAction *actionLoadTDB = menuDatabase ->addAction("Load TDB");
    QAction *actionLoadBMD = menuDatabase ->addAction("Load BMD");
    QAction *actionLoadBDB = menuDatabase ->addAction("Load BDB");
    QAction *actionSaveTMD = menuDatabase ->addAction("Save TMD");
    QAction *actionSaveTDB = menuDatabase ->addAction("Save TDB");
    QAction *actionSaveBMD = menuDatabase ->addAction("Save BMD");
    QAction *actionSaveBDB = menuDatabase ->addAction("Save BDB");
    QAction *actionSettings = menuSettings -> addAction("Settings");

    vbinFile = new VBIN;
    vbinFile->parent = this;
    itfFile = new ITF;
    itfFile->parent = this;
    //tmdFile.push_back(new TMDFile);
    //tmdFile[0]->parent = this;
    geometrySet = new GeometrySet;
    geometrySet->parent = this;
    PaletteTable = nullptr;
    ListLevels = nullptr;
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

    connect(actionSaveSTL, &QAction::triggered, this, &ProgWindow::convertVBINToSTL);
    connect(actionLoadVBIN, &QAction::triggered, this, &ProgWindow::openVBIN);
    connect(actionLoadMeshVBIN, &QAction::triggered, this, [this] {geometrySet->openMeshVBINFile();});
    //connect(ButtonITFtoBMP, &QPushButton::released, this, &ProgWindow::convertITFToBMP);
    connect(actionLoadITF, &QAction::triggered, this, &ProgWindow::openITF);
    connect(actionSaveITF, &QAction::triggered, this, [this] {itfFile->writeITF();});
    connect(actionSaveBMP, &QAction::triggered, this, [this] {itfFile->writeBMP();});
    connect(actionLoadTMD, &QAction::triggered, this, [this] {openDefinition(false);});
    connect(actionLoadBMD, &QAction::triggered, this, [this] {openDefinition(true);});
    connect(actionLoadTDB, &QAction::triggered, this, [this] {openDatabase(false);});
    connect(actionLoadBDB, &QAction::triggered, this, [this] {openDatabase(true);});
    connect(actionSaveTMD, &QAction::triggered, this, [this] {saveDefinitionFile(false);});
    connect(actionSaveBMD, &QAction::triggered, this, [this] {saveDefinitionFile(true);});
    connect(actionSaveBDB, &QAction::triggered, this, [this] {saveDatabaseFile(true);});
    /*going to need a ProgWindow function for writing TMD and BMD files since they're in lists
    use this:
    bool cancelled;
    QInputDialog::getInt(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal,0, parent->tmdFile.size(), 1, &cancelled);*/
    //connect(actionSaveTDB, &QAction::triggered, this, [this] {tdbFile[0].writeData();});
    connect(actionSaveTDB, &QAction::triggered, this, &ProgWindow::saveDatabaseFile);
    //uncomment once this function actually exists
    //connect(actionSaveTDB, &QAction::triggered, this, [this] {tdbFile->writeData();});
}

ProgWindow::~ProgWindow()
{
    delete ui;
}

void ProgWindow::dropdownSelectChange(){
    if (fileMode == "ITF"){
        itfFile->populatePalette();
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
        connect(PaletteTable, &QTableWidget::cellChanged, this, [this](int row, int column) {itfFile->editPalette(row, column);});
        PaletteTable->show();
    } else {
        qDebug() << "The table already exists.";
        PaletteTable->setGeometry(QRect(QPoint(50,250), QSize(125*columns,300)));
        PaletteTable->show();
        //clear and resize table
    }
}

void ProgWindow::createDropdown(int levels){
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
        connect(ListLevels, &QComboBox::currentIndexChanged, this, [this] {dropdownSelectChange();});
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
        if(item.parent().parent().data().toString() == databases[0].fileName){
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

