#ifndef MESH_H
#define MESH_H

#include "vbin.h"

class LODInfo{
public:
    long fileLocation;
    int levels;
    std::vector<std::vector<int>> targetIndecies;
    std::vector<float> levelDistance;
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

class MeshFaceSet{
public:
    int unknownProperty1;
    int unknownProperty2; //pretty sure this is index count or triangle strip count
    int unknownProperty3;
    IndexArray indexArray;
};

class PositionArray{
public:
  int arrayID;
  int vertexCount;
  long fileLocation;
  QString meshName;
  std::vector<QVector3D> positionList;
  VBIN *file;

  void Transform();
  //void getIndexArrays();
  void clear();
  const void operator=(PositionArray input);
};

class VertexSet{
public:
    void createArrays();

    int vertexCount;
    PositionArray positionArray;
    NormalArray normalArray;
    ColorArray colorArray;
    TextureCoords textureCoords;
    VBIN *file;

    void getArrayLocations();
};

class Material{
public:
    int unknownProperty1;
    int unknownProperty2;
    //some of these might be part of a vector or something
    int nameLength;
    QString name;
    float unknownFloat1;
    float unknownFloat2;
    float unknownFloat3;
    float unknownFloat4;
    float unknownFloat5;
    float unknownFloat6;
    float unknownFloat7;
    float unknownFloat8;
    float unknownFloat9;
    float unknownFloat10;
    float unknownFloat11;
    float unknownFloat12;
    float unknownFloat13;
};

class RenderStateGroup{
public:
    int unknownProperty1;
    int unknownProperty2;
    int unknownProperty3;
    int unknownProperty4;
    int unknownProperty5;
    int unknownProperty6;
    int unknownProperty7;
};

class SurfaceProperties{
public:
    QString name;
    int materialRelated; //recorded values are 4 and 5, with the lowest bit indicating a difference in the material section
    int unknownProperty2;
    Material material;
    RenderStateGroup renderStateGroup1;
    RenderStateGroup renderStateGroup2;
};

class Element{
public:
    MeshFaceSet meshFaceSet;
    SurfaceProperties surfaceProperties;
    int unknownProperty1;
    int unknownProperty2;
    int unknownProperty3;
};

class ElementArray{
public:
    std::vector<Element> elementArray;
    LODInfo lodInfo;
};

class Mesh : public FileSection{
public:
    BoundingVolume boundVol;
    VertexSet vertexSet;
    //PositionArray posArray;
    //NormalArray normalArray;
    //TextureCoords textureArray;
    //ColorArray colorArray;
    ElementArray elementArray;
    int elementCount;

    void clear();
    void readData(long meshLocation);
    int readMesh();
    //void modifyPosArrays(Modifications mods);
    void getModifications();
    void writeData(QTextStream &fileOut);
    void modify(std::vector<Modifications> addedMods);
    const void operator=(Mesh input);
};



#endif // MESH_H
