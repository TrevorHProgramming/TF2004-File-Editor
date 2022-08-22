#ifndef DATABASE_H
#define DATABASE_H

#include <QQuaternion>
#include <QString>
#include <map>
#include <QFile>


/*Someone should do something with this at some point - I feel like it could cut down on all the switch statements later on.

template <class dictTest>
class dictItem2{
public:
    dictTest value;

    dictTest parseFloat();
};

std::map<QString, std::function<std::variant<dictItem2<float>, dictItem2<int>>>> parsers;
*/

class ProgWindow;
//class TMDFile;
class DefinitionFile;
class FileData;

class dictItem{
public:

    int index;
    int length;
    QString type;
    bool active;
    bool isDefault;
    bool inherited;
    QString name;
    QString comment;
    DefinitionFile *file;

    QString value;
    QStringList valueList;

    void clear();
    const void operator=(dictItem input);
};


class dictClass{
public:
    QString name;
    long length;
    QString inheritedClass;
    std::vector<dictItem> itemList;
};

class dictInstance{
public:
    QString name;
    long length;
    QString inheritedClass;
    int instanceIndex;
    std::vector<dictItem> itemList;
};

class DefinitionFile{
public:
    bool binary; //0 for text, 1 for binary. used for getname, output value
    bool database; //0 for definition file, 1 for database file
    const QStringList sectionList{"IncludedFiles","Dictionary","FileDictionary","Instances"};
    const QStringList singleTypes{"Enum","Float","Bool","String","Integer", "Link", "Flag"};
    const QStringList multiTypes{"Enum","Point","Quaternion","IntegerArray","StringArray","Color", "FloatArray", "LinkArray","VectorArray"};
    const QStringList arrayTypes{"IntegerArray","StringArray", "FloatArray", "LinkArray","VectorArray"}; //need for bmd and bdb
    const QStringList stringTypes{"String","StringArray"};
    static QMap<int, QStringList> bdbTypeLength;
    ProgWindow *parent;
    QString filePath;
    QString fileName;
    QString includedFile;
    int versionNumber;
    int inheritedFileIndex;
    std::vector<dictClass> classList;
    std::vector<dictInstance> instanceList;
    QStringList majorSections;

    int readData();
    int readText();
    int readBinary();
    void writeText();
    void writeBinary();
    int indexIn(QString searchName);
    QString getName();
    void getFileLengths(); //for getting a binary file's file, section, and item lengths
    int itemLength(dictItem itemDetails);
    int instanceLength(dictItem itemDetails);
    int dictItemIndex(int dictIndex, QString searchName);
    virtual dictItem addItem(dictItem itemDetails, QString tempRead); //text
    virtual dictItem addItem(dictItem itemDetails, FileData *tempRead); //binary
    void newInstance();
    void newItem();
    void removeItem(QString className, int itemIndex);
    void removeClass(int classIndex);
    QString outputValue(dictItem itemDetails);
    void binaryOutput(QFile& file, dictItem itemDetails);
    QString displayValue(dictItem itemDetails);
    int readIncludedFiles(QString fullRead); //note that includedfiles only needs fullread, not partsplit
    void readDictionary(QStringList partSplit, int sectionIndex); //text version
    void readFileDictionary(QStringList partSplit, int sectionIndex); //text
    void readInstances(QStringList partSplit, int sectionIndex, QString instanceName); //text
    void readDictionary(QByteArray splitLine, int sectionIndex, QString instanceName); //binary version
    void readFileDictionary(QByteArray splitLine, int sectionIndex, QString instanceName); //binary
    void readInstances(QByteArray splitLine, int sectionIndex, QString instanceName); //binary
    void createDBTree();
    //void editItem(int dictIndex, int itemIndex, QString valueType, QString newValue);
    QStringList editItem(QString className, int itemIndex);
    void clear();
};

class DatabaseFile : public DefinitionFile{
public:

    dictItem addItem(dictItem itemDetails, QString tempRead); //text
    //dictItem addItem(dictItem itemDetails, FileData *tempRead); //binary
    void writeData();
    void writeText();
    void writeBinary();
    QString outputValue(dictItem itemDetails);
    int instanceIndexIn(int searchIndex);
    void createDBTree();
    QStringList editItem(int instanceIndex, int itemIndex);
    void removeItem(int instanceIndex, int itemIndex);
    void removeInstance(int instanceIndex);
};

/*class TMDFile{
  public:
    const QStringList sectionList{"IncludedFiles","Dictionary","FileDictionary","Instances"};
    const QStringList singleTypes{"Enum","Float","Bool","String","Integer", "Link", "Flag"};
    const QStringList multiTypes{"Enum","Point","Quaternion","IntegerArray","StringArray","Color", "FloatArray", "LinkArray","VectorArray"};
    const QStringList arrayTypes{"IntegerArray","StringArray", "FloatArray", "LinkArray","VectorArray"}; //need for bmd and bdb
    const QStringList stringTypes{"String","StringArray","LinkArray"};
    static QMap<int, QStringList> bdbTypeLength;
    ProgWindow *parent;
    QString filePath;
    QString fileName;
    QString fileType;
    QString includedFile;
    QString includedFileIndex;
    std::vector<dictClass> classList;
    std::vector<dictInstance> instanceList;
    int inheritedFileIndex;

    int readData();
    void writeData();
    int indexIn(QString searchName);
    QString getNextLine();
    QString getName();
    int dictItemIndex(int dictIndex, QString searchName);
    dictItem addItem(dictItem itemDetails, QString tempRead);
    QString outputValue(dictItem itemDetails);
    QString displayValue(dictItem itemDetails);
    int readIncludedFiles(QString fullRead); //note that includedfiles only needs fullread, not partsplit
    void readDictionary(QStringList partSplit, int sectionIndex);
    void readFileDictionary(QStringList partSplit, int sectionIndex);
    void readInstances(QStringList partSplit, int sectionIndex, QString instanceName);
    void createDBTree();
    //void editItem(int dictIndex, int itemIndex, QString valueType, QString newValue);
    QStringList editItem(QString className, int itemIndex);
    void clear();
};

class BMDFile : public TMDFile{
public:
    int readData();
    QString getName();
    dictItem addItem(dictItem itemDetails, FileData *tempRead);
    void writeData();
    QString outputValue(dictItem itemDetails);
    void readDictionary(QByteArray splitLine, int sectionIndex, QString instanceName);
    void readFileDictionary(QByteArray splitLine, int sectionIndex, QString instanceName);
    void readInstances(QByteArray splitLine, int sectionIndex, QString instanceName);
};

class TDBFile : public TMDFile{
public:
    //int readData();
    dictItem addItem(dictItem itemDetails, QString tempRead);
    void writeData();
    QString outputValue(dictItem itemDetails);
    int instanceIndexIn(int searchIndex);
    QStringList editItem(int instanceIndex, int itemIndex);

};

class BDBFile : public BMDFile {
    //inheriting tdb for access to instanceIndexIn and editItem
    dictItem addItem(dictItem itemDetails, QByteArray tempRead);
    void writeData();
    QString outputValue(dictItem itemDetails);
    int instanceIndexIn(int searchIndex);
    QStringList editItem(int instanceIndex, int itemIndex);
};*/

#endif // DATABASE_H
