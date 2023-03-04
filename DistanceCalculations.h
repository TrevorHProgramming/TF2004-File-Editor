#ifndef DISTANCECALCULATIONS_H
#define DISTANCECALCULATIONS_H

#include <QString>

class Warpgate{
public:
    float x_value;
    float y_value;
    float z_value;
    QString name;

    static std::vector<Warpgate> createAmazonWarpgates();
};

#endif // DISTANCECALCULATIONS_H
