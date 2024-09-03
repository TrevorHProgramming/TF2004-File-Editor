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
class taDataEnum;
class FileVisitor;

class taData{
public:
    int index;
    QString type;
    bool active;
    bool isDefault;
    bool inherited;
    QString name;
    QString comment;
    TFFile *file;

    taData(){};
    taData(taData *copyData){
        type = copyData->type;
        active = copyData->active;
        name = copyData->name;
        comment = copyData->comment;
        file = copyData->file;
    };
    int binarySize();
    //looked into doing template functions for these to cut down on how many there are, but virtual templates aren't, I guess
    virtual void read(); //for reading binary data to the data's value types
    virtual void write(QFile& file);
    virtual QString display();
    virtual QString options();
    virtual QString definitionOutput();
    virtual QString databaseOutput();
    virtual std::shared_ptr<taData> clone();
    virtual QString backupDisplay();
    virtual void setValue(QString changedValue);

    virtual taDataEnum* cloneEnum();

    std::variant<QString, QVector3D, QQuaternion, int, float> value();

    virtual QString stringValue();
    virtual QVector3D vectorValue();
    virtual QQuaternion quatValue();
    virtual int intValue();
    virtual float floatValue();

    virtual int size(){
        return 0;
    };

};

template <class valueType>
class taDataArray : public taData{
public:
    std::vector<valueType> values;
    QString options();
    QString definitionOutput();
    QString databaseOutput();

    int size(){
        return 4 + (sizeof(valueType) * values.size());
    };
};

template <class valueType>
class taDataSingle : public taData{
public:
    valueType value;
    //below are used for rare range definitions ex. "Range(0.000,0.000)"
    //should only be present on single-value items
    valueType maxValue;
    valueType minValue;

    QString display();
    valueType giveValue();

    int size(){
        return sizeof(valueType);
    };
};

template <class valueType>
class taDataVectorArray : public taDataArray<valueType>{
public:
    void read();
    std::shared_ptr<taData> clone();
    void write(QFile& file);
    QString backupDisplay();
    //void write();
};

template <class valueType>
class taDataIntArray : public taDataArray<valueType>{
public:
    void read();
    std::shared_ptr<taData> clone();
    void write(QFile& file);
    //void write();
};

template <class valueType>
class taDataStringArray : public taDataArray<valueType>{
public:
    void read();
    std::shared_ptr<taData> clone();
    void write(QFile& file);
    //void write();
    int size(){
        return (sizeof(valueType)+4) * this->values.size();
    };
};

template <class valueType>
class taDataFloatArray : public taDataArray<valueType>{
public:
    void read();
    std::shared_ptr<taData> clone();
    void write(QFile& file);
    //void write();
};

template <class valueType>
class taDataLinkArray : public taDataArray<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    //void write();
};

template <class valueType>
class taDataFloat : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    QString display();
    //void write();

    float floatValue(){
        return this->value;
    };
};

template <class valueType>
class taDataBool : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    //void write();
};

template <class valueType>
class taDataString : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    QString stringValue();
    //void write();
    int size(){
      return 4 + this->value.size();
    };
};

template <class valueType>
class taDataInteger : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    //void write();

    int intValue(){
        return this->value;
    };
};

template <class valueType>
class taDataLink : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    //void write();

    int intValue(){
        return this->value;
    };
};

template <class valueType>
class taDataFlag : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    //void write();
};

template <class valueType>
class taDataColor : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    QString definitionOutput();
    QString databaseOutput();
    //void write();
};

template <class valueType>
class taDataPoint : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    QString backupDisplay();
    void setValue(QString changedValue);
    QString definitionOutput();
    QString databaseOutput();
    QVector3D vectorValue();
    QString stringValue();
    //void write();
};

template <class valueType>
class taDataQuaternion : public taDataSingle<valueType>{
public:
    void read();
    void write(QFile& file);
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    QString backupDisplay();
    QString databaseOutput();
    QQuaternion quatValue();
    //void write();
};

class taDataEnum : public taData{
public:
    int defaultValue;
    QStringList valueOptions;
    void read();
    void write(QFile& file);
    QString options();
    QString display();
    QString definitionOutput();
    QString databaseOutput();
    std::shared_ptr<taData> clone();
    void setValue(QString changedValue);
    taDataEnum* cloneEnum(){
        return this;
    };

    int intValue(){
        return defaultValue;
    };

    int size(){
        return 4;
    };
};


class dictItem{
public:
    QString name;
    long length;
    QString copiedClass;
    int instanceIndex;
    int inheritedDictionaryIndex;
    //std::vector<dictItem> itemList;
    std::vector<std::shared_ptr<taData>> attributes;

    template <typename ValueType>
    ValueType searchAttributes(QString itemName){
        for(int i = 0; i < attributes.size(); i++){
            if(attributes[i]->name == itemName){
                return std::get<ValueType>(attributes[i]->value());
            }
        }
        return ValueType();
    };

    int setAttribute(QString itemName, QString value);
    int setAttributeDefault(QString itemName);

    QString prototype;
    QVector3D position;
    QQuaternion orientation;

    bool operator < (const dictItem& compItem) const
    {
        return (instanceIndex < compItem.instanceIndex);
    }
};

class DictionaryFile : public TFFile {
public:
    virtual const QString fileCategory(){
        return "Database";
    };

    const QStringList knownSections = {"IncludedFiles", "Dictionary", "FileDictionary", "Instances"};

    std::shared_ptr<DefinitionFile> inheritedFile;
    QString inheritedFileName;
    QTreeView *dataTree;
    QStandardItemModel *dataModel;
    int versionNumber;
    std::vector<dictItem> dictionary;

    void save(QString toType);
    void load(QString fromType);

    int readData();
    int readText();
    int readBinary();
    virtual void writeText();
    virtual void writeBinary();
    virtual void writeDAE();
    int readIncludedFiles(QString fullRead);
    QString includedFileRelativePath();

    virtual int readDictionary(); //text version
    virtual int readDictionary(SectionHeader signature); //binary version

    virtual int readFileDictionary(); //text version
    virtual int readInstances(); //text version
    virtual int readFileDictionary(SectionHeader signature); //binary
    virtual int readInstances(SectionHeader signature); //binary
};

class DefinitionFile : public DictionaryFile {
public:
    const QStringList validOutputs(){
        return QStringList{"TMD", "BMD"};
    };
    QStringList majorSections;
    void updateValue(QModelIndex topLeft, QModelIndex bottomRight);
    void updateEnum();
    int indexIn(QString searchName);
    void writeText();
    void writeBinary();
    int dictItemIndex(int dictIndex, QString searchName);
    void newInstance();
    void newItem();
    std::shared_ptr<taData> createItem(QString itemType);
    void updateCenter();
    int readDictionary(); //text version
    int readDictionary(SectionHeader signature); //binary version
    void createDBTree();
    void editTreeItem(QModelIndex item, int itemIndex);

};

class DatabaseFile : public DictionaryFile{
public:
    const QStringList validOutputs(){
        return QStringList{"TDB", "BDB", "DAE"};
    };

    std::vector<dictItem> instances;
    int maxInstances;
    QStringList majorSections;
    /*std::vector<Warpgate> warpgates;
    std::vector<Pickup> pickups;*/
    void writeText();
    void writeBinary();
    void writeDAE();
    void updateValue(QModelIndex topLeft, QModelIndex bottomRight);

    int dictItemIndex(int dictIndex, QString searchName);

    void newInstance();
    void newItem();

    std::shared_ptr<taData> createItem(QString itemType);
    /*std::vector<Warpgate> sendWarpgates();
    std::vector<Pickup> sendPickups();*/
    std::vector<dictItem> sendInstances(QString instanceType);
    void updateCenter();
    int readIncludedFiles(QString fullRead); //note that includedfiles only needs fullread, not partsplit
    int readFileDictionary(); //text version
    int readInstances(); //text version
    int readFileDictionary(SectionHeader signature); //binary
    int readInstances(SectionHeader signature); //binary
    void createDBTree();
    void filterInstances();
    void editTreeItem(QModelIndex item, int itemIndex);
    int addInstance(dictItem itemToAdd);
    //int addInstance(Pickup itemToAdd);
    void removeAll(QString itemType);

    std::vector<std::shared_ptr<taData>> generateAttributes(QString className);

    dictItem* getLink(int linkID);
    void writeData();
    int instanceIndexIn(int searchIndex);
    void createItem();
    void removeTreeInstance(QModelIndex item);
    void removeAttribute(int instanceIndex, int itemIndex);
    void removeInstance(int instanceIndex);
    void acceptVisitor(ProgWindow& visitor);
};

#endif // DATABASE_H
