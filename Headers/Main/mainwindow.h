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
#include <QInputDialog>
#include <QRadioButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QQuaternion>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QTreeView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>


#include <vector>

#include "Headers/Models/vbin.h"
#include "Headers/Textures/itf.h"
#include "Headers/Models/Mesh.h"
#include "BinChanger.h"
#include "Headers/Models/LevelGeo.h"
#include "Headers/Databases/Database.h"
#include "Headers/Audio/ToneLibraries.h"
#include "DistanceCalculator.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ProgWindow : public QMainWindow
{
    Q_OBJECT

public:
    ProgWindow(QWidget *parent = nullptr);
    ~ProgWindow();
    QMenuBar *menuMain;

    BinChanger binChanger;
    std::vector<VBIN> vbinFiles;
    std::vector<ITF> itfFiles;
    //VBIN* vbinFile;
    //ITF* itfFile;
    VACFile* vacFile;
    DistanceCalculator* warpgateCalculator;
    std::vector<DefinitionFile> definitions;
    std::vector<DatabaseFile> databases;
    MeshVBIN* levelGeo;
    int hSize = 1200;
    int vSize = 768;
    int mode = 0;
    double version = 5.5;

    QString fileMode;

    Ui::MainWindow *ui;

    QLabel *LabelMode;
    QLabel *LabelName;
    QLabel *ClosestWarpgate;

    std::vector<QObject*> currentModeObjects;


    QPushButton *ButtonRemoveItem;
    QPushButton *ButtonCalculate;
    QLineEdit *CalculateXValue;
    QLineEdit *CalculateYValue;
    QLineEdit *CalculateZValue;
    QLineEdit *DBNewValue;
    QRadioButton *radioSingle;
    QRadioButton *radioMultiple;
    QComboBox *ListLevels;
    QComboBox *ListFiles;
    QComboBox *ListAnimation;
    QComboBox *ListFrame;
    QPushButton *ButtonSaveITF;
    FileData fileData;
    QTableWidget *PaletteTable;
    QListWidget *DBClassList;
    QListWidget *DBItemList;
    QListWidget *DBDetailList;
    QListWidget *DBValueList;
    QMessageBox *MessagePopup;
    int numColors;
    QString whichModel;
    std::vector<std::vector<std::vector<int>>> lodArrays;
    std::vector<int> usedIndecies;
    QPixmap background;
    QPalette palette;

    QTreeView *testView;
    QStandardItemModel *testModel;


    void messageError(QString message);
    void messageSuccess(QString message);

    void createTable(int rows, int columns);
    void createLevelList(int levels);
    void createFileList();
    void createAnimationList(AnimationSourceSet animations);
    void createFrameList(int frames);
    void createMultiRadios();
    void deleteMultiRadios();
    void createDBButtons();
    void levelSelectChange();
    void fileSelectChange();
    void animationSelectChange();
    void clearWindow();
    void changeName(QString newName);

private:
    void resizeEvent(QResizeEvent* event);
    void editDatabaseItem(QModelIndex item, int itemIndex);
    void removeDatabaseItem(QModelIndex item, int itemIndex);
    void removeDatabaseClass(QModelIndex item);
    void saveDefinitionFile(bool binary);
    void saveDatabaseFile(bool binary);
    void saveITFFile();
    void saveBMPFile();
    int changeMode(int newMode);
    void openWarpgateCalculator();

    template <typename theFile>
    void openFile(QString fileType);

public slots:
    void convertVBINToSTL();
    void convertVBINToDAE();
    //void convertITFToBMP();
    //void convertBMPtoPNG(QString bmpPath);

    void openVBIN();
    void openITF();
    void openVAC();
    void openDefinition(bool binary);
    void openDatabase(bool binary);
    /*void openTMD();
    void openTDB();
    void openBMD();
    void openBDB();*/
};


#endif // MAINWINDOW_H
