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
#include <QQuaternion>
#include <QLabel>


#include <vector>

#include "vbin.h"
#include "itf.h"
#include "Mesh.h"
#include "BinChanger.h"
#include "FileHandler.h"
#include "LevelGeo.h"


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
    GeometrySet* geometrySet;

    Ui::MainWindow *ui;

    QPushButton *ButtonVBINtoSTL;
    QPushButton *ButtonOpenVBIN;
    QPushButton *ButtonOpenMeshVBIN;
    QPushButton *ButtonMeshtoSTL;
    QPushButton *ButtonOpenITF;
    QPushButton *ButtonITFtoBMP;
    QRadioButton *radioSingle;
    QRadioButton *radioMultiple;
    QComboBox *ListLODLevels;
    QTableWidget *TablePalette;
    QPushButton *ButtonSaveITF;
    QByteArray fileData;
    QLabel *LabelImageTest;
    int highestLOD;
    long fileLength;
    int numColors;
    QString whichModel;
    Palette textureColors[256];
    std::vector<std::vector<std::vector<int>>> lodArrays;
    std::vector<int> usedIndecies;

public slots:
    void convertVBINToSTL();
    //void convertITFToBMP();
    //void convertBMPtoPNG(QString bmpPath);
    void openVBIN();
    //void openITF();
};


#endif // MAINWINDOW_H
