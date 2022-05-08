#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFile>
#include <QIODevice>
#include <QByteArrayMatcher>
#include <QBitArray>
#include <QVector>
#include <QComboBox>
#include <QTextStream>
#include <QFileDialog>
#include <QRadioButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QQuaternion>
#include <QLabel>


#include <vector>

#include "vbin.h"
#include "itf.h"
#include "Mesh.h"
#include "BinChanger.h"
#include "FileHandler.h"
#include "LevelGeo.h"
#include "TMD.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class ProgWindow : public QMainWindow
{
    Q_OBJECT

public:
    ProgWindow(QWidget *parent = nullptr);
    ~ProgWindow();

    BinChanger binChanger;
    VBIN* vbinFile;
    ITF* itfFile;
    TMDFile* tmdFile;
    GeometrySet* geometrySet;

    QString fileMode;

    Ui::MainWindow *ui;

    QPushButton *ButtonVBINtoSTL;
    QPushButton *ButtonOpenVBIN;
    QPushButton *ButtonOpenMeshVBIN;
    QPushButton *ButtonMeshtoSTL;
    QPushButton *ButtonOpenITF;
    QPushButton *ButtonITFtoBMP;
    QPushButton *ButtonOpenTMD;
    QRadioButton *radioSingle;
    QRadioButton *radioMultiple;
    QComboBox *ListLevels;
    QPushButton *ButtonSaveITF;
    QByteArray fileData;
    QTableWidget *PaletteTable;
    int numColors;
    QString whichModel;
    std::vector<std::vector<std::vector<int>>> lodArrays;
    std::vector<int> usedIndecies;
    QPixmap background;
    QPalette palette;


    void createTable(int rows, int columns);
    void createDropdown(int levels);
    void createMultiRadios();
    void deleteMultiRadios();
    void dropdownSelectChange();

private:
    void resizeEvent(QResizeEvent* event);

public slots:
    void convertVBINToSTL();
    //void convertITFToBMP();
    //void convertBMPtoPNG(QString bmpPath);
    void openVBIN();
    void openITF();
    void openTMD();
};


#endif // MAINWINDOW_H
