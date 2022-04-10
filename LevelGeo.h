#ifndef LEVELGEO_H
#define LEVELGEO_H

#include <QVector3D>

class ProgWindow;

class GeometrySet{
public:
    std::vector<float> geoSetVerticies;
    QString filePath;

    void getVerticies(ProgWindow &ProgWindow);
    void openMeshVBINFile(ProgWindow &ProgWindow);
};

#endif // LEVELGEO_H
