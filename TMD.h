#ifndef DATABASE_H
#define DATABASE_H

#include <QQuaternion>
#include <QString>

class ProgWindow;

class dictItem{
public:
    const QStringList typeList{"Enum","Float","Point","Quaternion","Bool","IntegerArray","StringArray","String","Color", "Integer", "FloatArray", "Link", "LinkArray"};
    int index;
    QString type;
    bool active;
    QString name;
    int intValue;
    float floatValue;
    QVector3D pointValue;
    QQuaternion quatValue;
    bool boolValue;
    int integerArray;
    int stringArray;
    int floatArray;
    QString link;
    int linkArray;
    std::vector<float> colorValue;
    QString stringValue;
    std::vector<QString> enumOptions;

    void clear();
    const void operator=(dictItem input);
};

class TMDFile{
  public:
    ProgWindow *parent;
    QString filePath;
    std::vector<std::vector<dictItem>> itemList;
    std::vector<QString> itemNames;

    void readData();
    int indexIn(QString searchName);
    int dictItemIndex(int dictIndex, QString searchName);
    dictItem addItem(dictItem itemDetails, QString tempRead);
    void clear();
};

class TDBFile : public TMDFile{
public:
    void readData();
    std::vector<std::vector<dictItem>> instances;
    std::vector<QString> instanceNames;
};

#endif // DATABASE_H
