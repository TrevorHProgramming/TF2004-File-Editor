#ifndef VBIN_H
#define VBIN_H

#include <QVector>
#include <QQuaternion>
#include <QByteArrayMatcher>
#include <QTransform>


class ProgWindow;

class Mesh;
class VBIN;

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
    VBIN *file;

    void populateData();
};

class NormalArray{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  VBIN *file;
  QVector3D positionList[];
};

class ColorArray{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  VBIN *file;
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
    VBIN *file;

    void getModifications();
    void modifyPosArrays();
    void clear();

};

class VBIN{
public:
    QString filePath;
    std::vector<SceneNode> nodeList;
    std::vector<Mesh> meshList;
    ProgWindow *parent;
    int highestLOD;

    std::vector<int> getIndexArrays(int posCount, int chosenLOD, int location);
    void makeTriangles(std::vector<int> indArrays, int whichArray, int location, QTextStream *stream);
    void readData();
    void writeData();
private:
    void modifyPosArrays();
    void outputPositionList(int location);
    void getSceneNodeTree(long searchStart);
    void analyzeTriangles(std::vector<int> indArrays, int whichArray);
    QVector3D needOffset(int offsetLocation);
    QQuaternion getRotation(int offsetLocation);
    float getScale(int offsetLocation);
};

#endif // VBIN_H
