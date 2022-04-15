#ifndef LEVELGEO_H
#define LEVELGEO_H

#include <QVector3D>

class ProgWindow;

class GeometrySet{
public:
    std::vector<float> geoSetVerticies;
    QString filePath;
    ProgWindow *parent;

    void getVerticies();
    void openMeshVBINFile();
};

#endif // LEVELGEO_H
