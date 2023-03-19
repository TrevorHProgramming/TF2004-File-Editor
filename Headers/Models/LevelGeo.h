#ifndef LEVELGEO_H
#define LEVELGEO_H

#include <QVector3D>
#include "Headers/Main/BinChanger.h"

class ProgWindow;
class MeshVBIN;
class FileData;

class GeometrySet{
public:
    std::vector<QVector3D> geoSetVerticies;
    std::vector<QVector2D> geoSetTexCoords;
    int version;
    int vertexAttributeGroup;
    int vertexFormat;
    MeshVBIN *file;
    FileData *fileData;

    void getVerticies();
};

class MeshVBIN : public TFFile {
  public:
    std::vector<GeometrySet> geoSets;

    void readData();
    void openMeshVBINFile();
};

#endif // LEVELGEO_H
