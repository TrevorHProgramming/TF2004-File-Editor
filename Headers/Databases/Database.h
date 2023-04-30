#ifndef DATABASE_H
#define DATABASE_H

#include <QQuaternion>
#include <QString>
#include <map>
#include <QFile>
#include <QTreeView>
#include <QStandardItemModel>

#include "Headers/Main/BinChanger.h"


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

class DefinitionFile : public TFFile {
public:
    std::shared_ptr<DefinitionFile> inheritedFile;
    QTreeView *dataTree;
    QStandardItemModel *dataModel;
    bool database; //0 for definition file, 1 for database file
    const QStringList sectionList{"IncludedFiles","Dictionary","FileDictionary","Instances"};
    const QStringList singleTypes{"Enum","Float","Bool","String","Integer", "Link", "Flag"};
    const QStringList multiTypes{"Enum","Point","Quaternion","IntegerArray","StringArray","Color", "FloatArray", "LinkArray","VectorArray"};
    const QStringList arrayTypes{"IntegerArray","StringArray", "FloatArray", "LinkArray","VectorArray"}; //need for bmd and bdb
    const QStringList stringTypes{"String","StringArray"};
    static QMap<int, QStringList> bdbTypeLength;
    //QString includedFile;
    int versionNumber;
    std::vector<dictClass> classList;
    std::vector<dictInstance> instanceList;
    QStringList majorSections;

    void save(QString toType);
    void load(QString fromType);

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
    void updateCenter();
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
    void sortDBTree(int column);
    //void editItem(int dictIndex, int itemIndex, QString valueType, QString newValue);
    void editTreeItem(QModelIndex item, int itemIndex);
    QStringList editItem(QString className, int itemIndex);
    void removeTreeClass(QModelIndex item);
    void removeTreeItem(QModelIndex item, int itemIndex);
    /*These functions should exist in definitionfile, not in ProgWindow.*/
//    void editDatabaseItem(QModelIndex item, int itemIndex);
//    void removeDatabaseItem(QModelIndex item, int itemIndex);
//    void removeDatabaseClass(QModelIndex item);
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
    void createItem();
    QStringList editItem(int instanceIndex, int itemIndex);
    void removeTreeInstance(QModelIndex item);
    void removeTreeItem(QModelIndex item, int itemIndex);
    void removeItem(int instanceIndex, int itemIndex);
    void removeInstance(int instanceIndex);
    void updateCenter();
    void editTreeItem(QModelIndex item, int itemIndex);
};

#endif // DATABASE_H
