#ifndef MESH_H
#define MESH_H

#include "vbin.h"

class LODInfo{
public:
    long fileLocation;
    int levels;
    std::vector<std::vector<int>> targetIndecies;
    std::vector<float> levelDistance;
    FileData *fileData;

    void populateLevels();
    void clear();
};

class IndexArray{
public:
    int arrayID;
    int arrayLength;
    long fileLocation;
    int triangleCount;
    std::vector<TriangleStrip> triangleStrips;
    std::vector<int> indexList;
    FileData *fileData;

    void populateTriangleStrips();
};

class MeshFaceSet{
public:
    int version;
    int indexCount; //pretty sure this is index count or triangle strip count
    int primitiveType;
    IndexArray indexArray;
};

class PositionArray{
public:
  int arrayID;
  int vertexCount;
  long fileLocation;
  QString meshName;
  std::vector<QVector3D> positionList;
  FileData *fileData;

  //void Transform();
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
    FileData *fileData;

    void getArrayLocations();
};

class Material{
public:
    int sectionLength;
    int version;
    int nameLength;
    QString name;
    QVector4D diffuse;
    QVector4D specular;
    QVector4D ambient;
    float specularPower;

    void debug();
};

class RenderStateGroup{
public:
    int version;
    int pixelBlend;
    int textureBlend;
    int textureClamp;
    int textureFlip;
    int textureFilter;
    int cullMode;
};

class SurfaceProperties{
public:
    QString textureName;
    QString texture2Name;
    int version;
    Material material;
    RenderStateGroup renderStateGroup1;
    RenderStateGroup renderStateGroup2;
};

class Element{
public:
    MeshFaceSet meshFaceSet;
    SurfaceProperties surfaceProperties;
    int version;
    int attributes;
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
    ElementArray elementArray;
    int elementCount;

    void clear();
    void applyKeyframe(QVector3D keyOffset);
    void applyKeyframe(QQuaternion keyRotation);
    int readMesh();
    //void modifyPosArrays(Modifications mods);
    void getModifications();
    void writeDataSTL(QTextStream &fileOut);
    std::vector<int> getChosenElements();
    void modify(std::vector<Modifications> addedMods);
    void writeNodesDAE(QTextStream &fileOut);
    void writeDataDAE(QTextStream &fileOut);
    void writeEffectsDAE(QTextStream &fileOut);
    void writeMaterialsDAE(QTextStream &fileOut);
    void writeImagesDAE(QTextStream &fileOut);

    //template<typename Item>
    //void searchSomethingListDAE(QTextStream &fileOut, Item write);

    const void operator=(Mesh input);
};



#endif // MESH_H
