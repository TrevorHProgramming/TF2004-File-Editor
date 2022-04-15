#ifndef MESH_H
#define MESH_H

#include "vbin.h"

class LODInfo{
public:
    long fileLocation;
    int levels;
    std::vector<std::vector<int>> targetIndecies;
    VBIN *file;

    void populateLevels();
    void clear();
};

class IndexArray{
public:
    int arrayID;
    int arrayLength;
    long fileLocation;
    std::vector<TriangleStrip> triangleStrips;
    std::vector<int> indexList;
    VBIN *file;

    void populateTriangleStrips();
};

class PositionArray{
public:
  int arrayID;
  int vertexCount;
  long fileLocation;
  QString meshName;
  std::vector<QVector3D> positionList;
  std::vector<IndexArray> indexArrays;
  VBIN *file;

  void Transform();
  void getIndexArrays();
  void clear();
};

class VertexSet{
public:
    void createArrays();

    PositionArray* posArray;
    NormalArray* normArray;
    ColorArray* colArray;
    TextureCoords* textCoords;
    VBIN *file;

    void getArrayLocations();
};

class Mesh{
public:
    long fileLocation;
    BoundingVolume boundVol;
    PositionArray posArray;
    LODInfo lodInfo;
    QString name;
    int modifications;
    QVector3D offset;
    QQuaternion rotation;
    float scale;
    VBIN *file;

    void clear();
    void getModifications();
};





#endif // MESH_H
