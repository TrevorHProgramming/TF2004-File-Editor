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


#include <vector>

#include "vbin.h"
#include "itf.h"
#include "Mesh.h"
#include "BinChanger.h"
#include "LevelGeo.h"
#include "Database.h"


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
    VBIN* vbinFile;
    ITF* itfFile;
    std::vector<DefinitionFile> definitions;
    std::vector<DatabaseFile> databases;
    GeometrySet* geometrySet;
    int hSize = 1200;
    int vSize = 768;

    QString fileMode;

    Ui::MainWindow *ui;

    QPushButton *ButtonVBINtoSTL;
    QPushButton *ButtonOpenVBIN;
    QPushButton *ButtonOpenMeshVBIN;
    QPushButton *ButtonMeshtoSTL;
    QPushButton *ButtonOpenITF;
    QPushButton *ButtonITFtoBMP;
    QPushButton *ButtonOpenTMD;
    QPushButton *ButtonOpenTDB;
    QPushButton *ButtonWriteTMD;
    QPushButton *ButtonClear;
    QPushButton *ButtonEditDB;
    QPushButton *ButtonRemoveItem;
    QPushButton *ButtonRemoveClass;
    QLineEdit *DBNewValue;
    QRadioButton *radioSingle;
    QRadioButton *radioMultiple;
    QComboBox *ListLevels;
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
    void createDropdown(int levels);
    void createMultiRadios();
    void deleteMultiRadios();
    void createDBButtons();
    void dropdownSelectChange();
    void clearWindow();

private:
    void resizeEvent(QResizeEvent* event);
    void editDatabaseItem(QModelIndex item, int itemIndex);
    void removeDatabaseItem(QModelIndex item, int itemIndex);
    void removeDatabaseClass(QModelIndex item);
    void saveDefinitionFile(bool binary);
    void saveDatabaseFile(bool binary);

public slots:
    void convertVBINToSTL();
    //void convertITFToBMP();
    //void convertBMPtoPNG(QString bmpPath);
    void openVBIN();
    void openITF();
    void openDefinition(bool binary);
    void openDatabase(bool binary);
    /*void openTMD();
    void openTDB();
    void openBMD();
    void openBDB();*/
};


#endif // MAINWINDOW_H
