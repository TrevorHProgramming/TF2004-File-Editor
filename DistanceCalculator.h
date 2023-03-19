#ifndef DISTANCECALCULATOR_H
#define DISTANCECALCULATOR_H

#include <QString>

class ProgWindow;

class Warpgate{
public:
    double x_value;
    double y_value;
    double z_value;
    QString name;

    static std::vector<Warpgate*> createAmazonWarpgates();
};

class DistanceCalculator{
public:
    std::vector<Warpgate*> warpgateList;
    ProgWindow *parent;

    void calculateWarpgateDistance();
};

#endif // DISTANCECALCULATOR_H
