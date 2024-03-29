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
#include <QImageWriter>

#include <winsparkle.h>
#include <vector>

#include "Headers/Models/vbin.h"
#include "Headers/Textures/itf.h"
#include "Headers/Models/Mesh.h"
#include "BinChanger.h"
#include "CustomQT.h"
#include "Headers/Models/LevelGeo.h"
#include "Headers/Databases/Database.h"
#include "Headers/Audio/ToneLibraries.h"
#include "Headers/Databases/DistanceCalculator.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SettingsWindow : public QWidget {
    Q_OBJECT

public:
    explicit SettingsWindow(ProgWindow *sentParent = nullptr);
    ~SettingsWindow();
    ProgWindow *parent;
    QStringList settingsNames = {"Starting window height","Starting window width","Address offset","Starting address","Long scroll length","Short scroll length"};
    QStringList settingsValues = {"1024","768","0","0","16","4"};
    QTableWidget *settingsEdit;

    void open();
    void writeSettings(QFile *outputFile);
    void changeSetting(int row, int column);

private slots:
    void updateSettings();
private:
    QPushButton *sendUpdate;
    bool savedChanges;
};

class ProgWindow : public QMainWindow
{
    Q_OBJECT

public:
    ProgWindow(QWidget *parent = nullptr);
    ~ProgWindow();

    BinChanger binChanger;
    VACFile* vacFile;
    DistanceCalculator* warpgateCalculator;
    SettingsWindow *setW;
    std::vector<std::shared_ptr<TFFile>> loadedFiles;
    QStringList loadedFileNames;
    QListWidget* fileBrowser;
    int hSize = 1200;
    int vSize = 768;
    double version = 6.6;

    QString fileMode;

    Ui::MainWindow *ui;
    QLabel *ClosestWarpgate;

    std::vector<QWidget*> currentModeWidgets;
    QDockWidget *rightSidebar;
    QDockWidget *leftSidebar;
    QListWidget* logPrintout;

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
    void log(QString message);

    void clearWindow();
    void clearFiles();
    void clearLog();

    std::shared_ptr<TFFile> matchFile(QString fileNameFull);

    //unprivated for cross-file loading
//private:
    void resizeEvent(QResizeEvent* event);
    void openWarpgateCalculator();
    void updateBackground();
    void updateCenter();
    void saveFile(QString fromType, QString givenPath = "");
    void bulkSave(QString category);
    void handleSettings();

    void bulkOpen(QString fileType);
    template <typename theFile>
    void loadBulkFile(theFile fileToOpen);

    template <typename theFile>
    void loadFile(theFile fileToOpen, QString givenPath = "");
    void openFile(QString fileType, QString givenPath = "");

};


#endif // MAINWINDOW_H
