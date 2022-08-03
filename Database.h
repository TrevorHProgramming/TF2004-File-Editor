#ifndef DATABASE_H
#define DATABASE_H

#include <QQuaternion>
#include <QString>
#include <map>


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
class TMDFile;

class dictItem{
public:

    int index;
    QString type;
    bool active;
    bool isDefault;
    QString name;
    TMDFile *file;

    QString value;
    QStringList valueList;

    void clear();
    const void operator=(dictItem input);
};


class dictClass{
public:
    int sectionIndex; //either 1 or 2 depending on file type
    QString name;
    QString inheritedClass;
    std::vector<dictItem> itemList;
};

class dictInstance{
public:
    int sectionIndex; //should always = 3 for instances
    QString name;
    QString inheritedClass;
    int instanceIndex;
    std::vector<dictItem> itemList;
};

class TMDFile{
  public:
    const QStringList sectionList{"IncludedFiles","Dictionary","FileDictionary","Instances"};
    const QStringList singleTypes{"Enum","Float","Bool","String","Integer", "Link"};
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
    dictItem addItem(dictItem itemDetails);
    void writeData();
    QString outputValue(dictItem itemDetails);
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

class BDBFile : public TDBFile, public BMDFile {
    //inheriting tdb for access to instanceIndexIn and editItem
    dictItem addItem(dictItem itemDetails, QByteArray tempRead);
    void writeData();
    QString outputValue(dictItem itemDetails);
};

#endif // DATABASE_H
