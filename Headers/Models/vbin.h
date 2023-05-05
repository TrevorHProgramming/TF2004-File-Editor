#ifndef VBIN_H
#define VBIN_H

#include <QVector>
#include <QQuaternion>
#include <QByteArrayMatcher>
#include <QTransform>

#include "Antioch2.h"
#include "Headers/Main/BinChanger.h"

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
    SectionHeader headerData;
    int version;
    bool hasVolume;
    int type;
    QVector3D center;
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
    SectionHeader headerData;
    long sectionEnd;
    BoundingVolume boundVol;

    Modifications mods;

    QStringList sectionTypes;
    std::vector<FileSection*> sectionList;
    std::vector<Mesh*> meshList;

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
    bool meshListContains(QString checkName);
//    void writeSectionListDAE(QTextStream &file);
//    void writeSectionListDAE(QString path);
//    void writeSceneListDAE(QTextStream &file);
//    void writeSceneListDAE(QString path);
//    void writeEffectListDAE(QTextStream &file);
//    void writeEffectListDAE(QString path);
//    void writeImageListDAE(QTextStream &file);
//    void writeImageListDAE(QString path);
//    void writeMaterialListDAE(QTextStream &file);
//    void writeMaterialListDAE(QString path);
    void modify(std::vector<Modifications> addedMods);
    // void modifyPosArrays(); //this might go somewhere else, just
    // commenting out for now

    //template<typename WriteType>
    void searchListsWriteDAE(QTextStream &fileOut, void (Mesh::*)(QTextStream&));

    template<typename WriteType>
    void searchListsWriteDAE(QString path, WriteType *write);

    void clear();
};

class SceneNode : public FileSection{
public:
    void modify(std::vector<Modifications> addedMods);
    void writeDataSTL(QTextStream &fileOut);
    void writeDataDAE(QTextStream &fileOut);
    void writeSceneListDAE(QTextStream &FileOut);
    //void getModifications(); //inherited version should be workable
    //void modifyPosArrays();
    void clear();
};

class AnimationPrototype{
public:
    VBIN *file;
    QString name;
    long sectionLength;
    long sectionEnd;

    int version1;
    int version2;
    int neutralDataCount;

    int version3;

    Modifications mods;

    int detailLevel;
    float visibilityFactorNear;
    float visibilityFactorFar;
    int attributes;

    BoundingVolume boundingVolume1;
    BoundingVolume boundingVolume2;
};

class VBIN : public TFFile {
public:
    const QStringList knownSections = {"~SceneNode", "~Mesh", "~anAnimationPrototype", "~BoundingVolume"};
    const QStringList validOutputs(){
        return QStringList{"STL", "DAE"};
    };
    virtual const QString fileCategory(){
        return "Model";
    };
    AnimationSourceSet animationSet;
    int highestLOD;
    long currentLocation;
    FileSection base;
    int selectedLOD;
    bool singleOutput;

    VBIN();
    std::vector<int> getIndexArrays(int posCount, int chosenLOD, int location);
    void makeTriangles(std::vector<int> indArrays, int whichArray, int location, QTextStream *stream);
    void applyKeyframe();
    void outputDataSTL();
    void outputDataDAE();
    void save(QString toType);
    void load(QString fromType);
    void updateCenter();
    void setOutput(bool single);
    void setLevel(int level);
private:
    //void modifyPosArrays();
    int readDataVBIN();
    void outputPositionList(int location);
    void readAnimationPrototype();
    void readBoundingVolume(SectionHeader* signature);
    int getSceneNodeTree();
    void readModifications();
    void analyzeTriangles(std::vector<int> indArrays, int whichArray);
    QVector3D needOffset(int offsetLocation);
    QQuaternion getRotation(int offsetLocation);
    float getScale(int offsetLocation);
};

#endif // VBIN_H
