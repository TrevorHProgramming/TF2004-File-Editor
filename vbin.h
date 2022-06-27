#ifndef VBIN_H
#define VBIN_H

#include <QVector>
#include <QQuaternion>
#include <QByteArrayMatcher>
#include <QTransform>


class ProgWindow;

class VBIN;
class Mesh;
class SceneNode;

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
    const void operator=(BoundingVolume input);
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

class Modifications{
public:
    int modByte;
    QVector3D offset;
    QQuaternion rotation;
    float scale;

    void clear();
};

class FileSection{
public:
    VBIN *file;
    long fileLocation;
    QString name;
    long trueLength;
    BoundingVolume boundVol;

    Modifications mods;

    QStringList sectionTypes;
    std::vector<std::variant<Mesh, SceneNode>> sectionList;

    void readData(long sceneLocation);
    void getSceneNodeTree(long searchStart, long searchEnd, int depth);
    void getModifications();
    void modifyPosArrays(std::vector<Modifications> addedMods);
    void printInfo(int depth); // for debugging
    const void operator=(FileSection input);
    void writeData(QTextStream &file);
    void writeData(QString path);
    // void modifyPosArrays(); //this might go somewhere else, just
    // commenting out for now
    void clear();
};

class SceneNode : public FileSection{
public:
    void readData(long sceneLocation);
    //void getModifications(); //inherited version should be workable
    //void modifyPosArrays();
    void clear();
};

class VBIN{
public:
    const QByteArray meshStr = QByteArray::fromHex("7E4D6573680001");
    const QStringList sectionNames = {"~SceneNode", "~PositionArray", "~LODinfo", "~BoundingVolume", "~VertexSet"};
    QString filePath;
    ProgWindow *parent;
    int highestLOD;
    long currentLocation;
    FileSection base;

    std::vector<int> getIndexArrays(int posCount, int chosenLOD, int location);
    void makeTriangles(std::vector<int> indArrays, int whichArray, int location, QTextStream *stream);
    void readData();
    void writeData();
private:
    //void modifyPosArrays();
    void outputPositionList(int location);
    void getSceneNodeTree(long searchStart, long searchEnd, int depth);
    void analyzeTriangles(std::vector<int> indArrays, int whichArray);
    QVector3D needOffset(int offsetLocation);
    QQuaternion getRotation(int offsetLocation);
    float getScale(int offsetLocation);
};

#endif // VBIN_H
