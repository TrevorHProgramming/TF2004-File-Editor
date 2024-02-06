#ifndef DISTANCECALCULATOR_H
#define DISTANCECALCULATOR_H

#include <QLineEdit>
#include <QString>

#include "Headers/Databases/Database.h"

class ProgWindow;

class Warpgate{
public:
    double x_value;
    double y_value;
    double z_value;
    QString name;

    static std::vector<Warpgate*> createAmazonWarpgates();
};

class WarpgateFile : public DatabaseFile{
    std::vector<Warpgate> warpgateList;

    void isolateWarpgates();
};

class DistanceCalculator{
public:
    DistanceCalculator(ProgWindow *parentPass);
    std::vector<Warpgate*> warpgateList;
    ProgWindow *parent;

    QLineEdit *inputXValue;
    QLineEdit *inputYValue;
    QLineEdit *inputZValue;

    void calculateWarpgateDistance();
    void loadWarpgates();
};

#endif // DISTANCECALCULATOR_H
