#ifndef VBIN_H
#define VBIN_H

#include <QVector>
#include <QQuaternion>
#include <QByteArrayMatcher>
#include <QTransform>

#include "Antioch2.h"
#include "BinChanger.h"

class ProgWindow;

class VBIN;
class Mesh;
class SceneNode;

class Triangle{
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
    FileData *fileData;

    void populateData();
    const void operator=(BoundingVolume input);
};

//something like this can be used to improve Normal, Color, and Position arrays
template <typename T> class binArray {
public:
    int arrayID;
    long arrayLength;
    Mesh *parentMesh;
    QVector3D valueList[];
};

class NormalArray{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  FileData *fileData;
  std::vector<QVector3D> positionList;
};

class ColorArray{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  FileData *fileData;
  std::vector<QVector4D> positionList;
};

class TextureCoords{
public:
  int arrayID;
  long arrayLength;
  QString meshName;
  FileData *fileData;
  std::vector<QVector2D> positionList;
};

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
    FileData *fileData;
    FileSection *parent;
    long fileLocation;
    QString name;
    long sectionLength;
    long sectionEnd;
    BoundingVolume boundVol;

    Modifications mods;

    QStringList sectionTypes;
    std::vector<FileSection*> sectionList;
    std::vector<Mesh*> meshList;

    void readData(long sceneLocation);
    void getSceneNodeTree(long searchStart, long searchEnd, int depth);
    void readNode();
    void sendKeyframe(QVector3D keyOffset, QString channelName);
    void sendKeyframe(QQuaternion keyRotation, QString channelName);
    void readModifications();
    //void getModifications();
    void modifyPosArrays(std::vector<Modifications> addedMods);
    void printInfo(int depth); // for debugging
    const void operator=(FileSection input);
    void writeSectionListSTL(QTextStream &file);
    void writeSectionListSTL(QString path);
    void writeSectionListDAE(QTextStream &file);
    void writeSectionListDAE(QString path);
    void writeSceneListDAE(QTextStream &file);
    void writeSceneListDAE(QString path);
    void writeEffectListDAE(QTextStream &file);
    void writeEffectListDAE(QString path);
    void writeImageListDAE(QTextStream &file);
    void writeImageListDAE(QString path);
    void writeMaterialListDAE(QTextStream &file);
    void writeMaterialListDAE(QString path);
    void modify(std::vector<Modifications> addedMods);
    // void modifyPosArrays(); //this might go somewhere else, just
    // commenting out for now
    void clear();
};

class SceneNode : public FileSection{
public:
    void readData(long sceneLocation);
    void modify(std::vector<Modifications> addedMods);
    void writeDataSTL(QTextStream &fileOut);
    void writeDataDAE(QTextStream &fileOut);
    void writeSceneListDAE(QTextStream &FileOut);
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
    FileData *fileData;
    QString fileName;
    QString fileWithoutExtension;
    AnimationSourceSet animationSet;
    int highestLOD;
    long currentLocation;
    FileSection base;

    std::vector<int> getIndexArrays(int posCount, int chosenLOD, int location);
    void makeTriangles(std::vector<int> indArrays, int whichArray, int location, QTextStream *stream);
    int readData();
    void applyKeyframe();
    void outputDataSTL();
    void outputDataDAE();
private:
    //void modifyPosArrays();
    void outputPositionList(int location);
    void readAnimationPrototype();
    void readBoundingVolume();
    int getSceneNodeTree();
    void readModifications();
    void analyzeTriangles(std::vector<int> indArrays, int whichArray);
    QVector3D needOffset(int offsetLocation);
    QQuaternion getRotation(int offsetLocation);
    float getScale(int offsetLocation);
};

#endif // VBIN_H
