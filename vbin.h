#ifndef VBIN_H
#define VBIN_H

#include <QVector>
#include <QQuaternion>
#include <QByteArrayMatcher>
#include <QTransform>


class ProgWindow;

class Mesh;

class Triangles{
  public:
    QVector3D vertex1;
    QVector3D vertex2;
    QVector3D vertex3;
};

class TriangleStrip{
public:
    std::vector<int> stripIndecies;
};

class BoundingVolume{
public:
    long fileLocation;
    bool hasVolume;
    int type;
    QVector3D location;
    float radius;

    void populateData(ProgWindow &ProgWindow);
};

class NormalArray{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  QVector3D positionList[];
};

class ColorArray{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  QVector3D positionList[];
};

class TextureCoords{};

class SceneNode{
public:
    long fileLocation;
    BoundingVolume boundVol;
    //std::vector<SceneNode> sceneNode;
    std::vector<Mesh> meshList;
    //PositionArray posArray;
    int modifications;
    QVector3D offset;
    QQuaternion rotation;
    float scale;
    QString name;

    void getModifications(ProgWindow& ProgWindow);
    void modifyPosArrays();
    void clear();

};

class VBIN{
public:
    QString filePath;
    std::vector<SceneNode> nodeList;
    std::vector<Mesh> meshList;

    std::vector<int> getIndexArrays(int posCount, int chosenLOD, int location, ProgWindow &ProgWindow);
    void makeTriangles(std::vector<int> indArrays, int whichArray, int location, QTextStream *stream, ProgWindow &ProgWindow);
    void readData(ProgWindow &ProgWindow);
    void writeData(ProgWindow &ProgWindow);
private:
    void modifyPosArrays();
    void outputPositionList(int location, ProgWindow &ProgWindow);
    void getSceneNodeTree(ProgWindow& ProgWindow, long searchStart);
    void analyzeTriangles(std::vector<int> indArrays, int whichArray, ProgWindow &ProgWindow);
    QVector3D needOffset(int offsetLocation, ProgWindow &ProgWindow);
    QQuaternion getRotation(int offsetLocation, ProgWindow &ProgWindow);
    float getScale(int offsetLocation, ProgWindow &ProgWindow);
};

#endif // VBIN_H
