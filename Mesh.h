#ifndef MESH_H
#define MESH_H

#include "vbin.h"

class LODInfo{
public:
    long fileLocation;
    int levels;
    std::vector<std::vector<int>> targetIndecies;

    void populateLevels(ProgWindow &ProgWindow);
    void clear();
};

class IndexArray{
public:
    int arrayID;
    int arrayLength;
    long fileLocation;
    std::vector<TriangleStrip> triangleStrips;
    std::vector<int> indexList;

    void populateTriangleStrips(ProgWindow &ProgWindow);
};

class PositionArray{
public:
  int arrayID;
  int vertexCount;
  long fileLocation;
  QString meshName;
  std::vector<QVector3D> positionList; //standin for positioniterator
  std::vector<IndexArray> indexArrays;

  void Transform();
  void getIndexArrays(ProgWindow& ProgWindow);
  void clear();
};

class VertexSet{
public:
    void createArrays();

    PositionArray* posArray;
    NormalArray* normArray;
    ColorArray* colArray;
    TextureCoords* textCoords;

    void getArrayLocations(ProgWindow& ProgWindow);
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

    void clear();
    void getModifications(ProgWindow& ProgWindow);
};





#endif // MESH_H
