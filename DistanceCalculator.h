#ifndef DISTANCECALCULATOR_H
#define DISTANCECALCULATOR_H

#include <QLineEdit>
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
    DistanceCalculator(ProgWindow *parentPass);
    std::vector<Warpgate*> warpgateList;
    ProgWindow *parent;

    QLineEdit *inputXValue;
    QLineEdit *inputYValue;
    QLineEdit *inputZValue;

    void calculateWarpgateDistance();
};

#endif // DISTANCECALCULATOR_H
