#ifndef LEVELGEO_H
#define LEVELGEO_H

#include <QVector3D>
#include "Headers/Main/BinChanger.h"

class ProgWindow;
class MeshVBIN;
class FileData;
class Color;

class GeometrySet{
public:
    SectionHeader headerData;
    std::vector<QVector3D> geoSetVerticies;
    std::vector<Color> geoSetColors;
    std::vector<QVector2D> geoSetTexCoords;
    int version;
    int unknownValue1;
    int vertexAttributeGroup;
    int vertexFormat;
    int unknownValue2;
    int unknownValue3;
    int unknownValue4;
    int unknownValue5;
    int unknownValue6;
    int unknownValue7;
    int unknownValue8;
    int unknownValue9;
    int unknownValue10;
    int unknownValue11;
    QVector3D position1;
    QVector3D position2;
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
