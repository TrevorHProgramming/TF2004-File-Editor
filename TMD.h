#ifndef TMD_H
#define TMD_H

#include <QQuaternion>
#include <QString>

class ProgWindow;

class dictItem{
public:
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
};

class TMDFile{
  public:
    const QStringList typeList{"Enum","Float","Point","Quaternion","Bool","IntegerArray","StringArray","String","Color", "Integer", "FloatArray", "Link", "LinkArray"};
    ProgWindow *parent;
    QString filePath;
    std::vector<std::vector<dictItem>> itemList;
    std::vector<QString> itemNames;

    void readData();
    void clear();
};

#endif // TMD_H
