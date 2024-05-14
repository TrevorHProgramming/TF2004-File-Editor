#ifndef DISTANCECALCULATOR_H
#define DISTANCECALCULATOR_H

#include <QLineEdit>
#include <QString>

#include "Headers/Databases/Database.h"

class ProgWindow;

class WarpgateFile : public DatabaseFile{
    std::vector<Warpgate> warpgateList;

    void isolateWarpgates();
};

class DistanceCalculator{
public:
    DistanceCalculator(ProgWindow *parentPass);
    std::vector<Warpgate> warpgateList;
    ProgWindow *parent;

    QLineEdit *inputXValue;
    QLineEdit *inputYValue;
    QLineEdit *inputZValue;

    int currentLevel;

    void calculateWarpgateDistance();
    void userSelectLevel(int selectedLevel);

};

#endif // DISTANCECALCULATOR_H
