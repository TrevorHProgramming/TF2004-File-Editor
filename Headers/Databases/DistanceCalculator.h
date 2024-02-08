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
    std::vector<std::shared_ptr<DatabaseFile>> databaseList;
    std::vector<Warpgate> warpgateList;
    ProgWindow *parent;

    QLineEdit *inputXValue;
    QLineEdit *inputYValue;
    QLineEdit *inputZValue;

    int currentLevel;

    void calculateWarpgateDistance();
    void loadWarpgates();
    void userSelectLevel(int selectedLevel);


    void visit(TFFile dataFile);
    void visit(DatabaseFile dataFile);
};

#endif // DISTANCECALCULATOR_H
