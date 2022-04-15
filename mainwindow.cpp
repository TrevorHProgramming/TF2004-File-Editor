#include "mainwindow.h"
#include "ui_mainwindow.h"



ProgWindow::ProgWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    highestLOD = 0;

    vbinFile = new VBIN;
    vbinFile->parent = this;
    itfFile = new ITF;
    itfFile->parent = this;
    geometrySet = new GeometrySet;
    geometrySet->parent = this;
    PaletteTable = nullptr;

    ButtonVBINtoSTL = new QPushButton("Convert VBIN to STL", this);
    ButtonVBINtoSTL -> setGeometry(QRect(QPoint(50,50), QSize(150,30)));
    ButtonOpenVBIN = new QPushButton("Open VBIN File", this);
    ButtonOpenVBIN -> setGeometry(QRect(QPoint(50,20), QSize(150,30)));
    ButtonMeshtoSTL = new QPushButton("Convert .Mesh.VBIN to STL", this);
    ButtonMeshtoSTL -> setGeometry(QRect(QPoint(50,110), QSize(150,30)));
    ButtonOpenMeshVBIN = new QPushButton("Open .Mesh.VBIN File", this);
    ButtonOpenMeshVBIN -> setGeometry(QRect(QPoint(50,80), QSize(150,30)));
    ButtonSaveITF = new QPushButton("Save ITF File", this);
    ButtonSaveITF -> setGeometry(QRect(QPoint(50,200), QSize(150,30)));
    ButtonITFtoBMP = new QPushButton("Convert ITF to BMP", this);
    ButtonITFtoBMP -> setGeometry(QRect(QPoint(50,170), QSize(150,30)));
    ButtonOpenITF = new QPushButton("Open ITF File", this);
    ButtonOpenITF -> setGeometry(QRect(QPoint(50,140), QSize(150,30)));
    ListLODLevels = new QComboBox(this);
    ListLODLevels -> setGeometry(QRect(QPoint(250,50), QSize(150,30)));
    radioSingle = new QRadioButton("Single file output", this);
    radioSingle -> setGeometry(QRect(QPoint(440,120), QSize(100,30)));
    radioMultiple = new QRadioButton("Multi-file output", this);
    radioMultiple -> setGeometry(QRect(QPoint(540,120), QSize(120,30)));
    radioSingle-> toggle();
    //PaletteTable = new QTableView(this);
    //PaletteTable->setGeometry(QRect(QPoint(250,250), QSize(150,30)));

    //createTable(4,4, *this);

    //try a QTableView for palette editing
    //check for max 255/min 0 value
    //then try QImage for the bmp display

    connect(ButtonVBINtoSTL, &QPushButton::released, this, &ProgWindow::convertVBINToSTL);
    connect(ButtonOpenVBIN, &QPushButton::released, this, &ProgWindow::openVBIN);
    connect(ButtonOpenMeshVBIN, &QPushButton::released, [this] {geometrySet->openMeshVBINFile();});
    //connect(ButtonITFtoBMP, &QPushButton::released, this, &ProgWindow::convertITFToBMP);
    connect(ButtonOpenITF, &QPushButton::released, this, &ProgWindow::openITF);
    //connect(ButtonSaveITF, &QPushButton::released, this, &ProgWindow::saveITFPalette);
}

ProgWindow::~ProgWindow()
{
    delete ui;
}


void ProgWindow::createTable(int rows, int columns){
    if(this->PaletteTable == nullptr){
        qDebug() << "The table does not already exist.";
        PaletteTable = new QTableWidget(rows, columns, this);
        PaletteTable->setGeometry(QRect(QPoint(50,250), QSize(125*columns,300)));
    } else {
        qDebug() << "The table already exists.";
        //clear and resize table
    }
}




