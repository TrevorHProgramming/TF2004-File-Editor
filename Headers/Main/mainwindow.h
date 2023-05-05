#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFile>
#include <QIODevice>
#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QRadioButton>
#include <QHeaderView>
#include <QQuaternion>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>
#include <QDockWidget>


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

    BinChanger binChanger;
    VACFile* vacFile;
    DistanceCalculator* warpgateCalculator;
    std::vector<std::shared_ptr<TFFile>> loadedFiles;
    QListWidget* fileBrowser;
    int hSize = 1200;
    int vSize = 768;
    double version = 5.5;

    QString fileMode;

    Ui::MainWindow *ui;
    QLabel *ClosestWarpgate;

    std::vector<QWidget*> currentModeWidgets;
    QDockWidget *rightSidebar;
    QDockWidget *leftSidebar;

    QWidget *centralContainer;
    //QPushButton *ButtonRemoveItem;
    QPushButton *ButtonCalculate;
    QLineEdit *CalculateXValue;
    QLineEdit *CalculateYValue;
    QLineEdit *CalculateZValue;
    QLineEdit *DBNewValue;
    QComboBox *ListAnimation;
    QComboBox *ListFrame;
    FileData fileData;
    QMessageBox *MessagePopup;
    int numColors;
    QString whichModel;
    std::vector<std::vector<std::vector<int>>> lodArrays;
    std::vector<int> usedIndecies;
    QPixmap background;
    QPalette palette;


    void messageError(QString message);
    void messageSuccess(QString message);

    void clearWindow();
    std::shared_ptr<TFFile> matchFile(QString fileNameFull);

private:
    void resizeEvent(QResizeEvent* event);
    void openWarpgateCalculator();
    void updateBackground();
    void updateCenter();
    void saveFile(QString fromType);
    void bulkSave(QString category);

    void bulkOpen(QString fileType);
    template <typename theFile>
    void loadBulkFile(theFile fileToOpen);

    template <typename theFile>
    void loadFile(theFile fileToOpen);
    void openFile(QString fileType);

};


#endif // MAINWINDOW_H
