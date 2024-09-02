#include "Headers/Main/mainwindow.h"

void DistanceCalculator::userSelectLevel(int selectedLevel){
    //called when user makes a selection on the level list dropdown
    currentLevel = selectedLevel;
    std::vector<dictItem> warpgateListRaw = parent->databaseList[currentLevel]->sendInstances("Warpgate");
    //warpgateList = parent->databaseList[currentLevel]->sendWarpgates();
    //then currentlevel is used below to calculate the closest warpgate
}

DistanceCalculator::DistanceCalculator(ProgWindow *parentPass){
    parent = parentPass;
    qDebug() << Q_FUNC_INFO << "parent value properly passed, window width is" << parent->hSize;
    if(parent->loadDatabases() != 0){
        parent->log("Could not load databases. Warpgate calculator was not loaded.");
        return;
    }
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
    ListLevels -> setGeometry(QRect(QPoint(250,150), QSize(200,30)));

    for(int i=0; i<parent->databaseList.size(); ++i){
        ListLevels->insertItem(i, parent->databaseList[i]->fileName);
    }

    QComboBox::connect(ListLevels, &QComboBox::currentIndexChanged, parent, [ListLevels, this] {userSelectLevel(ListLevels->currentIndex());});
    ListLevels->show();
    parent->currentModeWidgets.push_back(ListLevels);
    ListLevels->setCurrentIndex(0);
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
    closestGate.name += " x" + QString::number(closestGate.x_position) + " y" + QString::number(closestGate.y_position) + " z" + QString::number(closestGate.z_position);

    qDebug() << Q_FUNC_INFO << "Closest warpgate: " << closestGate.name;

    if(parent->ClosestWarpgate == nullptr){
        parent->ClosestWarpgate = new QLabel(closestGate.name, parent->centralContainer);
        parent->ClosestWarpgate->setGeometry(QRect(QPoint(650,320), QSize(450,30)));
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
