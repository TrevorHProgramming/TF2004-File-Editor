#include "mainwindow.h"

void DistanceCalculator::calculateWarpgateDistance(){
    Warpgate *closestGate = new Warpgate;
    std::vector<Warpgate*> warpgates = closestGate->createAmazonWarpgates();
    float totalDifference = 0;
    float lowestDistance = 99999;
    float xDistance = parent->CalculateXValue->text().toFloat();
    float yDistance = parent->CalculateYValue->text().toFloat();
    float zDistance = parent->CalculateZValue->text().toFloat();

    qDebug() << Q_FUNC_INFO << "Finding closes warpgate to point: x" << xDistance << "y" << yDistance << "z" << zDistance;
    for (int i = 0; i<warpgates.size(); i++) {
        totalDifference = std::pow(std::pow(xDistance - warpgates[i]->x_value, 2)
                + std::pow(yDistance - warpgates[i]->y_value, 2)
                + std::pow(zDistance - warpgates[i]->z_value, 2)
                    ,0.5);
        if (totalDifference < lowestDistance) {
            lowestDistance = totalDifference;
            closestGate = warpgates[i];
        }
        qDebug() << Q_FUNC_INFO << "Distance to warpgate" << warpgates[i]->name << ":" << totalDifference;
    }

    qDebug() << Q_FUNC_INFO << "Closest warpgate: " << closestGate->name;


    if(parent->ClosestWarpgate == nullptr){
        parent->ClosestWarpgate = new QLabel(closestGate->name, parent);
        parent->ClosestWarpgate->setGeometry(QRect(QPoint(650,320), QSize(150,30)));
        parent->ClosestWarpgate->setStyleSheet("QLabel { background-color: rgb(105,140,187) }");
        parent->ClosestWarpgate->show();
    } else {
        parent->ClosestWarpgate->setText(closestGate->name);
    }
}

std::vector<Warpgate*> Warpgate::createAmazonWarpgates(){
    std::vector<Warpgate*> warpgates;

    Warpgate* warpgate1 = new Warpgate;
    warpgate1->name = "Amazon Basin";
    warpgate1->x_value = 962.6;
    warpgate1->y_value = -1749.8;
    warpgate1->z_value = -3.5;
    warpgates.push_back(warpgate1);

    Warpgate* warpgate2 = new Warpgate;
    warpgate2->name = "Stone Bridge";
    warpgate2->x_value = 149.6;
    warpgate2->y_value = -1624.4;
    warpgate2->z_value = -5.4;
    warpgates.push_back(warpgate2);

    Warpgate* warpgate3 = new Warpgate;
    warpgate3->name = "Ruined Temple";
    warpgate3->x_value = -529.1;
    warpgate3->y_value = -1593.4;
    warpgate3->z_value = -65.8;
    warpgates.push_back(warpgate3);

    Warpgate* warpgate4 = new Warpgate;
    warpgate4->name = "Waterfall";
    warpgate4->x_value = -95.6;
    warpgate4->y_value = -751.8;
    warpgate4->z_value = 104.9;
    warpgates.push_back(warpgate4);

    Warpgate* warpgate5 = new Warpgate;
    warpgate5->name = "Deep Ravine";
    warpgate5->x_value = 748.2;
    warpgate5->y_value = -112.7;
    warpgate5->z_value = 133.9;
    warpgates.push_back(warpgate5);

    Warpgate* warpgate6 = new Warpgate;
    warpgate6->name = "Mountain Ruins";
    warpgate6->x_value = 1034.2;
    warpgate6->y_value = 287.6;
    warpgate6->z_value = 433.1;
    warpgates.push_back(warpgate6);

    return warpgates;
}
