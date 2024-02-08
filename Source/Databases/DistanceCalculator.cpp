#include "Headers/Main/mainwindow.h"

void DistanceCalculator::userSelectLevel(int selectedLevel){
    //called when user makes a selection on the level list dropdown
    currentLevel = selectedLevel;
    warpgateList = databaseList[currentLevel]->sendWarpgates();
    //then currentlevel is used below to calculate the closest warpgate
}

DistanceCalculator::DistanceCalculator(ProgWindow *parentPass){
    parent = parentPass;
    qDebug() << Q_FUNC_INFO << "parent value properly passed, window width is" << parent->hSize;
    loadWarpgates();
    parent->clearWindow();
    QPushButton *ButtonCalculate = new QPushButton("Calculate", parent->centralContainer);
    ButtonCalculate->setGeometry(QRect(QPoint(50,320), QSize(150,30)));
    QAbstractButton::connect(ButtonCalculate, &QPushButton::released, parent, [this] {calculateWarpgateDistance();});
    parent->currentModeWidgets.push_back(ButtonCalculate);
    ButtonCalculate->show();

    inputXValue = new QLineEdit("X Value", parent->centralContainer);
    inputXValue->setGeometry(QRect(QPoint(200,320), QSize(150,30)));
    parent->currentModeWidgets.push_back(inputXValue);
    inputXValue->show();

    inputYValue = new QLineEdit("Y Value", parent->centralContainer);
    inputYValue->setGeometry(QRect(QPoint(350,320), QSize(150,30)));
    parent->currentModeWidgets.push_back(inputYValue);
    inputYValue->show();

    inputZValue = new QLineEdit("Z Value", parent->centralContainer);
    inputZValue->setGeometry(QRect(QPoint(500,320), QSize(150,30)));
    parent->currentModeWidgets.push_back(inputZValue);
    inputZValue->show();

    QComboBox* ListLevels = new QComboBox(parent->centralContainer);
    ListLevels -> setGeometry(QRect(QPoint(250,150), QSize(150,30)));

    for(int i=0; i<databaseList.size(); ++i){
        ListLevels->insertItem(i, QString::number(i+1));
    }

    QComboBox::connect(ListLevels, &QComboBox::currentIndexChanged, parent, [ListLevels, this] {userSelectLevel(ListLevels->currentIndex());});
    ListLevels->show();
    parent->currentModeWidgets.push_back(ListLevels);
    ListLevels->setCurrentIndex(0);
}

Warpgate::Warpgate(dictItem copyItem){
    this->x_position = copyItem.position.x();
    this->y_position = copyItem.position.y();
    this->z_position = copyItem.position.z();
    this->attributes = copyItem.attributes;
}

Warpgate::Warpgate(){
    this->x_position = 0;
    this->y_position = 0;
    this->z_position = 0;
}

void DistanceCalculator::visit(TFFile dataFile){
    qDebug() << Q_FUNC_INFO << "invalid file visited:" << dataFile.fullFileName();
}

void DistanceCalculator::visit(DatabaseFile dataFile){
    qDebug() << Q_FUNC_INFO << "Correct data file visited:" << dataFile.fullFileName();
    databaseList.push_back(std::make_shared<DatabaseFile> (dataFile));
}

void DistanceCalculator::loadWarpgates(){
    qDebug() << Q_FUNC_INFO << "Attempting to load all level database files";
    std::shared_ptr<TFFile> testLoaded;
    //need to prompt the user for the game directory, then use that
    QString gamePath = QFileDialog::getExistingDirectory(parent, parent->tr(QString("Select TF2004 game folder.").toStdString().c_str()), QDir::currentPath());
    //then load TMD from TFA2, then load each file from TFA.
    testLoaded = parent->matchFile("CREATURE.TMD");
    if(testLoaded == nullptr){
        QString definitionPath = gamePath + "/TFA2/CREATURE.TMD";
        bool isFileInDirectory = QFileInfo::exists(definitionPath);
        qDebug() << Q_FUNC_INFO << "file directory is" << definitionPath << "and file exists?" << isFileInDirectory;
        if(isFileInDirectory){
            parent->openFile("TMD", definitionPath);
        }
        testLoaded = parent->matchFile("CREATURE.TMD");

        while(testLoaded == nullptr){
            parent->messageError("Please load a file CREATURE.TMD");
            parent->openFile("TMD");
            testLoaded = parent->matchFile("CREATURE.TMD");
        }
    }

    QString levelPath = gamePath + "/TFA/LEVELS/EPISODES";
    QStringList levelList = QDir(levelPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    int levelCount = levelList.count();
    for(int level = 0; level < levelCount; level++){
        testLoaded = parent->matchFile(levelList[level] + "-CREATURE.BDB");
        if(testLoaded == nullptr){
            QString creaturePath = levelPath + "/" + levelList[level] + "/CREATURE.BDB";
            bool isFileInDirectory = QFileInfo::exists(creaturePath);
            qDebug() << Q_FUNC_INFO << "file directory is" << creaturePath << "and file exists?" << isFileInDirectory;
            if(isFileInDirectory){
                parent->openFile("BDB", creaturePath);
            }
            testLoaded = parent->matchFile(levelList[level] + "-CREATURE.BDB");
        }
        while(testLoaded == nullptr){
            parent->messageError("Please load a file " + levelList[level]+".BDB");
            parent->openFile("BDB");
            testLoaded = parent->matchFile(levelList[level] + ".BDB");
        }
        testLoaded->acceptVisitor(*this);

    }

}

void DistanceCalculator::calculateWarpgateDistance(){
    Warpgate closestGate = Warpgate();
    float totalDifference = 0;
    float lowestDistance = 99999;
    float xDistance = inputXValue->text().toFloat();
    float yDistance = inputYValue->text().toFloat();
    float zDistance = inputZValue->text().toFloat();

    qDebug() << Q_FUNC_INFO << "Finding closes warpgate to point: x" << xDistance << "y" << yDistance << "z" << zDistance;
    for (int i = 0; i<warpgateList.size(); i++) {
        totalDifference = std::pow(std::pow(xDistance - warpgateList[i].x_position, 2)
                + std::pow(yDistance - warpgateList[i].y_position, 2)
                + std::pow(zDistance - warpgateList[i].z_position, 2)
                    ,0.5);
        if (totalDifference < lowestDistance) {
            lowestDistance = totalDifference;
            closestGate = warpgateList[i];
        }
        qDebug() << Q_FUNC_INFO << "Distance to warpgate" << warpgateList[i].name << ":" << totalDifference;
    }

    qDebug() << Q_FUNC_INFO << "Closest warpgate: " << closestGate.name;

    if(closestGate.name == ""){
        closestGate.name = QString::number(closestGate.x_position) + " " + QString::number(closestGate.y_position) + " " + QString::number(closestGate.z_position);
    }

    if(parent->ClosestWarpgate == nullptr){
        parent->ClosestWarpgate = new QLabel(closestGate.name, parent->centralContainer);
        parent->ClosestWarpgate->setGeometry(QRect(QPoint(650,320), QSize(150,30)));
        parent->ClosestWarpgate->setStyleSheet("QLabel { background-color: rgb(105,140,187) }");
        parent->ClosestWarpgate->show();
    } else {
        parent->ClosestWarpgate->setText(closestGate.name);
    }
}

std::vector<Warpgate*> Warpgate::createAmazonWarpgates(){
    std::vector<Warpgate*> warpgates;

    Warpgate* warpgate1 = new Warpgate;
    warpgate1->name = "Amazon Basin";
    warpgate1->x_position = 962.6;
    warpgate1->y_position = -1749.8;
    warpgate1->z_position = -3.5;
    warpgates.push_back(warpgate1);

    Warpgate* warpgate2 = new Warpgate;
    warpgate2->name = "Stone Bridge";
    warpgate2->x_position = 149.6;
    warpgate2->y_position = -1624.4;
    warpgate2->z_position = -5.4;
    warpgates.push_back(warpgate2);

    Warpgate* warpgate3 = new Warpgate;
    warpgate3->name = "Ruined Temple";
    warpgate3->x_position = -529.1;
    warpgate3->y_position = -1593.4;
    warpgate3->z_position = -65.8;
    warpgates.push_back(warpgate3);

    Warpgate* warpgate4 = new Warpgate;
    warpgate4->name = "Waterfall";
    warpgate4->x_position = -95.6;
    warpgate4->y_position = -751.8;
    warpgate4->z_position = 104.9;
    warpgates.push_back(warpgate4);

    Warpgate* warpgate5 = new Warpgate;
    warpgate5->name = "Deep Ravine";
    warpgate5->x_position = 748.2;
    warpgate5->y_position = -112.7;
    warpgate5->z_position = 133.9;
    warpgates.push_back(warpgate5);

    Warpgate* warpgate6 = new Warpgate;
    warpgate6->name = "Mountain Ruins";
    warpgate6->x_position = 1034.2;
    warpgate6->y_position = 287.6;
    warpgate6->z_position = 433.1;
    warpgates.push_back(warpgate6);

    return warpgates;
}
